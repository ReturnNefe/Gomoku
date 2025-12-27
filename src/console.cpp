#include "headers/ai.h"
#include <iostream>
#include <string>
#include <limits>

class ConsoleGame {
private:
    Board board;
    AI ai;

    void printBoard(Point lastMove) const {
        std::cout << "\033[2J\033[H";
        std::cout.flush();

        std::cout << "   ";
        for (int i = 0; i < BOARD_SIZE; i++)
            std::cout << (char)('A' + i) << " ";
        std::cout << std::endl;

        for (int i = 0; i < BOARD_SIZE; i++) {
            std::cout << (i < 9 ? " " : "") << i + 1 << " ";
            for (int j = 0; j < BOARD_SIZE; j++) {
                char piece = '+';
                Role cell = board.getCell(Point(i, j));

                if (cell == Role::USER) piece = 'X';
                else if (cell == Role::BOT) piece = 'O';

                if (Point(i, j) == lastMove)
                    std::cout << "\033[33m" << piece << "\033[0m ";
                else if (piece == 'X')
                    std::cout << "\033[36m" << piece << "\033[0m ";
                else if (piece == 'O')
                    std::cout << "\033[31m" << piece << "\033[0m ";
                else
                    std::cout << piece << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

public:
    void run() {
        printBoard(Point(-1, -1));

        while (true) {
            std::cout << "Input your move: ";
            std::string input;
            std::cin >> input;

            if (input == "quit") break;

            if (input.length() < 2) {
                std::cout << "Wrong format!" << std::endl;
                continue;
            }

            int col = toupper(input[0]) - 'A';
            int row = atoi(input.substr(1).c_str()) - 1;
            Point playerMove(row, col);

            if (!board.makeMove(playerMove, Role::USER)) {
                std::cout << "Illegal move, please try again!" << std::endl;
                continue;
            }

            printBoard(playerMove);

            if (board.checkWinner(playerMove) == Role::USER) {
                std::cout << "VICTORY!" << std::endl;
                break;
            }
            if (board.isFull()) {
                std::cout << "DRAW!" << std::endl;
                break;
            }

            std::cout << "AI is thinking..." << std::endl;
            Point aiMove = ai.getBestMove(board);
            board.makeMove(aiMove, Role::BOT);
            printBoard(aiMove);

            if (board.checkWinner(aiMove) == Role::BOT) {
                std::cout << "DEFEAT!" << std::endl;
                break;
            }
            if (board.isFull()) {
                std::cout << "DRAW!" << std::endl;
                break;
            }
        }
        
        std::cout << "Thanks for playing!" << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }
};

int main() {
    ConsoleGame game;
    game.run();
    return 0;
}
