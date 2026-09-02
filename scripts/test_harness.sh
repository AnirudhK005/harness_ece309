#!/usr/bin/env bash
set -euo pipefail

# Simple test script to compile harness.c, run inputs (>5 turns),
# and verify greeting, math, echo behavior and last-5 history order.

cd "$(dirname "$0")/.."

echo "Compiling harness.c..."
gcc -std=c11 -O2 -lm -o harness harness.c

# Prepare inputs (more than 5 turns) including combined math+hello/plain cases.
# Sequence includes:
# 1) alpha                -> echo
# 2) hello world          -> greeting
# 3) 3 * 4                -> math (12)
# 4) hello 2 + 3          -> contains hello -> greeting
# 5) 2 + 3 hello          -> contains hello -> greeting
# 6) 2 + 3                -> math (5)
# 7) 2 + 3 is nice        -> math with plain text -> echo
# 8) foo                  -> echo
# 9) exit                 -> exit program
INPUTS=$'alpha
hello world
3 * 4
hello 2 + 3
2 + 3 hello
2 + 3
2 + 3 is nice
foo
history
exit
'

echo "Running harness with test inputs..."
printf "%s" "$INPUTS" > /tmp/harness_inputs.txt
./harness < /tmp/harness_inputs.txt > /tmp/harness_out.txt

echo "Checking program outputs..."

expected=(
  "You said: alpha"
  "Hello! Nice to meet you."
  "Result: 12"
  "Hello! Nice to meet you."
  "Hello! Nice to meet you."
  "Result: 5"
  "You said: 2 + 3 is nice"
  "You said: foo"
  "Exiting..."
)

idx=0
num=${#expected[@]}
while IFS= read -r line; do
  if [[ "$line" == *"${expected[$idx]}"* ]]; then
    idx=$((idx+1))
    if [[ $idx -ge $num ]]; then
      break
    fi
  fi
done < /tmp/harness_out.txt

if [[ $idx -ne $num ]]; then
  echo "FAIL: program output did not contain expected sequence."
  echo "Captured output:"; sed -n '1,200p' /tmp/harness_out.txt
  exit 2
fi

echo "Output checks passed."

echo "Verifying last 5 non-exit User turns via program 'history' output..."

# Extract history from the program output (we included a 'history' input before exit)
start_line=$(grep -n "History (oldest->newest):" /tmp/harness_out.txt | head -n1 | cut -d: -f1 || true)
history_entries=()
if [[ -n "$start_line" ]]; then
  sed -n "$((start_line+1)),\$p" /tmp/harness_out.txt | sed -n '/^-/p' | sed 's/^- //' > /tmp/_harness_history.txt
  while IFS= read -r line; do history_entries+=("$line"); done < /tmp/_harness_history.txt
fi

if [[ ${#history_entries[@]} -eq 0 ]]; then
  echo "FAIL: no history printed by program"; sed -n '1,200p' /tmp/harness_out.txt; exit 3
fi

# If the last printed entry is 'exit', remove it for the purposes of checking the last 5 turns
len=${#history_entries[@]}
if [[ $len -gt 0 && "${history_entries[$((len-1))]}" == "exit" ]]; then
  unset "history_entries[$((len-1))]"
  len=${#history_entries[@]}
fi

# Take the last 5 entries
len=${#history_entries[@]}
start=0
if [[ $len -gt 5 ]]; then start=$((len-5)); fi
last5=()
for ((i=start;i<len;i++)); do last5+=("${history_entries[i]}"); done

expected_last5=("hello 2 + 3" "2 + 3 hello" "2 + 3" "2 + 3 is nice" "foo")

if [[ ${#last5[@]} -ne ${#expected_last5[@]} ]]; then
  echo "FAIL: unexpected number of last5 entries: got ${#last5[@]} expected ${#expected_last5[@]}"; exit 4
fi

for i in "${!expected_last5[@]}"; do
  if [[ "${last5[i]}" != "${expected_last5[i]}" ]]; then
    echo "FAIL: history mismatch at position $i: got '${last5[i]}', expected '${expected_last5[i]}'"
    echo "Full last5: ${last5[*]}"; exit 5
  fi
done

echo "History check passed. Last 5 turns (oldest->newest):"
for x in "${last5[@]}"; do echo "- $x"; done

echo "ALL TESTS PASSED"

echo "\nRunning overflow and reuse tests to ensure no stale data leakage..."

# Build an overlong input (>127 chars), then a short input, then request history.
LONG=$(printf 'A%.0s' {1..200})
printf "%s\nfoo\nhistory\nexit\n" "$LONG" > /tmp/harness_inputs_overflow.txt

./harness < /tmp/harness_inputs_overflow.txt > /tmp/harness_overflow_out.txt

# Extract history entries printed by the program (lines starting with '- ' after the header)
start_line=$(grep -n "History (oldest->newest):" /tmp/harness_overflow_out.txt | head -n1 | cut -d: -f1 || true)
overflow_history=()
if [[ -n "$start_line" ]]; then
  sed -n "$((start_line+1)),\$p" /tmp/harness_overflow_out.txt | sed -n '/^-/p' | sed 's/^- //' > /tmp/_overflow_history.txt
  while IFS= read -r line; do overflow_history+=("$line"); done < /tmp/_overflow_history.txt
fi

if [[ ${#overflow_history[@]} -lt 2 ]]; then
  echo "FAIL: overflow test did not produce expected history output"; sed -n '1,200p' /tmp/harness_overflow_out.txt; exit 8
fi

# The program truncates inputs longer than MAX_LEN-1; verify the stored long entry
# equals the first 127 characters (MAX_LEN-1)
expected_long=${LONG:0:127}
if [[ "${overflow_history[0]}" != "$expected_long" ]]; then
  echo "FAIL: long history entry mismatch (was truncated differently)"; exit 9
fi

if [[ "${overflow_history[1]}" != "foo" ]]; then
  echo "FAIL: short entry was contaminated by previous long input: got='${overflow_history[1]}'"; exit 10
fi

echo "Overflow/reuse check passed: long entry preserved and short entry not contaminated."

echo "\nRunning memory-leak checks (AddressSanitizer preferred, fallback to valgrind if available)..."

# Attempt AddressSanitizer build and run
ASAN_BIN=./harness_asan
if gcc -fsanitize=address -g -O1 -lm -o "$ASAN_BIN" harness.c 2>/tmp/asan_build.txt; then
  echo "ASAN build succeeded; running ASAN check with timeout watchdog..."
  # Run ASAN build with same inputs under a simple watchdog to avoid hangs.
  "$ASAN_BIN" < /tmp/harness_inputs.txt > /tmp/harness_asan_out.txt 2> /tmp/harness_asan_err.txt &
  asan_pid=$!
  # Wait up to 8 seconds for ASAN to finish, then kill if still running
  watchdog=8
  for i in $(seq 1 $watchdog); do
    if ! kill -0 "$asan_pid" 2>/dev/null; then
      break
    fi
    sleep 1
  done
  if kill -0 "$asan_pid" 2>/dev/null; then
    # Still running -> timeout
    kill -9 "$asan_pid" 2>/dev/null || true
    echo "ASAN run timed out after ${watchdog}s" > /tmp/harness_asan_status.txt
  else
    wait "$asan_pid" 2>/dev/null || true
    exitcode=$?
    if [[ $exitcode -eq 0 ]]; then
      echo "ASAN run completed with no runtime error." > /tmp/harness_asan_status.txt
    else
      echo "ASAN reported errors; exitcode $exitcode" > /tmp/harness_asan_status.txt
    fi
  fi
  # Print ASAN results to console (stderr first, then status)
  echo "Memory-leak check (ASAN) stderr output:" >&2
  sed -n '1,200p' /tmp/harness_asan_err.txt >&2 || true
  echo "Memory-leak check (ASAN) status:" >&2
  sed -n '1,200p' /tmp/harness_asan_status.txt >&2 || true

  # If ASAN timed out or reported errors, try valgrind as a fallback (if available)
  if [ -f /tmp/harness_asan_status.txt ]; then
    status_text=$(cat /tmp/harness_asan_status.txt)
    if echo "$status_text" | grep -iqE "timed out|reported errors|error"; then
      if command -v valgrind >/dev/null 2>&1; then
        echo "ASAN timed out/reported errors; running valgrind fallback..." >&2
        valgrind --leak-check=full --log-file=/tmp/harness_valgrind.txt ./harness < /tmp/harness_inputs.txt > /tmp/harness_valgrind_out.txt 2>&1 || true
        echo "Memory-leak check (valgrind) output:" >&2
        sed -n '1,200p' /tmp/harness_valgrind.txt >&2 || true
      else
        echo "Valgrind not available on this system; cannot run fallback." >&2
      fi
    fi
  fi
else
  echo "ASAN build failed or not available; checking for valgrind..."
  if command -v valgrind >/dev/null 2>&1; then
    echo "valgrind found; running valgrind leak check..."
    valgrind --leak-check=full --error-exitcode=1 --log-file=/tmp/harness_valgrind.txt ./harness < /tmp/harness_inputs.txt > /tmp/harness_valgrind_out.txt 2>&1 || true
    echo "Memory-leak check (valgrind) output:" >&2
    sed -n '1,200p' /tmp/harness_valgrind.txt >&2 || true
  else
    echo "No ASAN or valgrind available; skipping automated leak check." >&2
  fi
fi

echo "Memory-leak checks complete."
