#include "chess.hpp"

extern "C" {
#include "chess.h"
#include "pi_rfid.h"
}

using namespace chess;

static Board *board;
static Movelist *movelist;

void init_chess() {
	board = new Board(constants::STARTPOS);
	movelist = new Movelist;
}

enum move_t valid_move(const char *move) {
    std::string input({'a', '2', 'a', '4'});
	Move moveO = uci::uciToMove(*board, input);
	Square square = moveO.from();
	auto type = board->at<PieceType>(square);
	auto genType = static_cast<PieceGenType>(1 << static_cast<int>(type));
	movegen::legalmoves(*movelist, *board, genType);
	if (std::find(movelist->begin(), movelist->end(), moveO) != movelist->end()) {
		// confirmMove();
		board->makeMove(moveO);
		switch (moveO.typeOf()) {
			case Move::CASTLING:
				return CASTLE;
			case Move::ENPASSANT:
				return EN_PASSANT;
			default:
				return board->isCapture(moveO) ? CAPTURE : VALID;
		}
	}
	return INVALID;
}

int fish_isready(int fish_in_fd, int fish_out_fd, int epollfd, struct epoll_event *events, int timeout) {
	char fish[PIPE_BUF];
	write(fish_in_fd, "isready\n", 8);
	int num_fd = epoll_wait(epollfd, events, 1, timeout);
	if (num_fd != 1) {
		return num_fd;
	}
	read(fish_out_fd, fish, PIPE_BUF);
	if (strstr(fish, "readyok") == nullptr) {
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
	init_chess();
	return write(fish_in_fd, "position startpos\n", 18) == 18;
}

struct moves_t init_moves() {
	struct moves_t moves = {0};
	memmove(moves.pos_cmd, pos_cmd_start, 24);// Intentionally discard NULL terminator
	memmove(moves.moves[0], "\n", 2);
	return moves;
}

game_state gameover() {
	auto gameState = board->isGameOver();
	if (gameState.second != GameResult::NONE) {
		Color whosMove = board->sideToMove();
		switch (gameState.second) {
			case GameResult::WIN:
				if (whosMove == Color::WHITE) {
					return WHITE_WIN;
				} else {
					return BLACK_WIN;
				}
			case GameResult::LOSE:
				if (whosMove == Color::WHITE) {
					return BLACK_WIN;
				} else {
					return WHITE_WIN;
				}
			case GameResult::DRAW:
				return DRAW;
			default:
				return ERROR;
		}
	}
	return ONGOING;
}

char get_capt_type(char tox, char toy) {
	auto type = board->at<PieceType>(Square(Rank(tox), File(toy)));
	switch (type) {
		case (PAWN):
			return 'p';
		case (KNIGHT):
			return 'n';
		case (BISHOP):
			return 'b';
		case (ROOK):
			return 'r';
		case (QUEEN):
			return 'q';
		default:
			return 0;
	}
}