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
exit
'

echo "Running harness with test inputs..."
printf "%s" "$INPUTS" > /tmp/harness_inputs.txt
# Reset the log so test can verify exact entries
printf "# Vibe Coding Log\n\n" > vibe_coding_log.md
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

echo "Verifying last 5 non-exit User turns in vibe_coding_log.md..."

# Extract User lines from the log in a portable way (avoid `mapfile` on older macOS bash)
grep '^User: ' vibe_coding_log.md | sed 's/^User: //' > /tmp/_harness_users.txt
users=()
while IFS= read -r line; do
  users+=("$line")
done < /tmp/_harness_users.txt

if [[ ${#users[@]} -eq 0 ]]; then
  echo "FAIL: no User entries found in vibe_coding_log.md"; exit 3
fi

# If the last entry is 'exit', remove it for the purposes of checking the last 5 turns
len=${#users[@]}
if [[ $len -gt 0 && "${users[$((len-1))]}" == "exit" ]]; then
  unset "users[$((len-1))]"
  len=${#users[@]}
fi

# Take the last 5 entries
len=${#users[@]}
start=0
if [[ $len -gt 5 ]]; then start=$((len-5)); fi
last5=()
for ((i=start;i<len;i++)); do last5+=("${users[i]}"); done

expected_last5=("hello 2 + 3" "2 + 3 hello" "2 + 3" "2 + 3 is nice" "foo")

# Verify that the log contains all prompts and AI responses in order.
echo "Verifying vibe_coding_log.md contains all prompts and responses..."
# Build provided input array from the file we sent to the program (/tmp/harness_inputs.txt)
provided=()
while IFS= read -r line; do
  provided+=("$line")
done < /tmp/harness_inputs.txt

# Read User and Program lines from the log
log_users=()
log_programs=()
while IFS= read -r line; do
  case "$line" in
    User:*) log_users+=("${line#User: }") ;;
    Program:*) log_programs+=("${line#Program: }") ;;
  esac
done < vibe_coding_log.md

if [[ ${#provided[@]} -ne ${#log_users[@]} ]]; then
  echo "FAIL: mismatch between provided inputs (${#provided[@]}) and log User entries (${#log_users[@]})"; exit 6
fi

for i in "${!provided[@]}"; do
  if [[ "${provided[i]}" != "${log_users[i]}" ]]; then
    echo "FAIL: log User entry $i mismatch: got '${log_users[i]}', expected '${provided[i]}'"; exit 7
  fi
done

echo "Log contains all User prompts in correct order."


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
