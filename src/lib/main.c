#ifndef _GNU_SOURCE
#define _GNU_SOURCE// For pipe2, dup2, etc.
#endif
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "main.h"
#include "pi5_gpio.h"

_Noreturn void cleanup_and_die(int num_fd, ...) {
	va_list to_close;
	va_start(to_close, num_fd);
	for (int i = 0; i < num_fd; i++) {
		close(va_arg(to_close, int));
	}
	exit(EXIT_FAILURE);
}

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
    if (pipe2(fish_stdout_pipefd, O_DIRECT) == -1) {
        perror("pipe");
	    cleanup_and_die(2, fish_stdin_pipefd[0], fish_stdin_pipefd[1]);
    }
    pid = fork();
    switch (pid) {
        case -1:
            perror("fork");
		    cleanup_and_die(4, fish_stdin_pipefd[0], fish_stdin_pipefd[1], fish_stdout_pipefd[0], fish_stdout_pipefd[1]);
        case 0:
			close(fish_stdout_pipefd[0]);
		    close(fish_stdin_pipefd[1]);
            if (dup2(fish_stdin_pipefd[0], STDIN_FILENO) == -1) {
                perror("dup");
	            cleanup_and_die(2, fish_stdin_pipefd[0], fish_stdout_pipefd[1]);
            }
            if (dup2(fish_stdout_pipefd[1], STDOUT_FILENO) == -1) {
                perror("dup");
	            cleanup_and_die(2, fish_stdin_pipefd[0], fish_stdout_pipefd[1]);
            }
            execv(argv[1], fishargv);
            perror("execv");
		    cleanup_and_die(2, fish_stdin_pipefd[0], fish_stdout_pipefd[1]);
        default:
	        close(fish_stdout_pipefd[1]);
		    close(fish_stdin_pipefd[0]);
    }
	char fish[PIPE_BUF] = {0};
	read(fish_stdout_pipefd[0], fish, 1);// Get rid of Stockfish intro
    write(fish_stdin_pipefd[1], "uci", 3);
	read(fish_stdout_pipefd[0], fish, PIPE_BUF);
	if (strstr(fish, "uciok") == NULL) {
		fprintf(stderr, "Stockfish failed to respond\n");
		cleanup_and_die(2, fish_stdin_pipefd[1], fish_stdout_pipefd[0]);
	}
	struct gpiod_chip *gpio_chip;
	if (init_gpio(gpio_chip)) {
		fprintf(stderr, "Could not initialize gpio\n");
		cleanup_and_die(2, fish_stdin_pipefd[1], fish_stdout_pipefd[0]);
	}
    close(fish_stdin_pipefd[1]);
	close(fish_stdout_pipefd[0]);
	// Maybe call wait()
	return EXIT_SUCCESS;
}
