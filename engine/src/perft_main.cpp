#include <chrono>
#include <iostream>
#include <string>
#include "board.h"
#include "perft.h"

int main(int argc, char* argv[]) {
    int depth = argc > 1 ? std::stoi(argv[1]) : 5;
    std::string fen = argc > 2 ? argv[2] : Board::START_FEN;

    Board board(fen);
    auto start = std::chrono::steady_clock::now();
    uint64_t nodes = perft(board, depth);
    double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();

    std::cout << "Depth " << depth << ": " << nodes << " nodes in " << seconds << " s ("
              << static_cast<uint64_t>(nodes / seconds) << " nodes/sec)\n";
    return 0;
}