#include <gtest/gtest.h>
#include "board.h"
#include "evaluate.h"
#include "search.h"

TEST(Evaluate, StartingPositionIsEqual) {
    EXPECT_EQ(evaluate(Board()), 0);
}

TEST(Evaluate, ExtraQueenIsWinning) {
    EXPECT_GT(evaluate(Board("4k3/8/8/8/8/8/8/3QK3 w - - 0 1")), 800);
}

TEST(Search, FindsBackRankMate) {
    Board board("6k1/5ppp/8/8/8/8/8/R5K1 w - - 0 1");
    SearchResult result = findBestMove(board, 4, 5000);
    EXPECT_EQ(moveToUci(result.bestMove), "a1a8");
}

TEST(Search, FindsScholarsMate) {
    Board board("r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4");
    SearchResult result = findBestMove(board, 4, 5000);
    EXPECT_EQ(moveToUci(result.bestMove), "h5f7");
}

TEST(Search, CapturesHangingQueen) {
    Board board("4k3/8/8/3q4/8/8/8/3QK3 w - - 0 1");
    SearchResult result = findBestMove(board, 4, 5000);
    EXPECT_EQ(moveToUci(result.bestMove), "d1d5");
}

TEST(Search, NoMoveWhenCheckmated) {
    Board board("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
    EXPECT_FALSE(findBestMove(board, 4, 1000).hasMove);
}