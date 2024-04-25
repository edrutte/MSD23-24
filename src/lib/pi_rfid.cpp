#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <unordered_map>
#include <vector>
#include "Adafruit_MFRC630.h"
extern "C" {
#include "pi_gpio.h"
#include "pi_rfid.h"
#include "pi_spi.h"
}

#define DEBUG

#define READ_ATTEMPTS (4)

#define RST    (6)
#define MUX2_EN (21)
#define MUX1_EN (22)
#define SEL3   (23)
#define SEL2   (24)
#define SEL1   (25)

#define CS1    (29)
#define CS2    (28)
#define CS3    (27)
#define CS4    (5)

#define WR1  ()
#define WN1  ()
#define WB1  ()
#define WQ1  ()
#define WK1  ()
#define WB2  ()
#define WN2  ()
#define WR2  ()
#define WP1  (0x048f07d2f06484)
#define WP2  ()
#define WP3  ()
#define WP4  ()
#define WP5  ()
#define WP6  ()
#define WP7  ()
#define WP8  ()
#define BP1  ()
#define BP2  ()
#define BP3  ()
#define BP4  ()
#define BP5  ()
#define BP6  ()
#define BP7  ()
#define BP8  ()
#define BR1  ()
#define BN1  ()
#define BB1  ()
#define BQ1  ()
#define BK1  ()
#define BB2  ()
#define BN2  ()
#define BR2  ()

typedef struct {
	signed char x;
	signed char y;
} Square;

std::unordered_map<long, int> rfid_tag_map;
char rfid_tag_type[32];
int board[8][8] = {-1};
int newBoard[8][8] = {-1};
int rfid_fd = -1;

Adafruit_MFRC630* rfid;

const int squareReadOrder[16] = {1, 6, 9, 15, 0, 7, 8, 13, 2, 5, 11, 12, 3, 4, 10, 14};

void init_mfrc(int8_t cs) {
	digitalWrite(RST, HIGH);
	delay(10);
	digitalWrite(RST, LOW);
	delay(10);
	delete rfid;
	rfid = new Adafruit_MFRC630(rfid_fd, cs, RST);
	rfid->begin();
	rfid->softReset();
	rfid->configRadio(MFRC630_RADIOCFG_ISO1443A_106);
}

int get_tag() {
	// Four attempts to get a valid uid
	for (int i=0; i<READ_ATTEMPTS; i++) {
		uint16_t atqa = 0;

		atqa = rfid->iso14443aRequest();
		/* Looks like we found a tag, move on to selection. */
		if (atqa) {
			uint8_t uid[10] = { 0 };
			uint8_t uidlen;
			uint8_t sak;

			/* Retrieve the UID and SAK values. */
			uidlen = rfid->iso14443aSelect(uid, &sak);

			#ifdef DEBUG
			struct timeval currentTime;
			struct tm *timeinfo;
			gettimeofday(&currentTime, NULL);
			time_t rawtime = currentTime.tv_sec;
			timeinfo = localtime(&rawtime);
			long milliseconds = currentTime.tv_usec / 1000; // Convert microseconds to milliseconds
			printf("%02d:%02d:%02d.%03ld: ", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, milliseconds);
			
			printf("Found a tag with UUID ");
			for (uint8_t i = 0; i < uidlen; i++) {
				printf("%x ", uid[i]);
			}
			printf("\n");
			#endif
			
			long tag_id = 0;
			if (uidlen == 7) {
				for (uint8_t i = uidlen; i > 0; i--) {
					tag_id = tag_id << 8;
					tag_id |= uid[i-1];
				}

				return rfid_tag_map[tag_id];
			}
		}

		delay(10);
	}
	
	return -1;
}
		
void populateCorner(int boardArray[4][4]) {
	for (int i=0; i<16; i++) {
		int ant_sel = squareReadOrder[i];

		digitalWrite(MUX2_EN, (ant_sel/8 % 2) ? LOW : HIGH);
		digitalWrite(MUX1_EN, (ant_sel/8 % 2) ? HIGH : LOW);
		digitalWrite(SEL3, (ant_sel/4 % 2) ? HIGH : LOW);
		digitalWrite(SEL2, (ant_sel/2 % 2) ? HIGH : LOW);
		digitalWrite(SEL1, (ant_sel   % 2) ? HIGH : LOW);

		delay(10);

		boardArray[i/4][i%4] = get_tag();
	}
}

void populateBoard(int boardArray[8][8]) {
	int boardCorner[4][4];
	
	init_mfrc(CS1);
	populateCorner(boardCorner);
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			boardArray[i][j] = boardCorner[i][j];
		}
	}

	init_mfrc(CS2);
	populateCorner(boardCorner);
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			boardArray[i+4][j] = boardCorner[i][j];
		}
	}

	init_mfrc(CS3);
	populateCorner(boardCorner);
	for (int i=0; i<4; i++) {
		for (int j=0; j<4; j++) {
			boardArray[i][j+4] = boardCorner[3-i][3-j];
		}
	}

	// init_mfrc(CS4);
	// populateCorner(boardCorner);
	// for (int i=0; i<4; i++) {
	// 	for (int j=0; j<4; j++) {
	// 		boardArray[i+4][j+4] = boardCorner[3-i][3-j];
	// 	}
	// }

	delete rfid;
	digitalWrite(RST, HIGH);
	delay(50);
}

// Use this to get the move made by the player
void getMove(signed char* moveArray) {
	std::vector<Square> newEmptySquares;
	std::vector<Square> newOccupiedSquares;
	std::vector<Square> newCaptures;

	populateBoard(newBoard);

	// Get all changes in each square
	for (signed char i=0; i<8; i++) {
		for (signed char j=0; j<8; j++) {
			if (board[i][j] != newBoard[i][j]) {
				Square thisSquare = {i, j};
				if (newBoard[i][j] == -1) {
					newEmptySquares.push_back(thisSquare);
				} else if (board[i][j] == -1) {
					newOccupiedSquares.push_back(thisSquare);
				} else {
					newCaptures.push_back(thisSquare);
				}
			}
		}
	}

	int lenEmptySquares = newEmptySquares.size();
	int lenOccupiedSquares = newOccupiedSquares.size();
	int lenCaptures = newCaptures.size();

	Square from = {-1, -1};
	Square to = {-1, -1};
	moveArray[0] = -1;
	moveArray[1] = -1;
	moveArray[2] = -1;
	moveArray[3] = -1;

	// Analyze changed squares to determine which type of move is being made
	if ((lenEmptySquares == 2) && (lenOccupiedSquares == 2) && (lenCaptures == 0)) {
		// Castle
		Square oldPos1 = newEmptySquares[0];
		Square oldPos2 = newEmptySquares[1];
		int oldPiece1 = board[oldPos1.x][oldPos1.y];
		int oldPiece2 = board[oldPos2.x][oldPos2.y];
		char oldPiece1Type = rfid_tag_type[oldPiece1];
		char oldPiece2Type = rfid_tag_type[oldPiece2];

		// Assert that one of the moves is the king
		if ((oldPiece1Type != 'k') && (oldPiece2Type != 'k')) {
			return;
		}

		Square kingSquare = (oldPiece1Type == 'k') ? oldPos1 : oldPos2;
		Square rookSquare = (oldPiece1Type == 'k') ? oldPos2 : oldPos1;

		// Assert that the other piece is a rook
		if (rfid_tag_type[board[rookSquare.x][rookSquare.y]] != 'r') {
			return;
		}
		
		// Assert that the king was in the correct spot
		if ((kingSquare.x != 4) || ((kingSquare.y != 0) && (kingSquare.y != 7))) {
			return;
		}
	
		Square newPos1 = newEmptySquares[0];
		Square newPos2 = newEmptySquares[1];
		char newPiece1Type = rfid_tag_type[newBoard[newPos1.x][newPos1.y]];
		char newPiece2Type = rfid_tag_type[newBoard[newPos1.x][newPos1.y]];

		// Assert that one of the new pieces is a king
		if ((newPiece1Type != 'k') && (newPiece2Type != 'k')) {
			return;
		}
		Square newKingSquare = (newPiece1Type == 'k') ? newPos1 : newPos2;
		Square newRookSquare = (newPiece1Type == 'k') ? newPos2 : newPos1;
		
		// Assert that the other piece is a rook
		if (rfid_tag_type[board[newRookSquare.x][newRookSquare.y]] != 'r') {
			return;
		}

		// Assert that the king is moving into one of two spots
		if ((newKingSquare.x != 2) && (newKingSquare.x != 6)) {
			return;
		}

		// Assert that the rook is in one of two spots
		if (newKingSquare.x == 2) {
			// Queenside castle
			if (newRookSquare.x != 3) {
				return;
			}
		} else {
			// Kingside castle
			if (newRookSquare.x != 5) {
				return;
			}
		}

		// Assert that the rook stays in the same row
		if (rookSquare.y != newRookSquare.y) {
			return;
		}

		// Valid move
		from = kingSquare;
		to = newKingSquare;
	} else if ((lenEmptySquares == 2) && (lenOccupiedSquares == 1) && (lenCaptures == 0)) {
		// En passant
		// Both empty squares must have been pawns
		Square oldPos1 = newEmptySquares[0];
		Square oldPos2 = newEmptySquares[1];
		int oldPiece1 = board[oldPos1.x][oldPos1.y];
		int oldPiece2 = board[oldPos2.x][oldPos2.y];

		if ((rfid_tag_type[oldPiece1] != 'p') || (rfid_tag_type[oldPiece2] != 'p') || (oldPos1.y != oldPos2.y)) {
			// Captured pawn must be in the adjacent position in the same row ----------------^
			return;
		}

		Square newPos = newOccupiedSquares[0];
		bool piece1Moving = (oldPiece1 == newBoard[newPos.x][newPos.y]);

		// Captured pawn must be in the adjacent position in the same column
		if (piece1Moving) {
			if (newPos.x != oldPos2.x) {
				return;
			}
			// Valid move
			from = oldPos1;
		} else {
			if (newPos.x != oldPos1.x) {
				return;
			}
			// Valid move
			from = oldPos2;
		}
		to = newPos;
	} else if ((lenEmptySquares == 1) && (lenOccupiedSquares == 0) && (lenCaptures == 1)) {
		// Capture
		// No validation required
		from = newEmptySquares[0];
		to = newCaptures[0];
	} else if ((lenEmptySquares == 1) && (lenOccupiedSquares == 1) && (lenCaptures == 0)) {
		// Move
		// No validation required
		from = newEmptySquares[0];
		to = newOccupiedSquares[0];
	}

	// Fill in return values
	moveArray[0] = from.x;
	moveArray[1] = from.y;
	moveArray[2] = to.x;
	moveArray[3] = to.y;

	// Promotion type must still be handled.
	return;
}

// Call this after each move has been confirmed to be valid
void confirmMove() {
	// Update board
	for (char i=0; i<8; i++) {
		for (char j=0; j<8; j++) {
			board[i][j] = newBoard[i][j];
			newBoard[i][j] = -1;
		}
	}
}

// Use this when stockfish makes a move
void movePiece(signed char* moveArray) {
	Square from = {moveArray[0], moveArray[1]};
	Square to = {moveArray[2], moveArray[3]};

	if ((to.x == -1) || (to.y == -1)) {
		board[to.x][to.y] = board[from.x][from.y];
	} // only not entered when removing an en passant capture
	
	board[from.x][from.y] = -1;
}

int init_rfid(int fd) {
	rfid_tag_map[WP1] = 0;
	rfid_tag_type[rfid_tag_map[WP1]] = 'p';

	pinMode(RST, OUTPUT);
	pinMode(MUX2_EN, OUTPUT);
	pinMode(MUX1_EN, OUTPUT);
	pinMode(SEL3, OUTPUT);
	pinMode(SEL2, OUTPUT);
	pinMode(SEL1, OUTPUT);

	pinMode(CS1, OUTPUT);
	pinMode(CS2, OUTPUT);
	pinMode(CS3, OUTPUT);
	pinMode(CS4, OUTPUT);

	digitalWrite(CS1, HIGH);
	digitalWrite(CS2, HIGH);
	digitalWrite(CS3, HIGH);
	digitalWrite(CS4, HIGH);

	rfid_fd = fd;

	rfid = new Adafruit_MFRC630(rfid_fd, CS1, RST);
	if (!(rfid->begin())) {
		return 1;
	}
	delete rfid;

	rfid = new Adafruit_MFRC630(rfid_fd, CS2, RST);
	if (!(rfid->begin())) {
		return 2;
	}
	delete rfid;

	rfid = new Adafruit_MFRC630(rfid_fd, CS3, RST);
	if (!(rfid->begin())) {
		return 3;
	}
	// delete rfid;

	// rfid = new Adafruit_MFRC630(rfid_fd, CS4, RST);
	// if (!(rfid->begin())) {
	// 	return 1;
	// }
	// delete rfid;
		
	// Get initial board
	populateBoard(board);

	printf("RFID Initialized\n");

	return 0;
}

// void test_rfid() {
// 	uint8_t ant_sel = 0;
// 	uint8_t board_sel = 0;
// 	printf("Waiting to read a tag...\n");

// 	while (true) {
// 		if (ant_sel == 0) {
// 			// Reset the respective boards
// 			switch (board_sel) {
// 			case 0:
// 				// reinit_rfid(rfid2);
// 				init_mfrc(CS1);
// 				break;
// 			case 1:
// 				// reinit_rfid(rfid3);
// 				init_mfrc(CS2);
// 				break;
// 			default:
// 				// reinit_rfid(rfid1);
// 				init_mfrc(CS3);
// 				break;
// 			}
// 		}

// 		digitalWrite(MUX2_EN, (ant_sel/8 % 2) ? LOW : HIGH);
// 		digitalWrite(MUX1_EN, (ant_sel/8 % 2) ? HIGH : LOW);
// 		digitalWrite(SEL3, (ant_sel/4 % 2) ? HIGH : LOW);
// 		digitalWrite(SEL2, (ant_sel/2 % 2) ? HIGH : LOW);
// 		digitalWrite(SEL1, (ant_sel   % 2) ? HIGH : LOW);

// 		delay(10);

// 		// print_device(rfid, ant_sel + 16*board_sel);
		
// 		ant_sel += 1;
// 		if (ant_sel >= 16) {
// 			ant_sel = 0;

// 			board_sel += 1;
// 			board_sel %= 3;
// 		}
// 	}

// 	delete rfid;
// 	digitalWrite(RST, HIGH);
// 	delay(50);
// }
