#ifndef CHESS_CHESS_H
#define CHESS_CHESS_H

#include <limits.h>
#include <stdint.h>
#include <sys/epoll.h>

static const char pos_cmd_start[] = "position startpos moves ";

struct __attribute__((packed)) moves_t {// Pack to ensure no padding between pos_cmd and moves
	uint32_t mov_num;
	char pos_cmd[24];
	char moves[(PIPE_BUF - 24) / 5][5];
};

int fish_isready(int fish_in_fd, int fish_out_fd, int epollfd, struct epoll_event *events, int timeout);
void fish_send_pos(int fish_in_fd, struct moves_t *moves);
uint32_t get_user_move(struct moves_t *moves);
struct moves_t init_moves();

#endif //CHESS_CHESS_H
