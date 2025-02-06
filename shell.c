#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include "tokenize_func.h"

#define MAX 255  // Maximum size of a command line

#define MAX_LEN 256

#define MAX_TOKENS 100  // Define a maximum number of tokens to handle

void help_pipe(char *current_command, char *next_command);
void help_redirIN(char *last_command, char *file);
void help_redirOUT(char *last_command, char *file);
void basic_commands(char *input);
help_lhs_rhs(char *side_command, int pipe_file_descriptors[], int is_left);

// Function to split the input command into individual arguments.
// Uses spaces between words to seperate them and stores the commands in args.
// note: I added stuff to check for quotes because I assumed it was given input
// with quotes but realized it wasnt later down the line
void check_commands(char *input, char **args) {
    int index = 0;
    char *arg_start = input;
    int in_quotes = 0; 
    
    // Allocate space for args elements and initialize
    for (int i = 0; i < 10; i++) {
        args[i] = malloc(256);
        args[i][0] = '\0';
    }

    for (int i = 0; input[i] != '\0'; i++) { // my attempt at adapting the check commands for quotes
        if (input[i] == '"') {
            in_quotes = !in_quotes;
            continue;
        }
        if (in_quotes == 1 || input[i] != ' ') { // if in quotes and not empty space
            strncat(args[index], &input[i], 1);
        } else if (input[i] == ' ' && in_quotes == 0) { // Move to next argument if not inside quotes
            if (strlen(args[index]) > 0) {  // Only move if there's something in args[index]
                index++;
            }
        }
    }

    // Null-terminate the argument list
    args[index + 1] = NULL;
}

// Function to execute a single command.
// Checks for commands 'prev' and 'cd', and handling other commands using execvp.
// note: known issue here with commands in quotes. Since tokenize_func is called in handle_commands the quotes on 
// all inputs are already removed, when check commands is run it then splits the values that should be together
// apart since the input given to basic commands had them removed earlier.
void basic_commands(char *input) {
    char *args[MAX / 2 + 1];  // Command line arguments

    // command 1 helper func
    int helpTrue = strcmp(input, "help");
    if (helpTrue == 0) {
        printf("all methods that are callable\n");
        printf("cd [directory]:   change directories to [directory].\n");
        printf("prev:  run the last command used.\n");
        printf("help:  display all commands usable.\n");
        printf("source [file]:  read and run commands from [file].\n");
        printf("exit:   exit/quit the shell\n");
        printf("[command1] ; [command2]:  execute [command1] then [command2].\n");
        printf("[command] > [file]:   redirects the output of [command] to [file].\n");
        printf("[command] < [file]:   use [file] as the input for [command].\n");
        printf("[command1] | [command2]:  use the output of [command1] as the input for [command2].\n");
        return;
    }

    check_commands(input, args);  // call func

    int sourceTrue = strcmp(args[0], "source");
    if (sourceTrue == 0) {
        // init vars
        char *line = NULL;
        size_t linelen = 0;
        ssize_t newlineread;

        if (args[1] == NULL) {
            printf("no file to source\n");
            return;
        }

        // if file exists open the file in readmode
        FILE *readfile = fopen(args[1], "r");
        if (readfile == NULL) {
            perror("readfile null");
            return;
        }

        // Read and process each line
        while ((newlineread = getline(&line, &linelen, readfile)) != -1) {
            line[strcspn(line, "\n")] = 0;  // Remove newline character
            if (strlen(line) > 0) {
                basic_commands(line);  // Process the line as a command
            }
        }

        free(line);
        fclose(readfile);
        return;
    }

    // Handle built-in 'cd' command
    int cdTrue = strcmp(args[0], "cd");
    if (cdTrue == 0) {
        if (args[1] == NULL) {
            printf("cd needs arg");
        }
        int change = chdir(args[1]);
        if (change != 0) {
            perror("cd fail"); // if change dir fails perror
        }
        return; // exit function since changing dir affects fork
    }
    pid_t fork_pid = fork();  // fork for process
    if (fork_pid < 0) { // error handling
        perror("fork_pid fail");
        return;
    }
    if (fork_pid == 0) { // if fork is zero we are in child
        // citing :https://stackoverflow.com/questions/27541910/how-to-use-execvp
        int exec = execvp(args[0], args);
        if (exec == -1) {
            printf("%s: command not found\n", args[0]);
        }
        exit(EXIT_FAILURE);  // end
    } else { // if fork_pid > 0
        wait(NULL); // let child run
    }
}

// Did you try the following ??
// Splitting the input by ; and ensuring each command is trimmed and executed individually.
// Fork Error Handling Making sure the child process exits with EXIT_FAILURE if execvp() fails, to avoid zombie processes
// Ensuring that *command is freed and reassigned properly to avoid memory leaks when dealing with the “prev” command
// pipe handling notes:
// detect pipe
// if command run right before go to different pipe command
// run command
// detects another pipe
// command run before
// take out and use as lhs
// run pipe lhs rhs
// runs pipe on the second part of last command and the rhs of pipe
// Helper function to handle multiple commands 
// It splits the input by a delimiter if it exists and processes each command accordingly by calling helpers.
// note for grader: one known issue here for pipe. Pipe works on a basic level, if you call "shuf -i 1-10 | nl"
// pipe works fine, the issue is when either there is more than one pipe or that a command is run before pipe
// eg: "sort < file.txt | nl" this is bc I cant figure out how to capture output of sort < file.txt before the pipe.
void handle_commands(char *input) {
    // Tokenize the input using the new tokenize function
    TokenList tokenList = tokenize_func(input);

    // create vars
    char *current_command = NULL;
    char *pipe_out = NULL;
    int pipe_true = 0;
    int command_start = 0;
    int command_run = 0;
    char strings[tokenList.count][100];
    
    char *remaining_commands = 0; // Start with an empty string

    // Iterate through the tokens
    for (int i = 0; i < tokenList.count; i++) {
        // Check for the semicolon token to indicate command separation
        if (strcmp(tokenList.values[i], ";") == 0 ||
            strcmp(tokenList.values[i], "|") == 0 ||
            strcmp(tokenList.values[i], "<") == 0 ||
            strcmp(tokenList.values[i], ">") == 0) {
            if (current_command != NULL) {
                current_command[command_start] = '\0'; // Null-terminate the current command
                if (strchr(tokenList.values[i], '|') != NULL) {
                    int remaining_length = 0;
                    for (int j = i + 1; j < tokenList.count; j++) {
                        remaining_length += strlen(tokenList.values[j]) + 1; // +1 for space
                    }

                    // Allocate memory for the right-hand side command (+1 for null terminator)
                    char *remaining_commands = (char *)malloc((remaining_length + 1) * sizeof(char));
                    if (remaining_commands == NULL) {
                        perror("Failed to allocate memory for remaining commands");
                        exit(EXIT_FAILURE);
                    }

                    // Initialize the remaining_commands string
                    remaining_commands[0] = '\0'; // Start with an empty string

                    // Concatenate the remaining tokens into remaining_commands for the right-hand side
                    for (int k = i + 1; k < tokenList.count; k++) {
                        if (strcmp(tokenList.values[k], "|") == 0) {
                            i = k;
                            pipe_true = 1;
                            break;  // Handle the next pipe
                        } else {
                            strcat(remaining_commands, tokenList.values[k]); // Append the token
                            if (k < tokenList.count - 1) {
                                strcat(remaining_commands, " "); // Append a space if not the last token
                            }
                        }
                        i = k;
                    }
                    help_pipe(current_command, remaining_commands); // Handle the pipe logic

                    // Free allocated memory
                    free(remaining_commands);
                    free(current_command);
                    current_command = NULL; // Reset the current command
                    command_start = 0;  // Reset the command index for the next command
                    continue;
                } else if (strchr(tokenList.values[i], '<') != NULL) {
                    help_redirIN(current_command, tokenList.values[i + 1]);
                    i++;
                } else if (strchr(tokenList.values[i], '>') != NULL) {
                    help_redirOUT(current_command, tokenList.values[i + 1]);
                    i++;
                } else {
                    basic_commands(current_command); // Execute the command if nothing redirects to dif command
                }
                free(current_command); // Free the allocated memory
                current_command = NULL; // Reset for the next command
                command_start = 0; // Reset command start index
            }
        } else {
            // Allocate memory for the current command if it doesn't exist
            if (current_command == NULL) {
                current_command = (char *)malloc(MAX_LEN * sizeof(char));
                if (current_command == NULL) {
                    perror("Failed to allocate memory");
                    exit(EXIT_FAILURE);
                }
            }
            strcpy(&current_command[command_start], tokenList.values[i]);
                command_start += strlen(tokenList.values[i]);

            // Add a space between commands if not at the end of a command
            if (i + 1 < tokenList.count
                && strcmp(tokenList.values[i + 1], ";") != 0 && strcmp(tokenList.values[i + 1], "|") != 0
                && strcmp(tokenList.values[i + 1], "<") != 0 && strcmp(tokenList.values[i + 1], ">") != 0) {
                current_command[command_start] = ' ';
                command_start++;
            }
            strcpy(strings[i], tokenList.values[i]);
        }
    }

    // Handle the last command if it exists
    if (current_command != NULL) {
        command_run = 1;
        current_command[command_start] = '\0'; 
        // call basic_commands on the list of tokens
        basic_commands(current_command); // Execute the command if no pipe
        free(current_command); // Free the allocated memory
    }

    free_token_list(&tokenList);
}

// helper function to helper redirect the code in, takes the command that is desired and the file that
// needs to be redirected, no direction needed due to helper being for certain direction
void help_redirIN(char *last_command, char *file) {
    while (*file == ' ') {
        file++;
    }

    // open the file in readonly state
    int input_fd = open(file, O_RDONLY);
    if (input_fd == -1) {
        perror("Failed to open input file");
        return;
    }

    // setting up the child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        close(input_fd);
        return;
    }

    // execute child (the process)
    if (pid == 0) {
        if (dup2(input_fd, STDIN_FILENO) == -1) { // if true redirect failed, if false redirect success and go to close
            perror("Failed to redirect stdin");
            close(input_fd);
            exit(EXIT_FAILURE);
        }
        close(input_fd); // close and exit when done
        basic_commands(last_command);
        exit(EXIT_SUCCESS);
    } else { // if not child wait child finish
        waitpid(pid, NULL, 0);
        close(input_fd);
    }
}

// helper function to helper redirect the code out, takes the command that is desired and the file that
// needs to be redirected, no direction needed due to helper being for certain direction
void help_redirOUT(char *last_command, char *file) {
    while (*file == ' ') {
        file++;
    }

    char filepath[256]; // set size for filepath
    snprintf(filepath, sizeof(filepath), "/workspaces/p1-davidxiao/%s", file); // set filepath for the command

    // Open the output file for writing, create the file if it doesn't exist, truncate it if it does
    FILE *files = fopen(filepath, "w");
    if (files == NULL) {
        perror("Failed to open or create output file");
        return;
    }

    // setting up child process
    pid_t pid = fork();
    if (pid == -1) {
        perror("Fork failed");
        fclose(files);
        return;
    }

    if (pid == 0) { // if in child process
        // Redirect stdout to file
        FILE *files = stdin;
        int fd = fileno(files); // Get the file descriptor from files
        dup2(fd, STDOUT_FILENO);
        // Close and exit after done
        fclose(files);
        basic_commands(last_command);
        exit(EXIT_SUCCESS);
    } else { // if not child wait for it to finish and exit
        waitpid(pid, NULL, 0);
        fclose(files);
    }
}

// Helper function to handle commands with pipes ('|').
// It splits the command into left-hand side and right-hand side and executes them in a pipeline.
void help_pipe(char *lhs_command, char *rhs_command) {
    int pipe_file_descriptors[2];

    // Create a pipe for communication between the two commands
    if (pipe(pipe_file_descriptors) == -1) {
        perror("Pipe failed");
        return;
    }

    // Fork and handle the left-hand side command
    pid_t pid_left = help_lhs_rhs(lhs_command, pipe_file_descriptors, 1);
    // Fork and handle the right-hand side command
    pid_t pid_right = help_lhs_rhs(rhs_command, pipe_file_descriptors, 0);

    // Close the pipe in the parent process
    close(pipe_file_descriptors[0]); // Close read end after forking
    close(pipe_file_descriptors[1]); // Close write end after forking

    // Wait for both child processes to complete
    waitpid(pid_left, NULL, 0);
    waitpid(pid_right, NULL, 0);
}

// Helper function to reduce code duplication when handling left-hand side and right-hand side commands in a pipe.
// It forks a new process and sets up the necessary redirection for piping.
pid_t help_lhs_rhs(char *side_command, int pipe_file_descriptors[], int is_left) {
    pid_t pid_side = fork();
    if (pid_side < 0) {
        perror("Fork failed");
        return -1; // Return -1 on error
    }

    if (pid_side == 0) { // In child process
        if (is_left) {
            // manage left side
            close(pipe_file_descriptors[0]);
            dup2(pipe_file_descriptors[1], STDOUT_FILENO);
        } else {
            // manage right side
            close(pipe_file_descriptors[1]);
            dup2(pipe_file_descriptors[0], STDIN_FILENO);
        }

        // Execute
        basic_commands(side_command);
        exit(EXIT_FAILURE);
    }
    return pid_side; // Return the PID of the child process
}

// helper function to handle null case for prev
void help_prev(char* prev) {
    if (prev == NULL) {
        printf("no prev");
    } else {
        handle_commands(prev); // if not null use handle_commands for prev
    }
}

// main function for the shell
int main(int argc, char **argv) {
    char *input = NULL;
    size_t len = 0;
    char *prev = NULL;

    printf("Welcome to mini-shell.\n");

    while (1) { // loop until exit is called or nothing left
        // mock shell msg
        printf("shell $ ");

        int line = getline(&input, &len, stdin);
        // handle empty
        if (line == -1) {
            printf("\nBye bye.\n");
            free(input);
            free(prev);
            exit(0);
        }

        // check if prev
        int prev_prep = strcmp(input, "prev");
        if (prev_prep == 10) {
            help_prev(prev); // note input is not freed here intentionally since we just call input if prev
            continue;
        }

        // check if exit before calling to handle commands
        int exit_prep = strcmp(input, "exit");
        if (exit_prep == 10) {
            printf("Bye bye.\n");
            free(input);
            free(prev);
            exit(0);
        }

        // Handle multiple commands
        handle_commands(input);  // Handle operators and everything not caught in main

        if (prev != NULL) {
            free(prev); // Free the previous last command
        }

        prev = strdup(input);  // Duplicate input and store it in prev. Using strdup instead of strcpy as I didn't want to malloc
        if (prev == NULL) {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }
    }

    // end cleanup
    free(input);
    return 0;
}