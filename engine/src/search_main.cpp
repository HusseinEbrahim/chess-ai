#include <iostream>
#include <string>
#include "board.h"
#include "search.h"

int main(int argc, char* argv[]) {
    std::string fen = argc > 1 ? argv[1] : Board::START_FEN;
    int timeMs = argc > 2 ? std::stoi(argv[2]) : 2000;

    Board board(fen);
    SearchResult result = findBestMove(board, 64, timeMs);

    if (!result.hasMove) {
        std::cout << "No legal moves\n";
        return 0;
    }
    std::cout << "Best move: " << moveToUci(result.bestMove) << '\n'
              << "Score:     " << result.score << " centipawns\n"
              << "Depth:     " << result.depth << '\n'
              << "Nodes:     " << result.nodes << '\n'
              << "Time:      " << result.timeMs << " ms\n";
    return 0;
}