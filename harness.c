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

        /* Save the raw user input into history (circular) */
        strncpy(history[hist_idx], buf, MAX_LEN-1);
        history[hist_idx][MAX_LEN-1] = '\0';
        hist_idx = (hist_idx + 1) % HISTORY_SIZE;
        if (hist_count < HISTORY_SIZE) hist_count++;

        /* If user typed exactly "exit", break the loop and end program */
        if (strcmp(buf, "exit") == 0) {
            FILE *logf = fopen("vibe_coding_log.md", "a");
            if (logf) {
                fprintf(logf, "User: %s\nProgram: %s\n\n", buf, "Program exiting.");
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

        /* Prepare response string to echo or print result */
        char response[MAX_LEN];
        if (is_math) {
            /* Print numeric result with reasonable formatting */
            snprintf(response, MAX_LEN, "Result: %.10g", result);
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
                puts(response);
            } else {
                /* Default: echo the user's input */
                snprintf(response, MAX_LEN, "You said: %s", buf);
                puts(response);
            }
        }

        /* Append this interaction to the log file */
        FILE *logf = fopen("vibe_coding_log.md", "a");
        if (logf) {
            fprintf(logf, "User: %s\nProgram: %s\n\n", buf, response);
            fclose(logf);
        }
    }

    return 0;
}
