#ifndef CHESS_CHESS_H
#define CHESS_CHESS_H

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

static const char pos_cmd_start[] = "position startpos moves ";

struct __attribute__((packed)) moves_t {// Pack to ensure no padding between pos_cmd and moves
	uint32_t mov_num;
	char pos_cmd[24];
	char moves[(PIPE_BUF - 24) / 5][5];
};

enum game_state {
	BLACK_WIN,
	WHITE_WIN,
	DRAW,
	ONGOING,
	ERROR
};

enum move_t {
	INVALID = 0,
	VALID,
	CAPTURE,
	CASTLE,
	EN_PASSANT
};

void init_chess();
enum move_t valid_move(const char *move);
int fish_isready(int fish_in_fd, int fish_out_fd, int epollfd, struct epoll_event *events, int timeout);
void fish_sendpos(int fish_in_fd, struct moves_t *moves);
int fish_newgame(int fish_in_fd, int fish_out_fd, int epollfd, struct epoll_event *events, int timeout);
struct moves_t init_moves();
enum game_state gameover();
char get_capt_type(char tox, char toy);

#endif //CHESS_CHESS_H
