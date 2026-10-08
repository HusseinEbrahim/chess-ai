#pragma once
#include "board.h"

constexpr int PIECE_VALUES[7] = {0, 100, 320, 330, 500, 900, 0};

// Score in centipawns from the point of view of the side to move
int evaluate(const Board& board);