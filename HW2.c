
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/limits.h>
#include "shell.h"

int main()
{
    int childPid;
    char *line;

    while (1)
    {
        // Display prompt with current directory
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL)
            perror("getcwd");
        printf("%s> ", cwd);
        fflush(stdout);

        // Read input
        line = read_line();
        if (!line)
        {
            break;
        }

        // Parse input
        char *Tokens2[128];
        char *directFile = NULL;
        int directIndex = -1;
        int isPipe = 0;

        char **tokens = parse_line(line, &isPipe, Tokens2, &directFile, &directIndex);

        if (!tokens[0])
        {
            free(tokens);
            free(line);
            continue;
        }

        // Handle built-in commands
        int builtin_status = handle_builtin(tokens);
        if (builtin_status == -1)
        {
            // Exit command
            free(tokens);
            free(line);
            break;
        }
        else if (builtin_status == 1)
        {
            // Built-in command was handled
            free(tokens);
            free(line);
            continue;
        }

        childPid = fork();

        if (childPid == 0)
        {
            // Child process
            execute_command(tokens, isPipe, Tokens2, directFile, directIndex);

            exit(EXIT_FAILURE);
        }
        else if (childPid > 0)
        {
            // Parent process
            waitpid(childPid, NULL, 0);
        }

        free(tokens);
        free(line);
    }
    return 0;
}