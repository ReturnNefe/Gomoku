#include "core/ai.h"
#include <raylib/raylib.h>
#include <string>

const int CELL_SIZE = 40;
const int MARGIN = 30;
const int WINDOW_SIZE = CELL_SIZE * (BOARD_SIZE - 1) + MARGIN * 2;

class RaylibGame {
private:
    Board board;
    AI ai;
    Point lastMove{-1, -1};
    bool gameOver = false;
    std::string message;

    void drawBoard() {
        // Draw grid lines
        for (int i = 0; i < BOARD_SIZE; i++) {
            int pos = MARGIN + i * CELL_SIZE;
            DrawLine(MARGIN, pos, WINDOW_SIZE - MARGIN, pos, DARKGRAY);
            DrawLine(pos, MARGIN, pos, WINDOW_SIZE - MARGIN, DARKGRAY);
        }

        // Draw pieces
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                Role cell = board.getCell(Point(i, j));
                if (cell == Role::EMPTY) continue;

                int x = MARGIN + j * CELL_SIZE;
                int y = MARGIN + i * CELL_SIZE;
                Color color = (cell == Role::USER) ? BLUE : RED;

                DrawCircle(x, y, CELL_SIZE / 2 - 2, color);

                if (Point(i, j) == lastMove) {
                    DrawCircleLines(x, y, CELL_SIZE / 2 - 2, YELLOW);
                }
            }
        }

        // Draw message
        if (!message.empty()) {
            DrawText(message.c_str(), 10, WINDOW_SIZE - 25, 20, DARKGRAY);
        }
    }

    Point getClickedCell() {
        if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return Point(-1, -1);

        int mx = GetMouseX();
        int my = GetMouseY();

        int col = (mx - MARGIN + CELL_SIZE / 2) / CELL_SIZE;
        int row = (my - MARGIN + CELL_SIZE / 2) / CELL_SIZE;

        if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE)
            return Point(row, col);
        return Point(-1, -1);
    }

public:
    void run() {
        InitWindow(WINDOW_SIZE, WINDOW_SIZE, "Gomoku - Raylib");
        SetTargetFPS(60);

        while (!WindowShouldClose()) {
            // Handle input
            if (!gameOver) {
                Point clicked = getClickedCell();
                if (clicked.getX() >= 0 && board.makeMove(clicked, Role::USER)) {
                    lastMove = clicked;

                    if (board.checkWinner(clicked) == Role::USER) {
                        message = "VICTORY!";
                        gameOver = true;
                    } else if (board.isFull()) {
                        message = "DRAW!";
                        gameOver = true;
                    } else {
                        // AI move
                        Point aiMove = ai.getBestMove(board);
                        board.makeMove(aiMove, Role::BOT);
                        lastMove = aiMove;

                        if (board.checkWinner(aiMove) == Role::BOT) {
                            message = "DEFEAT!";
                            gameOver = true;
                        } else if (board.isFull()) {
                            message = "DRAW!";
                            gameOver = true;
                        }
                    }
                }
            }

            // Draw
            BeginDrawing();
            ClearBackground(RAYWHITE);
            drawBoard();
            EndDrawing();
        }

        CloseWindow();
    }
};

int main() {
    RaylibGame game;
    game.run();
    return 0;
}
