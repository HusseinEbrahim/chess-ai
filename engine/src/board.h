#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

enum Piece : int8_t { EMPTY = 0, PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6 };
constexpr int WHITE = 1;
constexpr int BLACK = -1;

enum CastlingRight : uint8_t {
    WHITE_KINGSIDE = 1,
    WHITE_QUEENSIDE = 2,
    BLACK_KINGSIDE = 4,
    BLACK_QUEENSIDE = 8,
};

struct Move {
    int from = 0;
    int to = 0;
    int promotion = EMPTY;  // piece type to promote to, or EMPTY
    bool operator==(const Move&) const = default;
};

std::string squareName(int sq);
std::string moveToUci(const Move& move);  // e.g. "e2e4", "e7e8q"

class Board {
public:
    static constexpr const char* START_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    Board();
    explicit Board(const std::string& fen);
    std::string fen() const;

    int sideToMove() const { return side_; }
    int pieceAt(int sq) const { return squares_[sq]; }  // positive = white, negative = black

    std::vector<Move> legalMoves() const;
    Board afterMove(const Move& move) const;  // returns a new board with the move played
    bool inCheck() const;
    bool isSquareAttacked(int sq, int bySide) const;
    bool parseUci(const std::string& uci, Move& out) const;

private:
    std::array<int8_t, 64> squares_{};
    int side_ = WHITE;
    uint8_t castling_ = 0;
    int epSquare_ = -1;  // en passant target square, or -1
    int halfmove_ = 0;
    int fullmove_ = 1;

    void loadFen(const std::string& fen);
    void generatePseudoLegal(std::vector<Move>& moves) const;
    int kingSquare(int side) const;
};