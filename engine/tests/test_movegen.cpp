#include <gtest/gtest.h>
#include "board.h"
#include "perft.h"

// Known correct perft results from the chess programming community

TEST(Perft, StartingPosition) {
    EXPECT_EQ(perft(Board(), 4), 197281u);
}

TEST(Perft, Kiwipete) {
    Board board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    EXPECT_EQ(perft(board, 3), 97862u);
}

TEST(Perft, EndgameWithEnPassant) {
    Board board("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    EXPECT_EQ(perft(board, 4), 43238u);
}

TEST(Perft, PromotionsAndCastling) {
    Board board("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    EXPECT_EQ(perft(board, 3), 9467u);
}

TEST(Perft, TrickyMiddlegame) {
    Board board("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
    EXPECT_EQ(perft(board, 3), 62379u);
}

TEST(Board, FenRoundTrip) {
    const std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
    EXPECT_EQ(Board(fen).fen(), fen);
}

TEST(Board, DetectsFoolsMate) {
    Board board("rnb1kbnr/pppp1ppp/8/4p3/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
    EXPECT_TRUE(board.inCheck());
    EXPECT_TRUE(board.legalMoves().empty());
}

TEST(Board, ParsesAndPlaysUciMoves) {
    Board board;
    Move move;
    EXPECT_FALSE(board.parseUci("e2e5", move));
    ASSERT_TRUE(board.parseUci("e2e4", move));
    EXPECT_EQ(board.afterMove(move).fen(), "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
}