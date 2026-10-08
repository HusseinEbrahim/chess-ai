#include "search.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <vector>
#include "evaluate.h"

namespace {

using Clock = std::chrono::steady_clock;
constexpr int INF = 1000000;

bool isCapture(const Board& board, const Move& move) {
    if (board.pieceAt(move.to) != EMPTY) return true;
    // En passant: a pawn moving diagonally onto an empty square
    return std::abs(board.pieceAt(move.from)) == PAWN && (move.from % 8) != (move.to % 8);
}

// Higher = search first. Captures of valuable pieces by cheap pieces come first.
int moveScore(const Board& board, const Move& move) {
    int score = 0;
    if (isCapture(board, move)) {
        int victim = std::abs(board.pieceAt(move.to));
        int attacker = std::abs(board.pieceAt(move.from));
        score += 10000 + 10 * PIECE_VALUES[victim == EMPTY ? PAWN : victim] - PIECE_VALUES[attacker];
    }
    if (move.promotion != EMPTY) score += 5000 + PIECE_VALUES[move.promotion];
    return score;
}

void orderMoves(const Board& board, std::vector<Move>& moves) {
    std::stable_sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        return moveScore(board, a) > moveScore(board, b);
    });
}

class Searcher {
public:
    explicit Searcher(Clock::time_point deadline) : deadline_(deadline) {}

    uint64_t nodes = 0;
    bool stopped = false;

    int negamax(const Board& board, int depth, int alpha, int beta, int ply) {
        if (timeUp()) return 0;

        std::vector<Move> moves = board.legalMoves();
        if (moves.empty()) {
            // Checkmate (prefer faster mates) or stalemate
            return board.inCheck() ? -MATE_SCORE + ply : 0;
        }
        if (depth == 0) return quiescence(board, alpha, beta);

        ++nodes;
        orderMoves(board, moves);
        for (const Move& move : moves) {
            int score = -negamax(board.afterMove(move), depth - 1, -beta, -alpha, ply + 1);
            if (stopped) return 0;
            if (score >= beta) return beta;  // opponent won't allow this line: prune
            alpha = std::max(alpha, score);
        }
        return alpha;
    }

    // Keep searching captures until the position is quiet
    int quiescence(const Board& board, int alpha, int beta) {
        if (timeUp()) return 0;
        ++nodes;

        int standPat = evaluate(board);
        if (standPat >= beta) return beta;
        alpha = std::max(alpha, standPat);

        std::vector<Move> captures;
        for (const Move& move : board.legalMoves()) {
            if (isCapture(board, move) || move.promotion != EMPTY) captures.push_back(move);
        }
        orderMoves(board, captures);

        for (const Move& move : captures) {
            int score = -quiescence(board.afterMove(move), -beta, -alpha);
            if (stopped) return 0;
            if (score >= beta) return beta;
            alpha = std::max(alpha, score);
        }
        return alpha;
    }

private:
    Clock::time_point deadline_;

    bool timeUp() {
        if (stopped) return true;
        if ((nodes & 2047) == 0 && Clock::now() >= deadline_) stopped = true;
        return stopped;
    }
};

}  // namespace

SearchResult findBestMove(const Board& board, int maxDepth, int timeLimitMs) {
    auto start = Clock::now();
    Searcher searcher(start + std::chrono::milliseconds(timeLimitMs));
    SearchResult result;

    std::vector<Move> moves = board.legalMoves();
    if (moves.empty()) return result;
    orderMoves(board, moves);
    result.hasMove = true;
    result.bestMove = moves[0];

    // Iterative deepening: depth 1, 2, 3... until time runs out
    for (int depth = 1; depth <= maxDepth; ++depth) {
        int alpha = -INF;
        int bestScore = -INF;
        Move bestThisDepth = moves[0];

        for (const Move& move : moves) {
            int score = -searcher.negamax(board.afterMove(move), depth - 1, -INF, -alpha, 1);
            if (searcher.stopped) break;
            if (score > bestScore) {
                bestScore = score;
                bestThisDepth = move;
            }
            alpha = std::max(alpha, score);
        }

        if (searcher.stopped) break;  // unfinished depth: keep the previous result

        result.bestMove = bestThisDepth;
        result.score = bestScore;
        result.depth = depth;

        // Search the best move first in the next iteration
        auto it = std::find(moves.begin(), moves.end(), bestThisDepth);
        std::rotate(moves.begin(), it, it + 1);

        if (bestScore >= MATE_SCORE - 100) break;  // forced mate found, no need to look deeper
    }

    result.nodes = searcher.nodes;
    result.timeMs = std::chrono::duration<double, std::milli>(Clock::now() - start).count();
    return result;
}