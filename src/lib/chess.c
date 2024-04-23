#include "chess.h"
#include "main.h"

int fish_isready(int fish_in_fd, int fish_out_fd, int epollfd, struct epoll_event *events, int timeout) {
	char fish[PIPE_BUF];
	write(fish_in_fd, "isready\n", 8);
	int num_fd = epoll_wait(epollfd, events, 1, timeout);
	if (num_fd != 1) {
		return num_fd;
	}
	read(fish_out_fd, fish, PIPE_BUF);
	if (strstr(fish, "readyok") == NULL) {
		return 0;
	}
	return 1;
}

void fish_sendpos(int fish_in_fd, struct moves_t *moves) {
#pragma GCC diagnostic push// From https://nelkinda.com/blog/suppress-warnings-in-gcc-and-clang/
#pragma GCC diagnostic ignored "-Wstringop-overread"// This is intentional to send the command and moves in 1 write
	write(fish_in_fd, moves->pos_cmd, strnlen(moves->pos_cmd, PIPE_BUF));
#pragma GCC diagnostic pop
}

int fish_newgame(int fish_in_fd, int fish_out_fd, int epollfd, struct epoll_event *events, int timeout) {
	write(fish_in_fd, "ucinewgame\n", 11);
	int isready = fish_isready(fish_in_fd, fish_out_fd, epollfd, events, timeout);
	if (isready != 1) {
		return isready;
	}
	return write(fish_in_fd, "position startpos\n", 18) == 18;
}

uint32_t get_user_move(struct moves_t *moves) {
	// TODO: get move for real
	memmove(moves->moves[moves->mov_num++], "e2e4 ", 5);
	memmove(moves->moves[moves->mov_num], "\n", 2);
	return moves->mov_num;
}

struct moves_t init_moves() {
	struct moves_t moves = {0};
	memmove(moves.pos_cmd, pos_cmd_start, 24);// Intentionally discard NULL terminator
	memmove(moves.moves[0], "\n", 2);
	return moves;
}

bool gameover(struct moves_t *moves) {
	// TODO: implement for real
	return moves->mov_num > 10;
}
