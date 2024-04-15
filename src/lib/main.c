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
#include <sys/epoll.h>
#include <unistd.h>
#include "main.h"
#include "lcd_i2c.h"
#include "pi_gpio.h"
#include "pi_i2c.h"
#include "pi_rfid.h"
#include "pi_spi.h"

#ifdef VERBOSE
#define read(...) printf("read %ld bytes on line %u\n", read(__VA_ARGS__), __LINE__)
#define write(...) printf("wrote %ld bytes on line %u\n", write(__VA_ARGS__), __LINE__)
#endif

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
#ifdef __aarch64__
	if (wiringPiSetup() == -1) {
		fprintf(stderr, "Could not initialize gpio\n");
		exit(EXIT_FAILURE);
	}
	int i2c_fd = init_i2c(11, 0x27);
	lcd_init(i2c_fd);
	lcd_putc(i2c_fd, '!');
	debug_block_until_tag_and_dump();
#endif
    if (signal(SIGCHLD, SIG_DFL) == SIG_ERR) {
        perror("signal");
        exit(EXIT_FAILURE);
    }
    if (pipe2(fish_stdin_pipefd, 0) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    if (pipe2(fish_stdout_pipefd, 0) == -1) {
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
		    close(fish_stdout_pipefd[1]);
		    close(fish_stdin_pipefd[0]);
            execle(argv[1], argv[1], NULL, NULL);
            perror("execle");
		    cleanup_and_die(2, fish_stdin_pipefd[0], fish_stdout_pipefd[1]);
        default:
	        close(fish_stdout_pipefd[1]);
		    close(fish_stdin_pipefd[0]);
    }
	char fish[PIPE_BUF] = {0};
	int epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		cleanup_and_die(2, fish_stdin_pipefd[1], fish_stdout_pipefd[0]);
	}
	struct epoll_event events, ev = {.events = EPOLLIN, .data.fd = fish_stdout_pipefd[0]};
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fish_stdout_pipefd[0], &ev) == -1) {
		perror("epoll_ctl: fish_stdout_pipe");
		cleanup_and_die(3, fish_stdin_pipefd[1], fish_stdout_pipefd[0], epollfd);
	}
	switch (epoll_wait(epollfd, &events, 1, 5000)) {
		case -1:
			perror("epoll_wait");
			cleanup_and_die(3, fish_stdin_pipefd[1], fish_stdout_pipefd[0], epollfd);
		case 0:
			fprintf(stderr, "epoll timeout\n");
			cleanup_and_die(3, fish_stdin_pipefd[1], fish_stdout_pipefd[0], epollfd);
	}
	read(fish_stdout_pipefd[0], fish, PIPE_BUF);// Get rid of Stockfish intro
    write(fish_stdin_pipefd[1], "uci\n", 4);
	if (epoll_wait(epollfd, &events, 1, 500) == -1) {
		perror("epoll_wait");
		cleanup_and_die(3, fish_stdin_pipefd[1], fish_stdout_pipefd[0], epollfd);
	}
	read(fish_stdout_pipefd[0], fish, PIPE_BUF);
	if (strstr(fish, "uciok") == NULL) {
		fprintf(stderr, "Stockfish failed to respond\n");
		cleanup_and_die(3, fish_stdin_pipefd[1], fish_stdout_pipefd[0], epollfd);
	}
	write(fish_stdin_pipefd[1], "setoption name Threads value 4\n", 31);
	write(fish_stdin_pipefd[1], "setoption name Ponder value true\n", 33);
	write(fish_stdin_pipefd[1], "ucinewgame\n", 11);
	// TODO: Extract into a stockfish_isready function
	write(fish_stdin_pipefd[1], "isready\n", 8);
	switch (epoll_wait(epollfd, &events, 1, 5000)) {
		case -1:
			perror("epoll_wait");
			cleanup_and_die(3, fish_stdin_pipefd[1], fish_stdout_pipefd[0], epollfd);
		case 0:
			fprintf(stderr, "epoll timeout\n");
			cleanup_and_die(3, fish_stdin_pipefd[1], fish_stdout_pipefd[0], epollfd);
	}
	read(fish_stdout_pipefd[0], fish, PIPE_BUF);
	if (strstr(fish, "readyok") == NULL) {
		fprintf(stderr, "Stockfish failed to respond\n");
		cleanup_and_die(3, fish_stdin_pipefd[1], fish_stdout_pipefd[0], epollfd);
	}
	// End TODO
	write(fish_stdin_pipefd[1], "position startpos\n", 18);
	close(fish_stdin_pipefd[1]);
	close(fish_stdout_pipefd[0]);
	// Maybe call wait()
	return EXIT_SUCCESS;
}
