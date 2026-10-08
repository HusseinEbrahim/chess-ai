#pragma once
#include <cstdint>
#include "board.h"

inline uint64_t perft(const Board& board, int depth) {
    if (depth == 0) return 1;
    std::vector<Move> moves = board.legalMoves();
    if (depth == 1) return moves.size();

    uint64_t nodes = 0;
    for (const Move& move : moves) nodes += perft(board.afterMove(move), depth - 1);
    return nodes;
}