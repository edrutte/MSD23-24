#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include <cstring>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#include "../chess-library/include/chess.hpp"

#include <wiringPi.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <wiringSerial.h>

const bool AIvsAI = 1;

using namespace chess;

void displayOutput(std::string* output) {
    std::cout << *output << std::endl;
}

void printToStockfish(int* pipe_in, std::string* command) {
    command->append("\n");
    write(pipe_in[1], command->c_str(), command->size());
}

int checkGameState(Board* board) {
    std::string command;
    std::string output = "";
    auto gameState = board->isGameOver();
    if (gameState.second != GameResult::NONE) {
        output.append("Game over. ");
        Color whosMove = board->sideToMove();

        switch (gameState.second) {
            case GameResult::WIN:
                if (whosMove == Color::WHITE) {
                    output.append("White won ");
                } else {
                    output.append("Black won ");
                }
                break;
            case GameResult::LOSE:
                if (whosMove == Color::WHITE) {
                    output.append("Black won ");
                } else {
                    output.append("White won ");
                }
                break;
            case GameResult::DRAW:
                output.append("Draw ");
                break;
            default:
                output.assign("ERROR: GNO.");
                displayOutput(&output);
                return -1;
        }
    
        switch (gameState.first) {
            case GameResultReason::CHECKMATE:
                output.append("by checkmate!");
                break;
            case GameResultReason::STALEMATE:
                output.append("by stalemate!");
                break;
            case GameResultReason::INSUFFICIENT_MATERIAL:
                output.append("by insufficient material!");
                break;
            case GameResultReason::FIFTY_MOVE_RULE:
                output.append("by fifty move rule!");
                break;
            case GameResultReason::THREEFOLD_REPETITION:
                output.append("by threefold repetition!");
                break;
            default:
                output.assign("ERROR: GNO.");
                displayOutput(&output);
                return -1;
        }
        displayOutput(&output);
        return 1;
    }
    return 0;
}

bool validate_move(Board* board, Move move) {
    Movelist moves;
    
    // Get the square from which a piece is moving
    Square square = move.from();

    // Get the type of piece at this square
    PieceType type = board->at<PieceType>(square);
    PieceGenType genType = static_cast<PieceGenType>(1 << static_cast<int>(type));

    // Get the list of all available moves for that piece
    movegen::legalmoves(moves, *board, genType);

    // Check to make sure our move is in that list of moves
    return !(moves.find(move) < 0);
}

int main() {
    std::string output = "Hello World";
    displayOutput(&output);

    int fd;

    if((fd=serialOpen("/dev/ttyACM0",9600))<0){
        fprintf(stderr,"Unable to open serial device: %s\n",strerror(errno));
        return 1;
    }

    char string[10];
    // strncpy(string, "111399p0\n", 10);
    // delay(8000);
    // for (int i=0; i<9; i++) {
    //     delay(10);
    //     serialPutchar(fd, string[i]);
    // }
    // strncpy(string, "666499p0\n", 10);
    // delay(10000);
    // for (int i=0; i<9; i++) {
    //     delay(10);
    //     serialPutchar(fd, string[i]);
    // }
    // strncpy(string, "102299p0\n", 10);
    // delay(10000);
    // for (int i=0; i<9; i++) {
    //     delay(10);
    //     serialPutchar(fd, string[i]);
    // }
    // // strncpy(string, "576699p0\n", 10);
    // // delay(10000);
    // // for (int i=0; i<9; i++) {
    // //     delay(10);
    // //     serialPutchar(fd, string[i]);
    // // }
    // // strncpy(string, "717399p0\n", 10);
    // // delay(10000);
    // // for (int i=0; i<9; i++) {
    // //     delay(10);
    // //     serialPutchar(fd, string[i]);
    // // }
    // // // strncpy(string, "647373p0\n", 10);
    // // // delay(10000);
    // // // for (int i=0; i<9; i++) {
    // // //     delay(10);
    // // //     serialPutchar(fd, string[i]);
    // // // }
    // // strncpy(string, "737199p0\n", 10);
    // // delay(10000);
    // // for (int i=0; i<9; i++) {
    // //     delay(10);
    // //     serialPutchar(fd, string[i]);
    // // }
    // // strncpy(string, "665799p0\n", 10);
    // // delay(10000);
    // // for (int i=0; i<9; i++) {
    // //     delay(10);
    // //     serialPutchar(fd, string[i]);
    // // }
    // strncpy(string, "221099p0\n", 10);
    // delay(10000);
    // for (int i=0; i<9; i++) {
    //     delay(10);
    //     serialPutchar(fd, string[i]);
    // }
    // strncpy(string, "646699p0\n", 10);
    // delay(10000);
    // for (int i=0; i<9; i++) {
    //     delay(10);
    //     serialPutchar(fd, string[i]);
    // }
    // strncpy(string, "131199p0\n", 10);
    // delay(8000);
    // for (int i=0; i<9; i++) {
    //     delay(10);
    //     serialPutchar(fd, string[i]);
    // }

    Board board = Board(constants::STARTPOS);
    Movelist moves;

    std::string bestmove;
    std::string ponder;

    std::string all_moves = "";

    // Stockfish
    int pipe_in[2]; // Pipe for writing to child process
    int pipe_out[2]; // Pipe for reading from child process
    pid_t pid;
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    // Create pipes
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        std::string output = "ERROR: Pipe creation failed.";
        displayOutput(&output);
        return -1;
    }

    // Fork the process
    pid = fork();
    if (pid < 0) {
        std::string output = "ERROR: Fork failed.";
        displayOutput(&output);
        return -1;
    }

    if (pid == 0) { // Child process
        // Close unused ends of the pipes
        close(pipe_in[1]);
        close(pipe_out[0]);

        // Redirect stdin and stdout to the pipe ends
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);

        // Close the original pipe ends
        close(pipe_in[0]);
        close(pipe_out[1]);

        // Execute the desired program (e.g., /usr/bin/cat)
        char* const argv[] = {"Stockfish/src/stockfish", nullptr};
        execve(argv[0], argv, nullptr);

        // execve should not return if successful, if it does, an error occurred
        std::string output = "ERROR: Execve failed.";
        displayOutput(&output);
        return -1;
    } else { // Parent process
        // Close unused ends of the pipes
        close(pipe_in[0]);
        close(pipe_out[1]);

        // Set up pollfd structures for polling stdin and stdout
        struct pollfd fds[2];
        fds[0].fd = STDIN_FILENO;
        fds[0].events = POLLIN;
        fds[1].fd = pipe_out[0];
        fds[1].events = POLLIN;
        
        // Send "uci" and "ucinewgame" to the child process
        // std::vector<std::string> commands = {"uci", "setoption name Threads value 4", "setoption name UCI_LimitStrength value true", "setoption name UCI_Elo value 1320", "isready"};
        std::vector<std::string> commands = {"uci", "setoption name Threads value 4", "isready"};
        for (auto& command : commands) {
            printToStockfish(pipe_in, &command);
        }

        // Initialize the start time
        std::time_t start_time = std::time(nullptr);

        // Timeout duration in seconds
        const int READYOK_TIMEOUT = 2;

        bool readyokReceived = false;
        while (!readyokReceived) { // Continue indefinitely until we receive 'readyok'
            // Calculate elapsed time
            std::time_t elapsed_time = std::time(nullptr) - start_time;
            if (elapsed_time > READYOK_TIMEOUT) {
                std::string output = "ERROR: Stockfish timeout.";
                displayOutput(&output);
                return -1;
            }

            // Poll for input and output from child process
            if (poll(fds, 2, -1) == -1) {
                std::string output = "ERROR: Poll failed.";
                displayOutput(&output);
                return -1;
            }

            // Check if output is available from child process
            if (fds[1].revents & POLLIN) {
                int bytes_read = read(pipe_out[0], buffer, BUFFER_SIZE);
                if (bytes_read <= 0) {
                    std::string output = "ERROR: No read child.";
                    displayOutput(&output);
                    return -1;
                }
                // Output received from child process
                // Check if "readyok" is received
                const char* readyOkStr = "readyok";
                if (strncmp(buffer, readyOkStr, 7) == 0) {
                    readyokReceived = true;
                }
            }

            // Check if child process has terminated
            int status;
            if (waitpid(pid, &status, WNOHANG) == pid) {
                std::string output = "ERROR: Stockfish closed.";
                displayOutput(&output);
                return -1;
            }
        }
        
        std::string output = "Stockfish is ready.";
        displayOutput(&output);

        while (true) { // Continue indefinitely until an error or termination occurs
            // Poll for input from stdin and output from child process with a timeout
            if (poll(fds, 2, -1) == -1) {
                std::string output = "ERROR: Poll failed.";
                displayOutput(&output);
                return -1;
            }

            // Check if input is available from stdin
            if (fds[0].revents & POLLIN) {
                std::string input;
                std::getline(std::cin, input);
                while (input.size() != 5) {
                    std::cout << "Invalid move" << std::endl;
                    std::getline(std::cin, input);
                }
                

                if (input[0] == ':') {
                    // Get the move object for the desired move
                    Move move = uci::uciToMove(board, input.substr(1));

                    bool valid_move = validate_move(&board, move);

                    // Check to make sure our move is in that list of moves
                    if (!valid_move) {
                        std::cout << "Invalid move." << std::endl;
                        std::string output = "Invalid move.";
                        displayOutput(&output);
                    } else {
                        // Make the move on the board
                        board.makeMove(move);

                        // Send the updated board to Stockfish
                        // std::string fen = board.getFen();
                        // std::string command = "position fen " + fen;
                        all_moves.append(" " + input.substr(1));
                        std::string command = "position startpos moves" + all_moves;
                        std::cout << command << std::endl;
                        printToStockfish(pipe_in, &command);
                        
                        command = "go movetime 10000";
                        printToStockfish(pipe_in, &command);
                    }
                } else {
                    // Write to the child process
                    printToStockfish(pipe_in, &input);
                }

            }

            // Check if output is available from child process
            if (fds[1].revents & POLLIN) {
                int bytes_read = read(pipe_out[0], buffer, BUFFER_SIZE);
                if (bytes_read <= 0) {
                    std::cerr << "Error reading from child process." << std::endl;
                    std::string output = "ERROR: No read child.";
                    displayOutput(&output);
                    return -1;
                }
                // Output received from child process

                // Print only lines that are not 'info'
                std::string bufferStr(buffer);
                bufferStr.assign(bufferStr.substr(0, bytes_read));
                while (bufferStr.length() > 0) {
                    int bestmovePos = bufferStr.find("\n", 0);
                    std::string bufferSubstr;
                    if (bestmovePos != std::string::npos) {
                        bufferSubstr.assign(bufferStr.substr(0, bestmovePos));
                        if (bufferStr.length() > (bestmovePos + 1)) {
                            bufferStr.assign(bufferStr.substr(bestmovePos + 1));
                        } else {
                            bufferStr.clear();
                        }
                    } else {
                        bufferSubstr.assign(bufferStr);
                        bufferStr.clear();
                    }
                    if (bufferSubstr.substr(0, 4) != "info") {
                        std::cout << bufferSubstr << std::endl;
                        std::cout.flush();
                    }
                }
                
                // Check if "bestmove" is received
                bufferStr.assign(buffer);
                int bestmovePos = bufferStr.find("bestmove", 0);
                if (bestmovePos != std::string::npos) {
                    // Remove all of the data after the first newline
                    bufferStr = bufferStr.substr(bestmovePos);
                    int newlinePos = bufferStr.find("\n", 0);
                    if (newlinePos != std::string::npos) {
                        bufferStr = bufferStr.substr(0, newlinePos);
                    }

                    // Get the best move and ponder move
                    std::istringstream iss(bufferStr);
                    std::vector<std::string> tokens;
                    // std::string stuff = "e5";
                    // Square sqr(stuff);
                    // std::cout << board.at<PieceType>(sqr) << std::endl;
                    

                    std::string token;
                    while (std::getline(iss, token, ' ')) {
                        tokens.push_back(token);
                    }
                    bestmove.assign(tokens[1]);
                    if (tokens.size() > 2) {
                        ponder.assign(tokens[3]);
                    } else {
                        ponder.clear();
                    }

                    // Get the move object for the desired move
                    Move move = uci::uciToMove(board, bestmove);

                    // Square sq_to = move.to();
                    // Get the type of piece at this square
                    std::string strat = "e4";
                    Piece piece = board.at(move.to());

                    string[0] = bestmove.at(0) - 'a' + '0';
                    string[1] = bestmove.at(1) - '1' + '0';
                    string[2] = bestmove.at(2) - 'a' + '0';
                    string[3] = bestmove.at(3) - '1' + '0';
                    if (board.at(move.to()) != Piece::NONE && move.typeOf() != Move::CASTLING) {
                        printf("capturing\n");
                        // Capturing
                        string[4] = bestmove.at(2) - 'a' + '0';
                        string[5] = bestmove.at(3) - '1' + '0';
                        if (piece.type() == PieceType::PAWN) {
                            string[6] = 'p';
                        } else if (piece.type() == PieceType::KNIGHT) {
                            string[6] = 'n';
                        } else if (piece.type() == PieceType::BISHOP) {
                            string[6] = 'b';
                        } else if (piece.type() == PieceType::ROOK) {
                            string[6] = 'r';
                        } else if (piece.type() == PieceType::QUEEN) {
                            string[6] = 'q';
                        } else {
                            string[6] = 'p';
                        }
                        string[7] = '0';
                    } else {
                        string[4] = '9';
                        string[5] = '9';
                        string[6] = 'p';
                        if (move.typeOf() == Move::CASTLING) {
                            string[7] = '1';
                        } else {
                            string[7] = '0';
                        }
                    }
                    string[8] = '\n';
                    string[9] = 0;

                    printf("%s", string);

                    for (int i=0; i<9; i++) {
                        delay(10);
                        serialPutchar(fd, string[i]);
                    }
                    delay(8000);

                    // Make the move on the board
                    board.makeMove(move);

                    // Send the updated board to Stockfish
                    // std::string fen = board.getFen();
                    // std::string command = "position fen " + fen; // + " moves " + ponder;
                    all_moves.append(" " + bestmove);
                    std::string command = "position startpos moves" + all_moves;
                    std::cout << command << std::endl;
                    printToStockfish(pipe_in, &command);

                    command = "d";
                    printToStockfish(pipe_in, &command);
                    
                    // Check end of game
                    int endOfGame = checkGameState(&board);
                    switch (endOfGame) {
                        case 1:
                            // Close stockfish
                            command = "quit";
                            printToStockfish(pipe_in, &command);
                            return 0;
                        case -1:
                            return -1;
                        default:
                            if (AIvsAI) {
                                command = "go movetime 10000";
                                printToStockfish(pipe_in, &command);
                            }
                            break;
                    }
                }
            }

            // Check if child process has terminated
            int status;
            if (waitpid(pid, &status, WNOHANG) == pid) {
                std::cout << "Child process terminated." << std::endl;
                std::string output = "ERROR: Stockfish closed.";
                displayOutput(&output);
                return -1;
            }
        }
    }

    return 0;
}