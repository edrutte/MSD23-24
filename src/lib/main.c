#ifndef _GNU_SOURCE
#define _GNU_SOURCE// For pipe2, dup2, etc.
#endif
#include <fcntl.h>
#include <gpiod.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "main.h"

/**
 * Parts taken from https://stackoverflow.com/questions/34348111/c-redirecting-stdout-after-forking-of-child-from-parent
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char* argv[]) {
    static char *fishargv[] = {NULL};
    pid_t pid;
    int fish_stdin_pipefd[2], fish_stdout_pipefd[2];
	printf("Hello World\n");
    if (argc < 2) {
        printf("Please supply path to stockfish binary\n");
        exit(EXIT_FAILURE);
    }
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }
    if (pipe2(fish_stdin_pipefd, O_NONBLOCK) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    if (pipe2(fish_stdout_pipefd, O_NONBLOCK) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    pid = fork();
    switch (pid) {
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);
        case 0:
            if (dup2(fish_stdin_pipefd[0], STDIN_FILENO) == -1) {
                perror("dup");
                exit(EXIT_FAILURE);
            }
            if (dup2(fish_stdout_pipefd[1], STDOUT_FILENO) == -1) {
                perror("dup");
                exit(EXIT_FAILURE);
            }
            execv(argv[1], fishargv);
            perror("execv");
            exit(EXIT_FAILURE);
        default:
            break;
    }
    write(fish_stdin_pipefd[1], "uci", 3);
    nanosleep(&pipe_delay, NULL);
    char fish[256] = {0};
	ssize_t n_read = read(fish_stdout_pipefd[0], fish, 256);
	while (n_read > 0) {
		printf("%.*s", (int) n_read, fish);
		n_read = read(fish_stdout_pipefd[0], fish, 256);
	}
    // Maybe call wait()
	return EXIT_SUCCESS;
}
