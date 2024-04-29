#ifndef CHESS_PI_RFID_H
#define CHESS_PI_RFID_H

int init_rfid(int rfid_fd);
void movePiece(signed char* moveArray);
void getMove(signed char* moveArray);
void confirmMove();
void test_rfid();

#endif //CHESS_PI_RFID_H
