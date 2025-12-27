#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const int BOARD_SIZE = 15;
const int SEARCH_DEPTH = 3;
const int SEARCH_RANGE = 2;
// K-Value = Defense / Attack
const double DEFENSE_WEIGHT = 1.2; 

namespace PredefinedScore {
    const int WIN = 1e7;
    const int LOSE = -1e7;

    const int FIVE = 1e6;
    const int LIVE_4 = 1e5;
    const int RUSH_4 = 1e4;
    const int LIVE_3 = 8e3;
    const int SLEEP_3 = 1e3;
    const int LIVE_2 = 5e2;
    const int SLEEP_2 = 5e1;
}

enum class Role { EMPTY, USER, BOT };

class Point {
private:
    int x, y;
public:
    Point(int x = 0, int y = 0) : x(x), y(y) { }
    int getX() const { return x; }
    int getY() const { return y; }
    void setX(int x) { this->x = x; }
    void setY(int y) { this->y = y; }

    bool operator == (const Point &p) const {
        return x == p.x && y == p.y;
    }
};

using Direction = Point;

// Forward declaration
class Board;

// Zero-copy line view with iterator support
class LineView {
private:
    const Board& board;
    Point start;
    Direction dir;
    int length;

public:
    class Iterator {
    private:
        const Board& board;
        Point current;
        Direction dir;
        int remaining;

    public:
        Iterator(const Board& b, Point p, Direction d, int r)
            : board(b), current(p), dir(d), remaining(r) { }

        Role operator*() const;

        Iterator& operator++() {
            current = Point(current.getX() + dir.getX(), current.getY() + dir.getY());
            --remaining;
            return *this;
        }

        bool operator!=(const Iterator&) const {
            return remaining > 0;
        }
    };

    LineView(const Board& b, Point start, Direction dir, int length = INT_MAX)
        : board(b), start(start), dir(dir), length(length) { }

    Iterator begin() const { return Iterator(board, start, dir, length); }
    Iterator end() const { return Iterator(board, start, dir, 0); }

    // Random access support (compute on-the-fly, no memory allocation)
    Role at(int index) const;
    int getActualLength() const;
};

// standard gomoku board
class Board {
private:
    vector<vector<Role>> board;

    bool isRangeValid(int x, int y) const {
        return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
    }

    bool isRangeValid(const Point &p) const {
        return isRangeValid(p.getX(), p.getY());
    }

    int analyzeShape(int count, bool leftOpen, bool rightOpen, bool hasGap) const {
        if (count >= 5)
            return PredefinedScore::FIVE;

        if (count == 4) {
            if (leftOpen && rightOpen)
                return hasGap ? PredefinedScore::RUSH_4 : PredefinedScore::LIVE_4;
            if (leftOpen || rightOpen)
                return PredefinedScore::RUSH_4;
            else
                return 0;
        }

        if (count == 3) {
            if (leftOpen && rightOpen)
                return hasGap ? (PredefinedScore::LIVE_3 * 9 / 10) : PredefinedScore::LIVE_3;
            if (leftOpen || rightOpen)
                return PredefinedScore::SLEEP_3;
            else
                return 0;
        }

        if (count == 2) {
            if (leftOpen && rightOpen)
                return PredefinedScore::LIVE_2;
            if (leftOpen || rightOpen)
                return PredefinedScore::SLEEP_2;
            else
                return 0;
        }

        return 0;
    }

    // TODO: test required
    int analyzeLine(const LineView &line, const Role &role) const {
        int score = 0;
        int index = 0;
        int length = line.getActualLength();

        while (index < length)
        {
            if (line.at(index) == role) {
                bool leftOpen = (index > 0 && line.at(index - 1) == Role::EMPTY);
                bool rightOpen = false;
                int count = 0;

                while (index < length && line.at(index) == role) {
                    ++index;
                    ++count;
                }

                bool hasGap = false;

                if (index < length && line.at(index) == Role::EMPTY) {
                    if (index + 1 < length && line.at(index + 1) == role) {
                        hasGap = true;
                        ++index;
                        while (index < length && line.at(index) == role) {
                            ++index;
                            ++count;
                        }
                    }
                }

                rightOpen = (index < length && line.at(index) == Role::EMPTY);
                score += analyzeShape(count, leftOpen, rightOpen, hasGap);
            }
            else ++index;
        }

        return score;
    }

public:
    Board() {
        board = vector<vector<Role>>(BOARD_SIZE, vector<Role>(BOARD_SIZE, Role::EMPTY));
    }

    bool makeMove(const Point &p, Role role)
    {
        if (!isCellEmpty(p))
            return false;

        board[p.getX()][p.getY()] = role;
        return true;
    }

    void undoMove(const Point &p) {
        board[p.getX()][p.getY()] = Role::EMPTY;
    }

    Role getCell(const Point &p) const {
        return isRangeValid(p)
            ? board[p.getX()][p.getY()]
            : Role::EMPTY;
    }

    bool isCellEmpty(const Point &p) const {
        return getCell(p) == Role::EMPTY;
    }

    Role checkWinner(const Point &p) const {
        int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
        Role cell = getCell(p);

        for (auto &direction : directions)
        {
            int count = 1;

            // check positive direction
            for (int i = 1; i < 5; ++i)
            {
                Point newP(p.getX() + direction[0] * i, p.getY() + direction[1] * i);
                if (isRangeValid(newP) && getCell(newP) == cell)
                    count++;
                else
                    break;
                if (count >= 5)
                    return cell;
            }

            // check negative direction
            for (int i = 1; i < 5; ++i)
            {
                Point newP(p.getX() - direction[0] * i, p.getY() - direction[1] * i);
                if (isRangeValid(newP) && getCell(newP) == cell)
                    count++;
                else
                    break;
                if (count >= 5)
                    return cell;
            }
        }

        return Role::EMPTY;
    }

    bool isFull() const {
        for (int i = 0; i < BOARD_SIZE; ++i)
            for (int j = 0; j < BOARD_SIZE; ++j)
                if (board[i][j] == Role::EMPTY)
                    return false;
        return true;
    }

    // fast evaluate point
    int evaluatePoint(const Point &p, const Role &role) const {
        int score = 0;
        int directions[4][2] = {{1, 0}, {0, 1}, {1, 1}, {1, -1}};
        Role opponent = (role == Role::USER) ? Role::BOT : Role::USER;

        for (auto &dir : directions) {
            // find the start point of the line (extend 4格 in the negative direction)
            int startX = p.getX() - dir[0] * 4;
            int startY = p.getY() - dir[1] * 4;
            
            // ensure the start point is within the valid range
            while (!isRangeValid(startX, startY)) {
                startX += dir[0];
                startY += dir[1];
            }
            
            LineView line(*this, Point(startX, startY), Direction(dir[0], dir[1]), 9);
            score += analyzeLine(line, role);
            score -= analyzeLine(line, opponent);
        }
        
        return score;
    }

    vector<Point> getCandidates() const {
        vector<Point> candidates;
        vector<Point> occupiedPoints;
        vector<vector<bool>> visited(BOARD_SIZE, vector<bool>(BOARD_SIZE, false));

        for (int i = 0; i < BOARD_SIZE; ++i)
            for (int j = 0; j < BOARD_SIZE; ++j)
                if (board[i][j] != Role::EMPTY)
                    occupiedPoints.push_back(Point(i, j));
        
        // return center point if board is empty
        if (occupiedPoints.empty()) {
            candidates.push_back(Point(BOARD_SIZE / 2, BOARD_SIZE / 2));
            return candidates;
        }

        for (auto &p : occupiedPoints) {
            for (int i = -SEARCH_RANGE; i <= SEARCH_RANGE; ++i) {
                for (int j = -SEARCH_RANGE; j <= SEARCH_RANGE; ++j) {
                    Point newP(p.getX() + i, p.getY() + j);
                    if (isRangeValid(newP) && !visited[newP.getX()][newP.getY()]) {
                        visited[newP.getX()][newP.getY()] = true;
                        if (isCellEmpty(newP))
                            candidates.push_back(newP);
                    }
                }
            }
        }

        return candidates;
    }

    // optimize: sort candidates by heuristic score
    vector<Point> getSortedCandidates(Role role) {
        auto candidates = getCandidates();
        
        // Calculate the score for each candidate point
        vector<pair<int, Point>> scoredMoves;
        for (auto &p : candidates) {
            // Evaluate the point
            board[p.getX()][p.getY()] = role;
            int score = evaluatePoint(p, role);
            board[p.getX()][p.getY()] = Role::EMPTY;
            scoredMoves.push_back({score, p});
        }
        
        // Sort the moves by score in descending order
        sort(
            scoredMoves.begin(),
            scoredMoves.end(),
            [](const auto &a, const auto &b) { return a.first > b.first; }
        );
        
        vector<Point> sortedMoves;
        for (auto &move : scoredMoves)
            sortedMoves.push_back(move.second);
        return sortedMoves;
    }

    int evaluate(const Role &role) const {
        int myScore = 0;
        int opponentScore = 0;
        Role opponent = (role == Role::USER) ? Role::BOT : Role::USER;

        // horizontal
        for (int i = 0; i < BOARD_SIZE; ++i) {
            LineView line(*this, Point(i, 0), Direction(0, 1));
            myScore += analyzeLine(line, role);
            opponentScore += analyzeLine(line, opponent);
        }

        // vertical
        for (int j = 0; j < BOARD_SIZE; ++j) {
            LineView line(*this, Point(0, j), Direction(1, 0));
            myScore += analyzeLine(line, role);
            opponentScore += analyzeLine(line, opponent);
        }

        // diagonal (↘)
        for (int i = 0; i < BOARD_SIZE; ++i) {
            LineView line(*this, Point(i, 0), Direction(1, 1));
            myScore += analyzeLine(line, role);
            opponentScore += analyzeLine(line, opponent);
        }
        for (int j = 1; j < BOARD_SIZE; ++j) {
            LineView line(*this, Point(0, j), Direction(1, 1));
            myScore += analyzeLine(line, role);
            opponentScore += analyzeLine(line, opponent);
        }

        // diagonal (↙)
        for (int i = 0; i < BOARD_SIZE; ++i) {
            LineView line(*this, Point(i, BOARD_SIZE - 1), Direction(1, -1));
            myScore += analyzeLine(line, role);
            opponentScore += analyzeLine(line, opponent);
        }
        for (int j = 0; j < BOARD_SIZE - 1; ++j) {
            LineView line(*this, Point(0, j), Direction(1, -1));
            myScore += analyzeLine(line, role);
            opponentScore += analyzeLine(line, opponent);
        }

        // Score = Σ(My Patterns) - k × Σ(Opponent Patterns)
        return myScore - static_cast<int>(DEFENSE_WEIGHT * opponentScore);
    }

    // NOTE: for debug only
    void print(Point lastMove) const {
        cout << "   ";
        for (int i = 0; i < BOARD_SIZE; i++) {
            cout << (char)('A' + i) << " ";
        }
        cout << endl;
        
        for (int i = 0; i < BOARD_SIZE; i++) {
            cout << (i < 9 ? " " : "") << i + 1 << " ";
            for (int j = 0; j < BOARD_SIZE; j++) {
                char piece = '+';

                if (board[i][j] == Role::USER) piece = 'X';
                else if (board[i][j] == Role::BOT) piece = 'O';

                if (Point(i, j) == lastMove) {
                    cout << "\033[33m" << piece << "\033[0m ";
                }
                else {
                    if (piece == 'X')
                        cout << "\033[36m" << piece << "\033[0m ";
                    else if (piece == 'O')
                        cout << "\033[31m" << piece << "\033[0m ";
                    else
                        cout << piece << " ";
                }
            }
            cout << endl;
        }
        cout << endl;
    }
};

// LineView method implementations (after Board is complete)
Role LineView::Iterator::operator*() const {
    return board.getCell(current);
}

Role LineView::at(int index) const {
    Point p(start.getX() + dir.getX() * index, start.getY() + dir.getY() * index);
    return board.getCell(p);
}

int LineView::getActualLength() const {
    int count = 0;
    Point p = start;
    while (count < length && board.getCell(p) != Role::EMPTY || 
           (p.getX() >= 0 && p.getX() < BOARD_SIZE && p.getY() >= 0 && p.getY() < BOARD_SIZE)) {
        if (!(p.getX() >= 0 && p.getX() < BOARD_SIZE && p.getY() >= 0 && p.getY() < BOARD_SIZE))
            break;
        ++count;
        if (count >= length) break;
        p = Point(p.getX() + dir.getX(), p.getY() + dir.getY());
    }
    return count;
}

class AI {
private:
    // Alpha-Beta Pruning
    // AI:      MAX
    // USER:    MIN
    // alpha:   the lowest score AI can promise
    // beta:    the highest score USER can promise
    int minimax(Board &board, Role role, int depth, Point lastMove, int alpha, int beta) {
        auto winner = board.checkWinner(lastMove);
        
        if (winner != Role::EMPTY)
            return winner == Role::BOT ? PredefinedScore::WIN : PredefinedScore::LOSE;
        if (depth == 0 || board.isFull())
            return board.evaluate(Role::BOT);
        
        auto candidates = board.getSortedCandidates(role);

        if (role == Role::BOT) {
            // BOT: try to get the MAX score
            for (auto &p : candidates) {
                if (board.makeMove(p, role)) {
                    int score = minimax(board, Role::USER, depth - 1, p, alpha, beta);
                    board.undoMove(p);

                    alpha = max(alpha, score);
                    if (alpha >= beta) break;
                }
            }
            return alpha;
        }
        else {
            // USER: try to get the MIN score
            for (auto &p : candidates) {
                if (board.makeMove(p, role)) {
                    int score = minimax(board, Role::BOT, depth - 1, p, alpha, beta);
                    board.undoMove(p);

                    beta = min(beta, score);
                    if (alpha >= beta) break;
                }
            }
            return beta;
        }
    }

public:
    Point getBestMove(Board &board) {
        auto candidates = board.getSortedCandidates(Role::BOT);
        Point bestMove;
        int bestScore = PredefinedScore::LOSE;
        
        for (auto &p : candidates) {
            if (board.makeMove(p, Role::BOT)) {
                int score = minimax(
                    board,
                    Role::USER,
                    SEARCH_DEPTH - 1,
                    p,
                    PredefinedScore::LOSE,
                    PredefinedScore::WIN
                );
                
                board.undoMove(p);

                if (score > bestScore) {
                    bestScore = score;
                    bestMove = p;
                }
            }
        }
        
        return bestMove;
    }
};

class ConsoleGame {
private:
    Board board;
    AI ai;

    void printBoard(Point lastMove) const {
        // clear screen
        cout << "\033[2J\033[H";
        cout.flush();

        board.print(lastMove);
    }

public:
    ConsoleGame() { }

    void run() {
        printBoard(Point(-1, -1));

        while (true) {
            cout << "Input your move: ";
            string input;
            cin >> input;

            if (input == "quit")
                break;
            
            if (input.length() < 2) {
                cout << "Wrong format!" << endl;
                continue;
            }
            
            int col = toupper(input[0]) - 'A';
            int row = atoi(input.substr(1).c_str()) - 1;
            Point playerMove(row, col);

            if (!board.makeMove(playerMove, Role::USER)) {
                cout << "Illegal move, please try again!" << endl;
                continue;
            }

            printBoard(playerMove);

            if (board.checkWinner(playerMove) == Role::USER) {
                cout << "VICTORY!" << endl;
                break;
            }
            
            if (board.isFull()) {
                cout << "DRAW!" << endl;
                break;
            }

            cout << "AI is thinking..." << endl;
            
            Point aiMove = ai.getBestMove(board);
            board.makeMove(aiMove, Role::BOT);

            printBoard(aiMove);
            
            if (board.checkWinner(aiMove) == Role::BOT) {
                cout << "DEFEAT!" << endl;
                break;
            }

            if (board.isFull()) {
                cout << "DRAW!" << endl;
                break;
            }
        }
    }
};

int main() {
    ConsoleGame game;
    game.run();
    return 0;
}