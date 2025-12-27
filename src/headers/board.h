#ifndef GOMOKU_BOARD_H
#define GOMOKU_BOARD_H

#include "types.h"
#include <vector>
#include <algorithm>
#include <climits>

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
    std::vector<std::vector<Role>> board;

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
        board = std::vector<std::vector<Role>>(BOARD_SIZE, std::vector<Role>(BOARD_SIZE, Role::EMPTY));
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
            // find the start point of the line (extend 4 blocks in the negative direction)
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

    std::vector<Point> getCandidates() const {
        std::vector<Point> candidates;
        std::vector<Point> occupiedPoints;
        std::vector<std::vector<bool>> visited(BOARD_SIZE, std::vector<bool>(BOARD_SIZE, false));

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
    std::vector<Point> getSortedCandidates(Role role) {
        auto candidates = getCandidates();
        
        // Calculate the score for each candidate point
        std::vector<std::pair<int, Point>> scoredMoves;
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
        
        std::vector<Point> sortedMoves;
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
};

// LineView implementations
inline Role LineView::Iterator::operator*() const {
    return board.getCell(current);
}

inline Role LineView::at(int index) const {
    Point p(start.getX() + dir.getX() * index, start.getY() + dir.getY() * index);
    return board.getCell(p);
}

inline int LineView::getActualLength() const {
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

#endif
