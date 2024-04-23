#ifndef _GNU_SOURCE
#define _GNU_SOURCE// For pipe2, dup2, etc.
#endif
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include "chess.h"
#include "lcd_i2c.h"
#include "pi_gpio.h"
#include "pi_i2c.h"
#include "pi_rfid.h"
#include "pi_spi.h"
#include "stepper.h"

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
	if (wiringPiSetup() == -1) {
		fprintf(stderr, "Could not initialize gpio\n");
		exit(EXIT_FAILURE);
	}

	int i2c_fd = init_i2c(11, 0x27);
	lcd_init(i2c_fd);
	lcd_putc(i2c_fd, '!');
	int rfid_fd = open(pi_spi_device, O_RDWR);
	if (init_rfid(rfid_fd)) {
		fprintf(stderr, "Could not initialize rfid\n");
		exit(EXIT_FAILURE);
	}
	// test_rfid();
	init_motors();
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
	int fish_in_fd = fish_stdin_pipefd[1], fish_out_fd = fish_stdout_pipefd[0];
	char fish[PIPE_BUF] = {0};
	int epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		cleanup_and_die(2, fish_in_fd, fish_out_fd);
	}
	struct epoll_event events, ev = {.events = EPOLLIN, .data.fd = fish_out_fd};
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fish_out_fd, &ev) == -1) {
		perror("epoll_ctl: fish_stdout_pipe");
		cleanup_and_die(3, fish_in_fd, fish_out_fd, epollfd);
	}
	switch (epoll_wait(epollfd, &events, 1, 5000)) {
		case -1:
			perror("epoll_wait");
			cleanup_and_die(3, fish_in_fd, fish_out_fd, epollfd);
		case 0:
			fprintf(stderr, "epoll timeout\n");
			cleanup_and_die(3, fish_in_fd, fish_out_fd, epollfd);
	}
	read(fish_out_fd, fish, PIPE_BUF);// Get rid of Stockfish intro
    write(fish_in_fd, "uci\n", 4);
	if (epoll_wait(epollfd, &events, 1, 500) == -1) {
		perror("epoll_wait");
		cleanup_and_die(3, fish_in_fd, fish_out_fd, epollfd);
	}
	read(fish_out_fd, fish, PIPE_BUF);
	if (strstr(fish, "uciok") == NULL) {
		fprintf(stderr, "Stockfish failed to respond\n");
		cleanup_and_die(3, fish_in_fd, fish_out_fd, epollfd);
	}
	write(fish_in_fd, "setoption name Threads value 4\n", 31);
	write(fish_in_fd, "setoption name Ponder value true\n", 33);
	switch (fish_newgame(fish_in_fd, fish_out_fd, epollfd, &events, 5000)) {
		case -1:
			perror("epoll_wait");
			cleanup_and_die(3, fish_in_fd, fish_out_fd, epollfd);
		case 0:
			fprintf(stderr, "Stockfish failed to respond\n");
			cleanup_and_die(3, fish_in_fd, fish_out_fd, epollfd);
	}
	struct moves_t moves = init_moves();
	char ponder[5] = "abcd";
	bool mate = false;
	while (!mate) {
		get_user_move(&moves);
		if (strncmp(ponder, moves.moves[moves.mov_num - 1], 4) == 0) {
			write(fish_in_fd, "ponderhit\n", 10);
		}
		memset(fish, 0, sizeof(fish));
		fish_sendpos(fish_in_fd, &moves);
		write(fish_in_fd, "go movetime 1000\n", 17);
		switch (epoll_wait(epollfd, &events, 1, 2000)) {
			case -1:
				perror("epoll_wait");
				cleanup_and_die(3, fish_in_fd, fish_out_fd, epollfd);
			case 0:
				fprintf(stderr, "Stockfish unresponsive\n");
				cleanup_and_die(3, fish_in_fd, fish_out_fd, epollfd);
			case 1:
				read(fish_out_fd, fish, PIPE_BUF);
				break;
		}
		// Expected Stockfish output: bestmove 1234 ponder 1234
		size_t adj = strspn(fish, "\r\n");
		memmove(moves.moves[moves.mov_num++], fish + 9 + adj, 5);
		memmove(ponder, fish + 21 + adj, 4);
#ifdef VERBOSE
		printf("Stockfish response: %s\n", fish);
		printf("Stockfish move: %s\n", moves.moves[moves.mov_num - 1]);
		printf("Stockfish ponder: %s\n", ponder);
#endif
		// TODO: Make board move
		mate = gameover(&moves);
	}
	close(fish_in_fd);
	close(fish_out_fd);
	// Maybe call wait()
	return EXIT_SUCCESS;
}
