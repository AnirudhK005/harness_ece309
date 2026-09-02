/*
 * harness.c
 *
 * Simple command-line program for beginners.
 * - Reads lines (max 127 chars) in an infinite loop using fgets
 * - If the line is exactly "exit", the program ends
 * - If the entire line is a simple math expression (number op number), it evaluates it
 * - If the line contains the word "hello" (case-insensitive), prints a greeting
 * - Otherwise, echoes the input back
 * - Maintains a circular history of the last 5 user inputs in history[5][128]
 * - Appends each prompt+response to vibe_coding_log.md
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#define MAX_LEN 128
#define HISTORY_SIZE 5

/* Evaluate a simple binary math operation for two doubles. */
static int evaluate_math(double a, char op, double b, double *out) {
    switch (op) {
        case '+': *out = a + b; return 1;
        case '-': *out = a - b; return 1;
        case '*': *out = a * b; return 1;
        case '/':
            if (b == 0.0) return 0; /* division by zero */
            *out = a / b; return 1;
        case '%': /* use fmod for floating-point modulus */
            if (b == 0.0) return 0;
            *out = fmod(a, b); return 1;
        case '^': /* exponentiation */
            *out = pow(a, b); return 1;
        default: return 0;
    }
}

/* Trim trailing newline and carriage return from string in-place. */
static void chomp(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len-1] == '\n' || s[len-1] == '\r')) {
        s[len-1] = '\0';
        len--;
    }
}

/* Securely zero memory to avoid leftover data being observable.
 * Uses a volatile pointer to prevent the optimizer from removing the clear.
 */
static void secure_zero(void *p, size_t n) {
    volatile unsigned char *q = (volatile unsigned char *)p;
    while (n--) *q++ = 0;
}

int main(void) {
    char buf[MAX_LEN];
    char history[HISTORY_SIZE][MAX_LEN]; /* circular buffer of last 5 inputs */
    int hist_idx = 0; /* next position to write */
    int hist_count = 0; /* how many entries currently stored (max 5) */
    int iteration = 1; /* iteration counter for logging */

    /* Logging to file removed: `vibe_coding_log.md` is treated as a static
     * documentation file. All per-interaction logging was intentionally
     * removed so the CLI no longer writes to disk. */

    /* Main loop: run forever until user types "exit" */
    while (1) {
        /* Prompt the user */
        printf("> "); /* simple prompt */
        fflush(stdout);

        /* Read input using fgets, limit to MAX_LEN-1 so we keep room for NUL */
        if (fgets(buf, MAX_LEN, stdin) == NULL) {
            /* EOF or error: exit cleanly */
            puts("Goodbye.");
            break;
        }

        /* If the input was longer than our buffer, fgets will not read the
         * trailing newline. Detect truncation and drain the rest of the line
         * from stdin to keep the next read in sync. */
        size_t readlen = strlen(buf);
        int truncated = 0;
        if (readlen > 0 && buf[readlen-1] != '\n' && !feof(stdin)) {
            /* line may be truncated if last char isn't a newline and there
             * are more bytes pending. Drain until newline or EOF. */
            int c;
            truncated = 1;
            while ((c = getchar()) != EOF && c != '\n') { /* drain */ }
        }

        /* Remove trailing newline(s) */
        chomp(buf);

        /* Always ensure NUL-termination */
        buf[MAX_LEN-1] = '\0';

        /* If user typed exactly "exit", break the loop and end program */
        if (strcmp(buf, "exit") == 0) {
            puts("Exiting...");
            break;
        }

        /* First: check whether the entire input is a math expression.
         * We accept forms like: <number> <op> <number>
         * where <op> is one of + - * / % ^ and numbers are scanned as doubles.
         */
        double a, b, result;
        char op;
        int chars_consumed = 0;
        /* Attempt to parse "double op double" and record how many chars were consumed. */
        int parsed = sscanf(buf, " %lf %c %lf %n", &a, &op, &b, &chars_consumed);
        int is_math = 0;
        if (parsed == 3) {
            /* Ensure remaining characters (if any) are only spaces */
            int ok = 1;
            for (size_t i = chars_consumed; i < strlen(buf); i++) {
                if (!isspace((unsigned char)buf[i])) { ok = 0; break; }
            }
            /* Ensure operator is one we support */
            if (ok && (op == '+' || op == '-' || op == '*' || op == '/' || op == '%' || op == '^')) {
                if (evaluate_math(a, op, b, &result)) {
                    is_math = 1;
                }
            }
        }

        /* Prepare response string to echo or print result. Also record decision for logging. */
        char response[MAX_LEN];
        char decision[MAX_LEN];
        if (is_math) {
            /* Math has highest precedence when the entire input matches */
            snprintf(response, MAX_LEN, "Result: %.10g", result);
            strncpy(decision, "math", MAX_LEN-1);
            decision[MAX_LEN-1] = '\0';
            puts(response);
        } else {
            /* Not a pure math expression; check for "hello" (case-insensitive) */
            char lower[MAX_LEN];
            for (size_t i = 0; i <= strlen(buf) && i < MAX_LEN; i++) lower[i] = tolower((unsigned char)buf[i]);
            lower[MAX_LEN-1] = '\0';

            if (strstr(lower, "hello") != NULL) {
                /* Hardcoded greeting */
                strncpy(response, "Hello! Nice to meet you.", MAX_LEN-1);
                response[MAX_LEN-1] = '\0';
                strncpy(decision, "hello", MAX_LEN-1);
                decision[MAX_LEN-1] = '\0';
                puts(response);
            } else if (strcmp(lower, "history") == 0) {
                /* History command has lower precedence than hello but higher than echo.
                 * Print the stored history (oldest -> newest). Note: history entries
                 * are the last stored user inputs and we store the current input
                 * after processing, so printing here excludes the current 'history' command.
                 */
                strncpy(decision, "history", MAX_LEN-1);
                decision[MAX_LEN-1] = '\0';
                if (hist_count == 0) {
                    puts("History is empty.");
                } else {
                    printf("History (oldest->newest):\n");
                    int oldest = (hist_idx - hist_count + HISTORY_SIZE) % HISTORY_SIZE;
                    for (int i = 0; i < hist_count; i++) {
                        int idx = (oldest + i) % HISTORY_SIZE;
                        printf("- %s\n", history[idx]);
                    }
                }
                snprintf(response, MAX_LEN, "Printed %d history entries.", hist_count);
            } else {
                /* Default: echo the user's input */
                snprintf(response, MAX_LEN, "You said: %s", buf);
                strncpy(decision, "echo", MAX_LEN-1);
                decision[MAX_LEN-1] = '\0';
                puts(response);
            }
        }

        /* Per-interaction file logging removed; the program no longer writes
         * to `vibe_coding_log.md`. */

        /* Now store the user input into history (circular). We store after processing
         * so that the 'history' command prints prior turns and does not include itself.
         * Zero the target slot first to avoid leftover bytes from previous longer
         * entries being observable if anything later reads the raw buffer.
         */
        secure_zero(history[hist_idx], sizeof history[hist_idx]);
        /* Copy up to MAX_LEN-1 and ensure NUL termination */
        strncpy(history[hist_idx], buf, MAX_LEN-1);
        history[hist_idx][MAX_LEN-1] = '\0';
        hist_idx = (hist_idx + 1) % HISTORY_SIZE;
        if (hist_count < HISTORY_SIZE) hist_count++;

        iteration++;
    }

    return 0;
}
