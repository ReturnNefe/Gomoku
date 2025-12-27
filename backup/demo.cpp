#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <string>
#include <cctype>

using namespace std;

const int BOARD_SIZE = 15;
const int EMPTY = 0;
const int PLAYER = 1;  // 玩家
const int BOT = 2;  // AI
const int SEARCH_DEPTH = 3;
const int SEARCH_RANGE = 2;  // 候选点搜索范围

// 棋盘类
class Board {
private:
    int board[BOARD_SIZE][BOARD_SIZE];
    
    int checkRow(int i, int j, int dx, int dy) const
    {
        int count = 1;
        int role = board[i][j];
        int x = i + dx, y = j + dy;
        while (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE && board[x][y] == role) {
            count++;
            x += dx;
            y += dy;
        }
        return count;
    }

public:
    Board() {
        // 初始化棋盘
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
    }
    
    // 落子
    bool makeMove(int x, int y, int player) {
        if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
            return false;
        }
        if (board[x][y] != EMPTY) {
            return false;
        }
        board[x][y] = player;
        return true;
    }
    
    // 撤销落子
    void undoMove(int x, int y) {
        board[x][y] = EMPTY;
    }
    
    // 获取棋子
    int getCell(int x, int y) const {
        if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE) {
            return -1;
        }
        return board[x][y];
    }
    
    // 检查是否为空
    bool isEmpty(int x, int y) const {
        return getCell(x, y) == EMPTY;
    }
    
    // 判断胜负
    int checkWin() const {
        // 检查横、竖、斜四个方向
        int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
        
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == EMPTY) continue;
                
                int role = board[i][j];
                
                // 检查四个方向
                for (int d = 0; d < 4; d++) {
                    int count = 1;
                    int dx = directions[d][0];
                    int dy = directions[d][1];
                    
                    // 双方向计数
                    count = checkRow(i, j, dx, dy) + checkRow(i, j, -dx, -dy) - 1;
                    
                    if (count >= 5) {
                        return role;  // 返回获胜方
                    }
                }
            }
        }
        return 0;  // 没有获胜方
    }
    
    // 判断棋盘是否已满
    bool isFull() const {
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == EMPTY) {
                    return false;
                }
            }
        }
        return true;
    }
    
    // 打印棋盘
    void print() const {
        cout << "   ";
        for (int i = 0; i < BOARD_SIZE; i++) {
            cout << (char)('A' + i) << " ";
        }
        cout << endl;
        
        for (int i = 0; i < BOARD_SIZE; i++) {
            cout << (i < 9 ? " " : "") << i + 1 << " ";
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == EMPTY) {
                    cout << "+ ";
                } else if (board[i][j] == PLAYER) {
                    cout << "X ";
                } else {
                    cout << "O ";
                }
            }
            cout << endl;
        }
        cout << endl;
    }
    
    // 生成候选点（在已有棋子周围SEARCH_RANGE范围内）
    vector<pair<int, int> > generateCandidates() const {
        vector<pair<int, int> > candidates;
        bool hasStone[BOARD_SIZE][BOARD_SIZE] = {false};
        
        // 如果棋盘为空，返回中心点
        bool isEmpty = true;
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] != EMPTY) {
                    isEmpty = false;
                    break;
                }
            }
            if (!isEmpty) break;
        }
        
        if (isEmpty) {
            candidates.push_back({BOARD_SIZE / 2, BOARD_SIZE / 2});
            return candidates;
        }
        
        // 找出所有已有棋子周围SEARCH_RANGE范围内的空位
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] != EMPTY) {
                    // 在周围SEARCH_RANGE格内寻找空位
                    for (int di = -SEARCH_RANGE; di <= SEARCH_RANGE; di++) {
                        for (int dj = -SEARCH_RANGE; dj <= SEARCH_RANGE; dj++) {
                            int ni = i + di;
                            int nj = j + dj;
                            if (ni >= 0 && ni < BOARD_SIZE && nj >= 0 && nj < BOARD_SIZE
                                && board[ni][nj] == EMPTY && !hasStone[ni][nj]) {
                                hasStone[ni][nj] = true;
                                candidates.push_back({ni, nj});
                            }
                        }
                    }
                }
            }
        }
        
        return candidates;
    }
    
    // 简单评估函数：统计连子数量打分
    int evaluate(int player) const {
        int score = 0;
        int opponent = (player == PLAYER) ? BOT : PLAYER;
        
        // 四个方向
        int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
        
        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                if (board[i][j] == EMPTY) continue;
                
                int currentPlayer = board[i][j];
                
                // 检查四个方向的连子
                for (int d = 0; d < 4; d++) {
                    int count = 1;
                    int dx = directions[d][0];
                    int dy = directions[d][1];
                    
                    // 正方向计数
                    int x = i + dx, y = j + dy;
                    while (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE 
                           && board[x][y] == currentPlayer) {
                        count++;
                        x += dx;
                        y += dy;
                    }
                    
                    // 根据连子数量打分
                    int lineScore = 0;
                    if (count >= 5) {
                        lineScore = 100000;  // 五连
                    } else if (count == 4) {
                        lineScore = 10000;   // 四连
                    } else if (count == 3) {
                        lineScore = 1000;    // 三连
                    } else if (count == 2) {
                        lineScore = 100;     // 二连
                    }
                    
                    // 如果是AI的棋子加分，对手的棋子减分
                    if (currentPlayer == player) {
                        score += lineScore;
                    } else {
                        score -= lineScore;
                    }
                }
            }
        }
        
        return score;
    }
};

// AI类 - Minimax算法
class AI {
private:
    int aiPlayer;
    int humanPlayer;
    
    // Minimax核心算法
    int minimax(Board& board, int depth, bool isMaximizing) {
        // 终止条件
        int winner = board.checkWin();
        if (winner == aiPlayer) {
            return 1000000;  // AI获胜
        }
        if (winner == humanPlayer) {
            return -1000000;  // 玩家获胜
        }
        if (depth == 0 || board.isFull()) {
            return board.evaluate(aiPlayer);  // 达到搜索深度或棋盘已满
        }
        
        // 生成候选点
        vector<pair<int, int> > candidates = board.generateCandidates();
        
        if (isMaximizing) {
            // AI回合，寻找最大值
            int maxScore = numeric_limits<int>::min();
            
            for (auto& move : candidates) {
                board.makeMove(move.first, move.second, aiPlayer);
                int score = minimax(board, depth - 1, false);
                board.undoMove(move.first, move.second);
                maxScore = (maxScore > score) ? maxScore : score;
            }
            
            return maxScore;
        } else {
            // 玩家回合，寻找最小值
            int minScore = numeric_limits<int>::max();
            
            for (auto& move : candidates) {
                board.makeMove(move.first, move.second, humanPlayer);
                int score = minimax(board, depth - 1, true);
                board.undoMove(move.first, move.second);
                minScore = (minScore < score) ? minScore : score;
            }
            
            return minScore;
        }
    }
    
public:
    AI(int ai, int human) : aiPlayer(ai), humanPlayer(human) {}
    
    // 获取AI的最佳落子位置
    pair<int, int> getBestMove(Board& board) {
        vector<pair<int, int> > candidates = board.generateCandidates();
        
        int bestScore = numeric_limits<int>::min();
        pair<int, int> bestMove = {-1, -1};
        
        cout << "AI正在思考...（搜索深度: " << SEARCH_DEPTH << "层）" << endl;
        
        // 遍历所有候选点
        for (auto& move : candidates) {
            board.makeMove(move.first, move.second, aiPlayer);
            int score = minimax(board, SEARCH_DEPTH - 1, false);
            board.undoMove(move.first, move.second);
            
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }
        
        cout << "AI选择: (" << (char)('A' + bestMove.second) << ", " 
             << bestMove.first + 1 << "), 评分: " << bestScore << endl;
        
        return bestMove;
    }
};

// 游戏主控制类
class Game {
private:
    Board board;
    AI ai;
    
public:
    Game() : ai(BOT, PLAYER) {}
    
    void run() {
        cout << "========== 五子棋 AI 对战 =========" << endl;
        cout << "你是 X (黑棋)，AI 是 O (白棋)" << endl;
        cout << "输入格式: A1, B2, C3 等 (列+行)" << endl;
        cout << "输入 'quit' 退出游戏" << endl;
        cout << "===================================" << endl << endl;
        
        board.print();
        
        while (true) {
            // 玩家回合
            string input;
            cout << "你的回合 (X): ";
            cin >> input;
            
            if (input == "quit") {
                cout << "游戏结束！" << endl;
                break;
            }
            
            // 解析输入
            if (input.length() < 2) {
                cout << "输入格式错误，请重新输入！" << endl;
                continue;
            }
            
            int col = toupper(input[0]) - 'A';
            int row = atoi(input.substr(1).c_str()) - 1;
            
            if (!board.makeMove(row, col, PLAYER)) {
                cout << "非法落子，请重新输入！" << endl;
                continue;
            }
            
            board.print();
            
            // 检查玩家是否获胜
            if (board.checkWin() == PLAYER) {
                cout << "恭喜！你赢了！" << endl;
                break;
            }
            
            if (board.isFull()) {
                cout << "平局！" << endl;
                break;
            }
            
            // AI回合
            cout << "\nAI的回合 (O): " << endl;
            auto aiMove = ai.getBestMove(board);
            board.makeMove(aiMove.first, aiMove.second, BOT);
            
            board.print();
            
            // 检查AI是否获胜
            if (board.checkWin() == BOT) {
                cout << "AI获胜！" << endl;
                break;
            }
            
            if (board.isFull()) {
                cout << "平局！" << endl;
                break;
            }
        }
    }
};

int main() {
    Game game;
    game.run();
    return 0;
}