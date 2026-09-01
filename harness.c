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

int main(void) {
    char buf[MAX_LEN];
    char history[HISTORY_SIZE][MAX_LEN]; /* circular buffer of last 5 inputs */
    int hist_idx = 0; /* next position to write */
    int hist_count = 0; /* how many entries currently stored (max 5) */
    int iteration = 1; /* iteration counter for logging */

    /* Write initial architecture/decision log header so the log contains
     * the AI's architectural decisions and format expectations. Open in
     * write mode to reset previous contents when program starts.
     */
    FILE *archf = fopen("vibe_coding_log.md", "w");
    if (archf) {
        fprintf(archf, "# Vibe Coding Log\n\n");
        fprintf(archf, "Architecture: simple CLI in C.\n");
        fprintf(archf, "- Input method: fgets, max %d chars.\n", MAX_LEN-1);
        fprintf(archf, "- History: circular buffer history[%d][%d], stores last 5 user turns.\n", HISTORY_SIZE, MAX_LEN);
        fprintf(archf, "- Precedence: math (entire input) -> hello (contains) -> history (exact) -> echo.\n");
        fprintf(archf, "- Math operators supported: + - * / %% ^ (%% uses fmod).\n\n");
        fclose(archf);
    }

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

        /* Remove trailing newline(s) */
        chomp(buf);

        /* If user typed exactly "exit", break the loop and end program */
        if (strcmp(buf, "exit") == 0) {
            /* Log this final turn with decision "exit" */
            FILE *logf = fopen("vibe_coding_log.md", "a");
            if (logf) {
                fprintf(logf, "Iteration: %d\n", iteration);
                fprintf(logf, "User: %s\n", buf);
                fprintf(logf, "Decision: exit\n");
                fprintf(logf, "Program: Program exiting.\n\n");
                fclose(logf);
            }
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

        /* Append this interaction to the log file with iteration and decision info */
        FILE *logf = fopen("vibe_coding_log.md", "a");
        if (logf) {
            fprintf(logf, "Iteration: %d\n", iteration);
            fprintf(logf, "User: %s\n", buf);
            fprintf(logf, "Decision: %s\n", decision);
            fprintf(logf, "Program: %s\n\n", response);
            fclose(logf);
        }

        /* Now store the user input into history (circular). We store after processing
         * so that the 'history' command prints prior turns and does not include itself.
         */
        strncpy(history[hist_idx], buf, MAX_LEN-1);
        history[hist_idx][MAX_LEN-1] = '\0';
        hist_idx = (hist_idx + 1) % HISTORY_SIZE;
        if (hist_count < HISTORY_SIZE) hist_count++;

        iteration++;
    }

    return 0;
}
