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


#include "../chess-library/include/chess.hpp"

using namespace chess;

int main() {
    Board board = Board(constants::STARTPOS);

    Movelist moves;

    std::string bestmove;
    std::string ponder;


    // Stockfish
    int pipe_in[2]; // Pipe for writing to child process
    int pipe_out[2]; // Pipe for reading from child process
    pid_t pid;
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];

    // Create pipes
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1) {
        std::cerr << "Pipe creation failed.\n";
        return 1;
    }

    // Fork the process
    pid = fork();
    if (pid < 0) {
        std::cerr << "Fork failed.\n";
        return 1;
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
        std::cerr << "Execve failed.\n";
        return 1;
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
        const std::vector<std::string> commands = {"uci", "setoption name Threads value 4", "isready"};
        for (const auto& command : commands) {
            write(pipe_in[1], command.c_str(), command.size());
            write(pipe_in[1], "\n", 1);
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
                std::cerr << "Timed out waiting for 'readyok'.\n";
                return 1;
            }

            // Poll for input and output from child process
            if (poll(fds, 2, -1) == -1) {
                std::cerr << "Poll failed.\n";
                break;
            }

            // Check if output is available from child process
            if (fds[1].revents & POLLIN) {
                int bytes_read = read(pipe_out[0], buffer, BUFFER_SIZE);
                if (bytes_read <= 0) {
                    std::cerr << "Error reading from child process.\n";
                    break;
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
                std::cerr << "Child process terminated unexpectedly.\n";
                return 1;
            }
        }
        
        std::cout << "Stockfish is ready." << std::endl;

        while (true) { // Continue indefinitely until an error or termination occurs
            // Poll for input from stdin and output from child process with a timeout
            if (poll(fds, 2, -1) == -1) {
                std::cerr << "Poll failed.\n";
                break;
            }

            // Check if input is available from stdin
            if (fds[0].revents & POLLIN) {
                std::string input;
                std::getline(std::cin, input);

                if (input[0] == ':') {
                    // Get the move object for the desired move
                    Move move = uci::uciToMove(board, input.substr(1));

                    // Get the square from which a piece is moving
                    Square square = move.from();

                    // Get the type of piece at this square
                    PieceType type = board.at<PieceType>(square);
                    PieceGenType genType = static_cast<PieceGenType>(1 << static_cast<int>(type));

                    // Get the list of all available moves for that piece
                    movegen::legalmoves(moves, board, genType);

                    // Check to make sure our move is in that list of moves
                    if (moves.find(move) < 0) {
                        std::cout << "Invalid move." << std::endl;
                    } else {
                        // Make the move on the board
                        board.makeMove(move);

                        // Send the updated board to Stockfish
                        std::string fen = board.getFen();
                        std::string command = "position fen " + fen;
                        std::cout << command << std::endl;

                        write(pipe_in[1], command.c_str(), command.size());
                        write(pipe_in[1], "\n", 1);
                        
                        command = "go movetime 100";

                        write(pipe_in[1], command.c_str(), command.size());
                        write(pipe_in[1], "\n", 1);
                    }
                } else {
                    // Write to the child process
                    write(pipe_in[1], input.c_str(), input.size());
                    write(pipe_in[1], "\n", 1);
                }

            }

            // Check if output is available from child process
            if (fds[1].revents & POLLIN) {
                int bytes_read = read(pipe_out[0], buffer, BUFFER_SIZE);
                if (bytes_read <= 0) {
                    std::cerr << "Error reading from child process.\n";
                    break;
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
                    if (bufferStr.length() > 26) { // TODO: remove
                        std::cout << "ITS HAPPENING: " << bufferStr << std::endl;
                        return 0;
                    }

                    // Get the best move and ponder move
                    std::istringstream iss(bufferStr);
                    std::vector<std::string> tokens;

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

                    // Make the move on the board
                    board.makeMove(move);

                    // Send the updated board to Stockfish
                    std::string fen = board.getFen();
                    std::string command = "position fen " + fen; // + " moves " + ponder;
                    std::cout << command << std::endl;
                    
                    write(pipe_in[1], command.c_str(), command.size());
                    write(pipe_in[1], "\n", 1);

                    command = "d";
                    write(pipe_in[1], command.c_str(), command.size());
                    write(pipe_in[1], "\n", 1);

                    command = "go movetime 50";
                    write(pipe_in[1], command.c_str(), command.size());
                    write(pipe_in[1], "\n", 1);
                }
            }

            // Check if child process has terminated
            int status;
            if (waitpid(pid, &status, WNOHANG) == pid) {
                std::cout << "Child process terminated." << std::endl;
                return 0;
            }
        }
    }

    return 0;
}