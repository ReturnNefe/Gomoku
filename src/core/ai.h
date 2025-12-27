#ifndef GOMOKU_AI_H
#define GOMOKU_AI_H

#include "board.h"

class AI {
private:
    int minimax(Board& board, Role role, int depth, Point lastMove, int alpha, int beta) {
        auto winner = board.checkWinner(lastMove);

        if (winner != Role::EMPTY)
            return winner == Role::BOT ? PredefinedScore::WIN : PredefinedScore::LOSE;
        if (depth == 0 || board.isFull())
            return board.evaluate(Role::BOT);

        auto candidates = board.getSortedCandidates(role);

        if (role == Role::BOT) {
            for (auto& p : candidates) {
                if (board.makeMove(p, role)) {
                    int score = minimax(board, Role::USER, depth - 1, p, alpha, beta);
                    board.undoMove(p);
                    alpha = std::max(alpha, score);
                    if (alpha >= beta) break;
                }
            }
            return alpha;
        } else {
            for (auto& p : candidates) {
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
    Point getBestMove(Board& board) {
        auto candidates = board.getSortedCandidates(Role::BOT);
        Point bestMove;
        int bestScore = PredefinedScore::LOSE;

        for (auto& p : candidates) {
            if (board.makeMove(p, Role::BOT)) {
                int score = minimax(board, Role::USER, SEARCH_DEPTH - 1, p,
                                    PredefinedScore::LOSE, PredefinedScore::WIN);
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
