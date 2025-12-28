#ifndef GOMOKU_AI_H
#define GOMOKU_AI_H

#include "board.h"
#include <limits>

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

                    alpha = std::max(alpha, score);
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

                    beta = std::min(beta, score);
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
        int bestScore = std::numeric_limits<int>::min();
        
        for (auto &p : candidates) {
            if (board.makeMove(p, Role::BOT)) {
                int score = minimax(
                    board,
                    Role::USER,
                    SEARCH_DEPTH - 1,
                    p,
                    std::numeric_limits<int>::min(),
                    std::numeric_limits<int>::max()
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

#endif
