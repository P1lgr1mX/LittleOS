#include "commands.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"

#define MAX_LINE_LEN 128
#define MAX_ARGS     16

static char line_buf[MAX_LINE_LEN];
static char *argv[MAX_ARGS];

/**
 * tokenize:
 * Parses an input command line into an argument vector (argv) delimited by whitespace.
 */
static int tokenize(char *line, char **args, int max_args)
{
    int count = 0;
    while (*line != '\0' && count < max_args) {
        /* Skip leading whitespace characters */
        while (*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r') {
            line++;
        }
        if (*line == '\0') {
            break;
        }

        args[count++] = line;

        /* Scan to the next whitespace delimiter */
        while (*line != '\0' && *line != ' ' && *line != '\t' && *line != '\n' && *line != '\r') {
            line++;
        }

        if (*line != '\0') {
            *line++ = '\0';
        }
    }
    args[count] = (char *)0;
    return count;
}

int main(void)
{
    printf("\n=================================================================\n");
    printf("         Welcome to AetherOS Userland Shell (Ring 3)             \n");
    printf("         Type 'help' to see available commands.                  \n");
    printf("=================================================================\n\n");

    while (1) {
        printf("AetherOS> ");

        char *line = gets(line_buf, MAX_LINE_LEN);
        if (!line) {
            continue;
        }

        int argc = tokenize(line, argv, MAX_ARGS);
        if (argc == 0) {
            continue;
        }

        /* Look up command in the builtin_commands dispatch table */
        int found = 0;
        for (int i = 0; i < num_builtin_commands; i++) {
            if (strcmp(argv[0], builtin_commands[i].name) == 0) {
                builtin_commands[i].func(argc, argv);
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("AetherOS: command not found: %s. Type 'help' for available commands.\n", argv[0]);
        }
    }

    return 0;
}
