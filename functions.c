

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/limits.h>
#include "shell.h"

char *read_line()
{
    char *line = NULL;
    size_t bufsize = 0;

    if (getline(&line, &bufsize, stdin) == -1)
    {
        if (feof(stdin))
        {
            // if user pressed eof/ctrl d
            free(line);
            return NULL;
        }

        perror("get line error");
        free(line);
        return NULL;
    }
    return line;
}

char **parse_line(char *line, int *isPipe, char **Tokens2, char **directFile, int *directIndex)
{

    char **tokens = calloc(128, sizeof(char *));
    int pos = 0;

    *isPipe = 0;
    *directIndex = -1;
    *directFile = NULL;
    memset(Tokens2, 0, 128 * sizeof(char *));

    whiteSpace(line);

    char *token = strtok(line, " \t\r\n");
    while (token)
    {
        if (strcmp(token, "|") == 0)
        {
            *isPipe = 1;
            int pos2 = 0;
            token = strtok(NULL, " \t\r\n");
            while (token)
            {
                Tokens2[pos2++] = token;
                token = strtok(NULL, " \t\r\n");
            }
            Tokens2[pos2] = NULL;
            break;
        }
        else if (strcmp(token, ">") == 0)
        {
            *directIndex = pos;
            token = strtok(NULL, " \t\r\n");
            if (token)
                *directFile = token;
            break;
        }
        else
        {
            tokens[pos++] = token;
        }
        token = strtok(NULL, " \t\r\n");
    }
    tokens[pos] = NULL;
    return tokens;
}

void whiteSpace(char *str)
{
    char *frontp = str;
    while (*frontp && (*frontp == ' ' || *frontp == '\t' || *frontp == '\n'))
        frontp++;
    memmove(str, frontp, strlen(frontp) + 1);

    char *endp = str + strlen(str) - 1;
    while (endp > str && (*endp == ' ' || *endp == '\t' || *endp == '\n'))
    {
        *endp = '\0';
        endp--;
    }
}

int launch_pipe(char **left, char **right)
{
    int fd[2];
    if (pipe(fd) == -1)
    {
        perror("pipe");
        return -1;
    }

    pid_t p1 = fork();
    if (p1 == 0)
    { /* left side  */
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(left[0], left);
        perror("execvp left");
        exit(EXIT_FAILURE);
    }

    pid_t p2 = fork();
    if (p2 == 0)
    { /* right side */
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        execvp(right[0], right);
        perror("execvp right");
        exit(EXIT_FAILURE);
    }

    close(fd[0]);
    close(fd[1]); /* parent closes both ends */
    waitpid(p1, NULL, 0);
    waitpid(p2, NULL, 0);
    return 0;
}

int handle_builtin(char **tokens)
{
    if (!tokens || !tokens[0])
        return 1;

    if (strcmp(tokens[0], "exit") == 0)
        return -1;

    if (strcmp(tokens[0], "cd") == 0)
    {
        const char *dest = tokens[1];
        if (!dest)
        {
            dest = getenv("HOME");
            if (!dest)
                dest = "/";
        }
        if (chdir(dest) != 0)
            perror("cd");
        return 1;
    }

    if (strcmp(tokens[0], "pwd") == 0)
    {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)))
            printf("%s\n", cwd);
        else
            perror("getcwd");
        return 1;
    }

    if (strcmp(tokens[0], "clear") == 0)
    {
        system("clear");
        return 1;
    }

    return 0;
}

void execute_command(char **tokens, int isPipe, char **Tokens2, char *directFile, int directIndex)
{
    // Special handling for tree command
    if (strcmp(tokens[0], "tree") == 0)
    {
        char *tree_args[3];
        tree_args[0] = "tree";

        // If path argument was provided, use it
        if (tokens[1])
        {
            tree_args[1] = tokens[1];
            tree_args[2] = NULL;
        }
        else
        {
            // Otherwise, use current directory
            tree_args[1] = ".";
            tree_args[2] = NULL;
        }

        // Handle redirection for tree command
        if (directIndex != -1 && directFile != NULL)
        {
            int fd = open(directFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0)
            {
                perror("open error for redirection");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        // Execute the tree program
        execvp("./tree", tree_args);
        perror("execvp tree");
        exit(EXIT_FAILURE);
    }

    // Original function code for other commands
    if (directIndex != -1 && directFile != NULL)
    {
        int fd = open(directFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0)
        {
            perror("open error for redirection");
            exit(EXIT_FAILURE);
        }

        dup2(fd, STDOUT_FILENO);
        close(fd);

        tokens[directIndex] = NULL;
    }

    if (isPipe)
    {
        launch_pipe(tokens, Tokens2);
        exit(EXIT_SUCCESS);
    }
    else
    {
        execvp(tokens[0], tokens);
        perror("execvp error");
        exit(EXIT_FAILURE);
    }
}