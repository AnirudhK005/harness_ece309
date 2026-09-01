# harness_ece309

Simple C command-line harness for ECE309-style exercises.

Project files
- `harness.c`: The command-line program that reads user input, evaluates simple math expressions, responds to greetings, echoes inputs, and keeps a 5-turn conversation history.
- `vibe_coding_log.md`: Conversation and interaction log (appended by the program each turn).

How to build

Compile with gcc (requires math library):

```
gcc -std=c11 -O2 -lm -o harness harness.c
```

Run

```
./harness
```

Behavior
- Reads input lines (max 127 characters) in an infinite loop using `fgets()`.
- If the input is exactly `exit`, the program terminates.
- If the entire input is a math expression of the form `number operator number` (operators: + - * / % ^), it evaluates the expression and prints the result.
- If the input contains the word `hello` (case-insensitive), the program prints a hardcoded greeting.
- Otherwise, the program echoes the user's input back.
- The program maintains a circular conversation history of the last 5 user inputs in a `char history[5][128]` array.
- Each interaction (user input and program response) is appended to `vibe_coding_log.md`.

Notes
- The program uses only standard C libraries (`stdio.h`, `string.h`, `ctype.h`, `math.h`) and is intentionally kept simple for beginners.
