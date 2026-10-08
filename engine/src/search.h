#pragma once
#include <cstdint>
#include "board.h"

constexpr int MATE_SCORE = 100000;

struct SearchResult {
    bool hasMove = false;
    Move bestMove;
    int score = 0;       // centipawns, from the side to move's point of view
    int depth = 0;       // deepest fully completed search
    uint64_t nodes = 0;  // positions examined
    double timeMs = 0.0;
};

SearchResult findBestMove(const Board& board, int maxDepth, int timeLimitMs);