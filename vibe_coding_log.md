
# Vibe Coding Log

## Architectural Decisions

- Simple CLI written in C.
- Input method: `fgets()` with a 127-character input limit (buffer size 128).
- Conversation history: a circular buffer `char history[5][128]` storing the last 5 user turns.
- Precedence (applied in this order):
	1. Math detection — the entire input must be a binary expression `number operator number` to be evaluated.
	2. `hello` detection — case-insensitive substring match.
	3. `history` command — exact input `history` (case-insensitive) prints stored turns. This has lower precedence than `hello`.
	4. Echo — default fallback that echoes the user's input.
- Supported math operators: `+`, `-`, `*`, `/`, `%` (uses `fmod`), `^` (exponentiation via `pow`).
- Logging: each interaction appends an `Iteration`, `User`, `Decision`, and `Program` entry to this file in plain English.

## User Prompts (verbatim)

Below are the exact prompts you provided during this session. Each block contains a user message verbatim.

1) Initial program specification:

```
I need to write a simple command-line program in C. I am a beginner, so please keep the code as simple as possible. Do not use external libraries, only standard ones like <stdio.h> and <string.h>. Here is the specification for the program:

It should run an infinite while loop that asks for user input using fgets. Input is limited to 127.
If the user types 'exit', the loop should break and the program should end.
If the user types a sentence containing the word 'hello', the program should print a hardcoded greeting.
We need to be able to call a tool to execute certain functions (for example math calculations) that an LLM is less suited for. If a math operator: */+-%^, etc. is appropriately surrounded by numbers as it would look like in a math expression, then call the math tool.
Check the input first for math. Math must be the entire input. If not, then check for hello, if hello is not present, then echo.
If the user types anything else, it should echo their input back to them.
Please add clear, line-by-line comments explaining what the code is doing.
Ensure that all exact prompts, iterations, and the AI's responses are recorded in a file called vibe_coding_log.md and push it to the repo.
We need to manage memory to store a conversation history of the last 5 turns. Use an array of size [5][128] to do this
Include project details in the readme file.
Code needs to be in a file named harness.c
```

2) Request for a test script:

```
Write a simple Bash script (for
Linux/Mac) that compiles the code, inputs more than 5 turns that contain a hello, a math expression, plain text (not containing hello or math), and exit. Check the outputs for correct greeting, echo, math behavior and correct order of the latest 5 turns from history.
```

3) Testing and logging requirements:

```
1. Remember to include all the prompts, iterations, and AI outputs in the vibe_coding_log.md file. 
2. For testing, include prompts that combine math with hello, or math with plain text, in order to test the precedence ordering. 
3. Test if history correctly saves last 5 turns.
```

4) History command and completeness request:

```
You did not include all the prompts, iterations, AI outputs, and architectural decisions in the vibe_coding_log.md file. Implement that. Add a history command for testing purposes to print stored turns. The command will only print history if the word history only is in the input line. History takes lowest precedence in our precedence list (goes after hello). Verify history works as intended.
```

5) Clarification on what "iteration" means and final request to include all material:

```
By iteration, I mean include all the prompts, iterations, AI outputs, and architectural decisions in the vibe_coding_log.md file as English descriptions. Include all the past exact prompts I gave you and English summary of each AI prompt response/action taken. Include archetectural decisions as its own section in the file as well as iterations.
```

## AI Responses and Actions (summary)

Below is a concise, chronological summary of the assistant's actions in response to the prompts above. Each entry maps to the corresponding prompt(s) and describes what was implemented, tested, and committed to the repository.

1) Implement CLI program (`harness.c`):

- Created `harness.c` implementing the specified behavior: reads input with `fgets()`, checks for exact `exit`, detects math-only expressions and evaluates them, detects `hello` substring, echoes otherwise.
- Implemented a circular history buffer `char history[5][128]` and stored entries.
- Wrote iteration- and decision-aware logging to `vibe_coding_log.md` for each turn.
- Added line-by-line comments in `harness.c` for clarity.
- Updated `README.md` with build and run instructions and a project summary.
- Committed and pushed these files to the repository.

2) Add test script (`scripts/test_harness.sh`):

- Wrote a Bash script that compiles `harness.c`, feeds more than 5 inputs (hello, math, plain text), captures output, and verifies that responses match expectations and that the last 5 stored turns are in the correct order.
- Made the script macOS-compatible (avoided `mapfile`, avoided negative array indices) and added checks for log contents.
- Committed and pushed the test script to the repository.

3) Iterative fixes and improvements:

- Fixed logging and test ordering when initial test revealed the persistent log had prior content; adjusted the test to reset `vibe_coding_log.md` before running to ensure deterministic checks.
- Resolved macOS shell compatibility issues in the test script (replaced `mapfile` usage and negative array indices).

4) Add `history` command and richer logging:

- Implemented a `history` command in `harness.c` that prints stored turns when the user input is exactly `history` (case-insensitive). The command is lower precedence than `hello` but higher than `echo`.
- Changed when the program appends to the circular history: inputs are now stored after processing so that `history` does not include itself.
- Enhanced `vibe_coding_log.md` entries to include `Iteration` and `Decision` fields alongside `User` and `Program` outputs.
- The program writes an initial architecture header to `vibe_coding_log.md` when it starts.
- Committed and pushed these changes.

5) Test updates to cover combined math+hello/plain cases and verification of logs:

- Updated the test script to include combined cases (`hello 2 + 3`, `2 + 3 hello`, `2 + 3 is nice`) to verify precedence rules (math-only must be entire input; otherwise `hello` wins when present; math with extra text should not be evaluated).
- Verified that the program and test script pass on macOS and Linux-compatible shells.

## Iterations (record of prompts, decisions, and program outputs)

The following sections are the recorded iterations produced by recent test runs. Each iteration lists the `User` input, the `Decision` the program took, and the `Program` output. These entries were appended to this file by the running program.

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

---

If you would like, I can now:

- Append a timestamp to each iteration entry.
- Persist history between program runs in a small file (e.g., `history.dat`).
- Expand math parsing to support parentheses and operator precedence.

Which of these would you like next?

