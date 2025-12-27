#include "headers/ai.h"
#include "raylib/raylib.h"
#include <string>
#include <thread>
#include <atomic>
#include <future>
#include <cmath>

// Layout Constants
const float CELL_SIZE = 40.0f;
const float MARGIN = 50.0f;
const float PANEL_WIDTH = 200.0f;
const float BOARD_PX = CELL_SIZE * (BOARD_SIZE - 1) + MARGIN * 2;
const int WINDOW_WIDTH = (int)(BOARD_PX + PANEL_WIDTH);
const int WINDOW_HEIGHT = (int)BOARD_PX;

// Color Palette (Morandi Style)
const Color BG_COLOR = { 245, 243, 238, 255 };        // Warm off-white
const Color BOARD_COLOR = { 232, 225, 214, 255 };     // Light beige
const Color GRID_COLOR = { 180, 170, 155, 255 };      // Muted brown
const Color BLACK_PIECE = { 45, 45, 45, 255 };        // Soft black
const Color WHITE_PIECE = { 250, 250, 248, 255 };     // Cream white
const Color SHADOW_COLOR = { 0, 0, 0, 40 };           // Subtle shadow
const Color HIGHLIGHT_COLOR = { 180, 100, 90, 255 };  // Terracotta accent
const Color HOVER_COLOR = { 45, 45, 45, 80 };         // Ghost piece
const Color TEXT_COLOR = { 80, 75, 70, 255 };         // Dark brown text
const Color BTN_COLOR = { 215, 205, 190, 255 };       // Button normal
const Color BTN_HOVER = { 195, 185, 170, 255 };       // Button hover
const Color STAR_COLOR = { 140, 130, 115, 255 };      // Star points

class RaylibGame {
private:
    Board board;
    AI ai;
    Point lastMove;
    bool gameOver = false;
    std::string message;
    
    // Async AI
    std::atomic<bool> aiThinking{false};
    std::future<Point> aiFuture;
    Point pendingAiMove;
    
    // Game over animation
    float gameOverAlpha = 0.0f;
    float gameOverTime = 0.0f;
    
    // AI thinking indicator animation
    float aiThinkingAlpha = 0.0f;
    float aiThinkingTime = 0.0f;

    // Custom font for crisp text
    Font font;

    // Star points for 15x15 board
    const int starPoints[5][2] = {{3,3}, {3,11}, {7,7}, {11,3}, {11,11}};

    // Helper: draw text with custom font
    void drawText(const char* text, float x, float y, float size, Color color) {
        DrawTextEx(font, text, {x, y}, size, 1.0f, color);
    }

    void drawBoard() {
        // Draw board background (rounded rect)
        float bgPadding = 20.0f;
        DrawRectangleRounded(
            {MARGIN - bgPadding, MARGIN - bgPadding, 
             BOARD_PX - 2*MARGIN + 2*bgPadding, BOARD_PX - 2*MARGIN + 2*bgPadding},
            0.02f, 8, BOARD_COLOR
        );

        // Draw grid lines
        for (int i = 0; i < BOARD_SIZE; i++) {
            float pos = MARGIN + i * CELL_SIZE;
            DrawLineEx(
                {MARGIN, pos},
                {BOARD_PX - MARGIN, pos},
                1.5f, GRID_COLOR
            );
            DrawLineEx(
                {pos, MARGIN},
                {pos, BOARD_PX - MARGIN},
                1.5f, GRID_COLOR
            );
        }

        // Draw star points
        for (auto& sp : starPoints) {
            float x = MARGIN + sp[1] * CELL_SIZE;
            float y = MARGIN + sp[0] * CELL_SIZE;
            DrawCircle((int)x, (int)y, 4.0f, STAR_COLOR);
        }

        // Draw hover preview (only when not AI's turn and game not over)
        if (!gameOver && !aiThinking) {
            Point hover = getHoveredCell();
            if (hover.getX() >= 0 && board.isCellEmpty(hover)) {
                float hx = MARGIN + hover.getY() * CELL_SIZE;
                float hy = MARGIN + hover.getX() * CELL_SIZE;
                DrawCircle((int)hx, (int)hy, CELL_SIZE / 2.0f - 4.0f, HOVER_COLOR);
            }
        }

        // Draw pieces
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                Role cell = board.getCell(Point(i, j));
                if (cell == Role::EMPTY) continue;

                float x = MARGIN + j * CELL_SIZE;
                float y = MARGIN + i * CELL_SIZE;
                float radius = CELL_SIZE / 2.0f - 4.0f;

                // Shadow
                DrawCircle((int)x + 2, (int)y + 2, radius, SHADOW_COLOR);

                // Piece
                Color pieceColor = (cell == Role::USER) ? BLACK_PIECE : WHITE_PIECE;
                DrawCircle((int)x, (int)y, radius, pieceColor);

                // White piece border
                if (cell == Role::BOT) {
                    DrawCircleLines((int)x, (int)y, radius, GRID_COLOR);
                }

                // Last move indicator
                if (Point(i, j) == lastMove) {
                    Color dotColor = (cell == Role::USER) ? WHITE_PIECE : BLACK_PIECE;
                    DrawCircle((int)x, (int)y, 4.0f, dotColor);
                }
            }
        }
    }

    void drawAiThinkingIndicator() {
        if (!aiThinking) {
            aiThinkingAlpha = std::max(0.0f, aiThinkingAlpha - GetFrameTime() * 3.0f);
            if (aiThinkingAlpha <= 0.0f) return;
        } else {
            aiThinkingAlpha = std::min(1.0f, aiThinkingAlpha + GetFrameTime() * 3.0f);
            aiThinkingTime += GetFrameTime();
        }
        
        // Card dimensions and position (top-right of board)
        float cardWidth = 200.0f;
        float cardHeight = 70.0f;
        float cardX = BOARD_PX - MARGIN - cardWidth - 20.0f;
        float cardY = MARGIN - 10.0f;
        
        // Card shadow
        Rectangle shadowRect = {cardX + 3, cardY + 3, cardWidth, cardHeight};
        Color shadowCol = {0, 0, 0, (unsigned char)(40 * aiThinkingAlpha)};
        DrawRectangleRounded(shadowRect, 0.2f, 12, shadowCol);
        
        // Card background with border
        Rectangle cardRect = {cardX, cardY, cardWidth, cardHeight};
        Color borderCol = {180, 170, 155, (unsigned char)(180 * aiThinkingAlpha)};
        DrawRectangleRounded(cardRect, 0.2f, 12, borderCol);
        
        Rectangle innerRect = {cardX + 2, cardY + 2, cardWidth - 4, cardHeight - 4};
        Color cardBg = {245, 243, 238, (unsigned char)(240 * aiThinkingAlpha)};
        DrawRectangleRounded(innerRect, 0.2f, 12, cardBg);
        
        // Animated loading spinner
        float spinnerX = cardX + 25.0f;
        float spinnerY = cardY + cardHeight / 2.0f;
        float spinnerRadius = 12.0f;
        float rotation = aiThinkingTime * 180.0f; // Rotate 180 degrees per second
        
        // Draw circular arc as spinner
        for (int i = 0; i < 3; i++) {
            float startAngle = rotation + i * 120.0f;
            float endAngle = startAngle + 80.0f;
            Color arcCol = {180, 100, 90, (unsigned char)(200 * aiThinkingAlpha)};
            
            // Draw arc using line segments
            for (float angle = startAngle; angle < endAngle; angle += 5.0f) {
                float rad1 = angle * DEG2RAD;
                float rad2 = (angle + 5.0f) * DEG2RAD;
                Vector2 p1 = {spinnerX + (float)cos(rad1) * spinnerRadius, spinnerY + (float)sin(rad1) * spinnerRadius};
                Vector2 p2 = {spinnerX + (float)cos(rad2) * spinnerRadius, spinnerY + (float)sin(rad2) * spinnerRadius};
                DrawLineEx(p1, p2, 3.0f, arcCol);
            }
        }
        
        // Text with breathing effect
        float breathe = 0.7f + 0.3f * std::sin(aiThinkingTime * 2.0f);
        Color textCol = {80, 75, 70, (unsigned char)(255 * aiThinkingAlpha * breathe)};
        drawText("AI Thinking", cardX + 62.0f, cardY + 25.0f, 20, textCol);
        
        // Animated dots
        int dotCount = ((int)(aiThinkingTime * 2.0f) % 3) + 1;
        std::string dots(dotCount, '.');
        drawText(dots.c_str(), cardX + 168.0f, cardY + 20.0f, 20, textCol);
    }
    
    void drawGameOverOverlay() {
        if (!gameOver) return;
        
        // Update animation
        gameOverTime += GetFrameTime();
        gameOverAlpha = std::min(1.0f, gameOverTime * 2.0f); // Fade in over 0.5s
        
        // Semi-transparent overlay
        Color overlayBg = {0, 0, 0, (unsigned char)(80 * gameOverAlpha)};
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, overlayBg);
        
        // Central message card
        float cardWidth = 350.0f;
        float cardHeight = 180.0f;
        float cardX = (BOARD_PX - cardWidth) / 2.0f;
        float cardY = (WINDOW_HEIGHT - cardHeight) / 2.0f;
        
        // Card shadow
        Rectangle shadowRect = {cardX + 4, cardY + 4, cardWidth, cardHeight};
        Color shadowCol = {0, 0, 0, (unsigned char)(60 * gameOverAlpha)};
        DrawRectangleRounded(shadowRect, 0.15f, 16, shadowCol);
        
        // Card background with border
        Rectangle cardRect = {cardX, cardY, cardWidth, cardHeight};
        Color borderCol = {180, 170, 155, (unsigned char)(200 * gameOverAlpha)};
        DrawRectangleRounded(cardRect, 0.15f, 16, borderCol);
        
        Rectangle innerRect = {cardX + 2, cardY + 2, cardWidth - 4, cardHeight - 4};
        Color cardBg = {245, 243, 238, (unsigned char)(250 * gameOverAlpha)};
        DrawRectangleRounded(innerRect, 0.15f, 16, cardBg);
        
        // Message text with different colors
        Color msgColor;
        if (message == "VICTORY!") {
            msgColor = {90, 150, 90, (unsigned char)(255 * gameOverAlpha)}; // Soft green
        } else if (message == "DEFEAT!") {
            msgColor = {180, 100, 90, (unsigned char)(255 * gameOverAlpha)}; // Terracotta
        } else {
            msgColor = {120, 120, 120, (unsigned char)(255 * gameOverAlpha)}; // Gray
        }
        
        // Center the text
        Vector2 textSize = MeasureTextEx(font, message.c_str(), 48, 1.0f);
        float textX = cardX + (cardWidth - textSize.x) / 2.0f;
        float textY = cardY + 50.0f;
        drawText(message.c_str(), textX, textY, 48, msgColor);
        
        // Subtitle with pulsing effect
        float pulse = 0.8f + 0.2f * std::sin(GetTime() * 2.0f);
        Color subtitleCol = {100, 95, 90, (unsigned char)(200 * gameOverAlpha * pulse)};
        const char* subtitle = "Click RESTART to play again";
        Vector2 subSize = MeasureTextEx(font, subtitle, 16, 1.0f);
        float subX = cardX + (cardWidth - subSize.x) / 2.0f;
        drawText(subtitle, subX, textY + 70, 16, subtitleCol);
    }
    
    void drawPanel() {
        float panelX = BOARD_PX + 15.0f;
        float panelY = 40.0f;

        // Title
        drawText("GOMOKU", panelX, panelY, 32, TEXT_COLOR);
        drawText("Five in a Row", panelX, panelY + 38, 15, GRID_COLOR);

        // Turn indicator
        float indicatorY = panelY + 100.0f;
        drawText("Current Turn", panelX, indicatorY, 15, GRID_COLOR);
        
        if (gameOver) {
            // Simplified indicator when game over (main message is in overlay)
            drawText("Game Over", panelX, indicatorY + 28, 18, GRID_COLOR);
        } else if (aiThinking) {
            // Show white piece + AI text
            DrawCircle((int)(panelX + 12), (int)(indicatorY + 40), 12.0f, WHITE_PIECE);
            DrawCircleLines((int)(panelX + 12), (int)(indicatorY + 40), 12.0f, GRID_COLOR);
            drawText("Bot", panelX + 32, indicatorY + 30, 18, TEXT_COLOR);
        } else {
            // Show black piece + Your Turn
            DrawCircle((int)(panelX + 12), (int)(indicatorY + 40), 12.0f, BLACK_PIECE);
            drawText("Your Turn", panelX + 32, indicatorY + 30, 18, TEXT_COLOR);
        }

        // Restart button
        Rectangle btnRect = {panelX, (float)WINDOW_HEIGHT - 90.0f, 150.0f, 45.0f};
        bool btnHover = CheckCollisionPointRec(GetMousePosition(), btnRect);
        
        DrawRectangleRounded(btnRect, 0.3f, 8, btnHover ? BTN_HOVER : BTN_COLOR);
        drawText("Restart", panelX + 42, WINDOW_HEIGHT - 77, 20, TEXT_COLOR);

        if (btnHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            resetGame();
        }

        // Credits
        drawText("by Tianjian Chen", panelX, WINDOW_HEIGHT - 35, 13, GRID_COLOR);
    }

    Point getHoveredCell() {
        float mx = (float)GetMouseX();
        float my = (float)GetMouseY();

        int col = (int)((mx - MARGIN + CELL_SIZE / 2) / CELL_SIZE);
        int row = (int)((my - MARGIN + CELL_SIZE / 2) / CELL_SIZE);

        if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE)
            return Point(row, col);
        return Point(-1, -1);
    }

    Point getClickedCell() {
        if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) return Point(-1, -1);
        return getHoveredCell();
    }

    void resetGame() {
        // Wait for AI thread if running
        if (aiThinking && aiFuture.valid()) {
            aiFuture.wait();
        }
        board = Board();
        lastMove = Point(-1, -1);
        gameOver = false;
        aiThinking = false;
        message.clear();
        gameOverAlpha = 0.0f;
        gameOverTime = 0.0f;
    }

    void startAiThinking() {
        aiThinking = true;
        aiThinkingTime = 0.0f;
        // Copy board for thread safety
        Board boardCopy = board;
        aiFuture = std::async(std::launch::async, [this, boardCopy]() mutable {
            AI aiCopy;
            return aiCopy.getBestMove(boardCopy);
        });
    }

    void checkAiResult() {
        if (!aiThinking || !aiFuture.valid()) return;
        
        // Check if AI is done (non-blocking)
        if (aiFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
            Point aiMove = aiFuture.get();
            board.makeMove(aiMove, Role::BOT);
            lastMove = aiMove;
            aiThinking = false;

            if (board.checkWinner(aiMove) == Role::BOT) {
                message = "DEFEAT!";
                gameOver = true;
                gameOverTime = 0.0f;
            } else if (board.isFull()) {
                message = "DRAW!";
                gameOver = true;
                gameOverTime = 0.0f;
            }
        }
    }

public:
    RaylibGame() : lastMove(-1, -1) { }

    void run() {
        // Enable high-DPI support and anti-aliasing
        SetConfigFlags(FLAG_WINDOW_HIGHDPI | FLAG_MSAA_4X_HINT);
        InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Gomoku");
        SetTargetFPS(60);

        // Load custom font (Windows Segoe UI) for crisp text rendering
        // Load at larger size for better quality when scaled
        font = LoadFontEx("fonts/HarmonyOS_Sans_SC_Medium.ttf", 48, nullptr, 0);
        SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

        while (!WindowShouldClose()) {
            // Check AI result (non-blocking)
            checkAiResult();

            // Handle input
            if (!gameOver && !aiThinking) {
                Point clicked = getClickedCell();
                if (clicked.getX() >= 0 && board.makeMove(clicked, Role::USER)) {
                    lastMove = clicked;

                    if (board.checkWinner(clicked) == Role::USER) {
                        message = "VICTORY!";
                        gameOver = true;
                        gameOverTime = 0.0f;
                    } else if (board.isFull()) {
                        message = "DRAW!";
                        gameOver = true;
                        gameOverTime = 0.0f;
                    } else {
                        // Start AI thinking in background
                        startAiThinking();
                    }
                }
            }

            // Draw
            BeginDrawing();
            ClearBackground(BG_COLOR);
            drawBoard();
            drawAiThinkingIndicator();  // Draw AI indicator over board
            drawPanel();
            drawGameOverOverlay();  // Draw overlay last for proper layering
            EndDrawing();
        }

        // Cleanup: wait for AI thread
        if (aiThinking && aiFuture.valid()) {
            aiFuture.wait();
        }
        UnloadFont(font);
        CloseWindow();
    }
};

int main() {
    RaylibGame game;
    game.run();
    return 0;
}
