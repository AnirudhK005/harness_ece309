# Vibe Coding Log

Architecture: simple CLI in C.
- Input method: fgets, max 127 chars.
- History: circular buffer history[5][128], stores last 5 user turns.
- Precedence: math (entire input) -> hello (contains) -> history (exact) -> echo.
- Math operators supported: + - * / % ^ (% uses fmod).

Iteration: 1
User: alpha
Decision: echo
Program: You said: alpha

Iteration: 2
User: hello world
Decision: hello
Program: Hello! Nice to meet you.

Iteration: 3
User: 3 * 4
Decision: math
Program: Result: 12

Iteration: 4
User: hello 2 + 3
Decision: hello
Program: Hello! Nice to meet you.

Iteration: 5
User: 2 + 3 hello
Decision: hello
Program: Hello! Nice to meet you.

Iteration: 6
User: 2 + 3
Decision: math
Program: Result: 5

Iteration: 7
User: 2 + 3 is nice
Decision: echo
Program: You said: 2 + 3 is nice

Iteration: 8
User: foo
Decision: echo
Program: You said: foo

Iteration: 9
User: exit
Decision: exit
Program: Program exiting.

