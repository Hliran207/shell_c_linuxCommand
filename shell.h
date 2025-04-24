
#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>

char *read_line(void);
char **parse_line(char *line, int *isPipe, char **Tokens2, char **directFile, int *directIndex);
int launch_process(char **args);
int launch_pipe(char **tokens, char **tokens2);
void whiteSpace(char *str);
int handle_builtin(char **tokens);                                                                  /* for built in functions like pwd, cd, clear, exit  */
void execute_command(char **tokens, int isPipe, char **Tokens2, char *directFile, int directIndex); /* other functins contains pipe and redirections */

#endif