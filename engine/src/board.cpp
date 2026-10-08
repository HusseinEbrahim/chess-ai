#include "board.h"
#include <cctype>
#include <cstdlib>
#include <sstream>
#include <stdexcept>

namespace {

int fileOf(int sq) { return sq % 8; }
int rankOf(int sq) { return sq / 8; }
bool onBoard(int file, int rank) { return file >= 0 && file < 8 && rank >= 0 && rank < 8; }

const int KNIGHT_OFFSETS[8][2] = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};
const int KING_OFFSETS[8][2] = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};
const int DIAGONALS[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
const int STRAIGHTS[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

int pieceFromChar(char c) {
    int type;
    switch (std::tolower(static_cast<unsigned char>(c))) {
        case 'p': type = PAWN; break;
        case 'n': type = KNIGHT; break;
        case 'b': type = BISHOP; break;
        case 'r': type = ROOK; break;
        case 'q': type = QUEEN; break;
        case 'k': type = KING; break;
        default: throw std::invalid_argument("Invalid piece in FEN");
    }
    return std::isupper(static_cast<unsigned char>(c)) ? type : -type;
}

char charFromPiece(int piece) {
    char c = ".pnbrqk"[std::abs(piece)];
    return piece > 0 ? static_cast<char>(std::toupper(c)) : c;
}

}  // namespace

std::string squareName(int sq) {
    return {static_cast<char>('a' + fileOf(sq)), static_cast<char>('1' + rankOf(sq))};
}

std::string moveToUci(const Move& move) {
    std::string uci = squareName(move.from) + squareName(move.to);
    if (move.promotion != EMPTY) uci += ".pnbrqk"[move.promotion];
    return uci;
}

Board::Board() : Board(START_FEN) {}

Board::Board(const std::string& fen) {
    loadFen(fen);
}

void Board::loadFen(const std::string& fen) {
    std::istringstream in(fen);
    std::string placement, side, castling, ep;
    in >> placement >> side >> castling >> ep;
    if (placement.empty() || (side != "w" && side != "b")) throw std::invalid_argument("Invalid FEN");
    if (!(in >> halfmove_)) halfmove_ = 0;
    if (!(in >> fullmove_)) fullmove_ = 1;

    squares_.fill(EMPTY);
    int rank = 7, file = 0;
    for (char c : placement) {
        if (c == '/') {
            --rank;
            file = 0;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            file += c - '0';
        } else {
            if (!onBoard(file, rank)) throw std::invalid_argument("Invalid FEN");
            squares_[rank * 8 + file] = static_cast<int8_t>(pieceFromChar(c));
            ++file;
        }
    }

    side_ = side == "w" ? WHITE : BLACK;

    castling_ = 0;
    for (char c : castling) {
        if (c == 'K') castling_ |= WHITE_KINGSIDE;
        if (c == 'Q') castling_ |= WHITE_QUEENSIDE;
        if (c == 'k') castling_ |= BLACK_KINGSIDE;
        if (c == 'q') castling_ |= BLACK_QUEENSIDE;
    }

    epSquare_ = -1;
    if (ep.size() == 2 && ep != "--") epSquare_ = (ep[1] - '1') * 8 + (ep[0] - 'a');

    if (kingSquare(WHITE) == -1 || kingSquare(BLACK) == -1) {
        throw std::invalid_argument("Both sides need a king");
    }
}

std::string Board::fen() const {
    std::string out;
    for (int rank = 7; rank >= 0; --rank) {
        int empty = 0;
        for (int file = 0; file < 8; ++file) {
            int piece = squares_[rank * 8 + file];
            if (piece == EMPTY) {
                ++empty;
                continue;
            }
            if (empty) {
                out += static_cast<char>('0' + empty);
                empty = 0;
            }
            out += charFromPiece(piece);
        }
        if (empty) out += static_cast<char>('0' + empty);
        if (rank > 0) out += '/';
    }

    out += side_ == WHITE ? " w " : " b ";

    std::string rights;
    if (castling_ & WHITE_KINGSIDE) rights += 'K';
    if (castling_ & WHITE_QUEENSIDE) rights += 'Q';
    if (castling_ & BLACK_KINGSIDE) rights += 'k';
    if (castling_ & BLACK_QUEENSIDE) rights += 'q';
    out += rights.empty() ? "-" : rights;

    out += ' ';
    out += epSquare_ == -1 ? "-" : squareName(epSquare_);
    out += ' ' + std::to_string(halfmove_) + ' ' + std::to_string(fullmove_);
    return out;
}

int Board::kingSquare(int side) const {
    for (int sq = 0; sq < 64; ++sq) {
        if (squares_[sq] == side * KING) return sq;
    }
    return -1;
}

bool Board::isSquareAttacked(int sq, int bySide) const {
    int f = fileOf(sq), r = rankOf(sq);

    // Pawns attack diagonally forward, so look one rank "behind" the square
    int pawnRank = r - bySide;
    for (int df : {-1, 1}) {
        if (onBoard(f + df, pawnRank) && squares_[pawnRank * 8 + f + df] == bySide * PAWN) return true;
    }

    for (const auto& o : KNIGHT_OFFSETS) {
        int nf = f + o[0], nr = r + o[1];
        if (onBoard(nf, nr) && squares_[nr * 8 + nf] == bySide * KNIGHT) return true;
    }

    for (const auto& o : KING_OFFSETS) {
        int nf = f + o[0], nr = r + o[1];
        if (onBoard(nf, nr) && squares_[nr * 8 + nf] == bySide * KING) return true;
    }

    auto slidingAttack = [&](const int (&dirs)[4][2], int pieceA, int pieceB) {
        for (const auto& d : dirs) {
            int nf = f + d[0], nr = r + d[1];
            while (onBoard(nf, nr)) {
                int piece = squares_[nr * 8 + nf];
                if (piece != EMPTY) {
                    if (piece == bySide * pieceA || piece == bySide * pieceB) return true;
                    break;
                }
                nf += d[0];
                nr += d[1];
            }
        }
        return false;
    };

    return slidingAttack(DIAGONALS, BISHOP, QUEEN) || slidingAttack(STRAIGHTS, ROOK, QUEEN);
}

void Board::generatePseudoLegal(std::vector<Move>& moves) const {
    const int us = side_;
    auto isEnemy = [&](int piece) { return piece * us < 0; };

    auto addPawnMove = [&](int from, int to) {
        int toRank = rankOf(to);
        if (toRank == 0 || toRank == 7) {
            for (int promo : {QUEEN, ROOK, BISHOP, KNIGHT}) moves.push_back({from, to, promo});
        } else {
            moves.push_back({from, to, EMPTY});
        }
    };

    for (int sq = 0; sq < 64; ++sq) {
        int piece = squares_[sq];
        if (piece * us <= 0) continue;  // empty square or enemy piece

        int type = std::abs(piece);
        int f = fileOf(sq), r = rankOf(sq);

        if (type == PAWN) {
            int forward = r + us;
            if (onBoard(f, forward) && squares_[forward * 8 + f] == EMPTY) {
                addPawnMove(sq, forward * 8 + f);
                int startRank = us == WHITE ? 1 : 6;
                int twoAhead = r + 2 * us;
                if (r == startRank && squares_[twoAhead * 8 + f] == EMPTY) {
                    moves.push_back({sq, twoAhead * 8 + f, EMPTY});
                }
            }
            for (int df : {-1, 1}) {
                int nf = f + df;
                if (!onBoard(nf, forward)) continue;
                int target = forward * 8 + nf;
                if (isEnemy(squares_[target])) addPawnMove(sq, target);
                else if (target == epSquare_) moves.push_back({sq, target, EMPTY});
            }
        } else if (type == KNIGHT || type == KING) {
            const auto& offsets = type == KNIGHT ? KNIGHT_OFFSETS : KING_OFFSETS;
            for (const auto& o : offsets) {
                int nf = f + o[0], nr = r + o[1];
                if (!onBoard(nf, nr)) continue;
                int target = nr * 8 + nf;
                if (squares_[target] == EMPTY || isEnemy(squares_[target])) moves.push_back({sq, target, EMPTY});
            }
        } else {
            auto slide = [&](const int (&dirs)[4][2]) {
                for (const auto& d : dirs) {
                    int nf = f + d[0], nr = r + d[1];
                    while (onBoard(nf, nr)) {
                        int target = nr * 8 + nf;
                        if (squares_[target] == EMPTY) {
                            moves.push_back({sq, target, EMPTY});
                        } else {
                            if (isEnemy(squares_[target])) moves.push_back({sq, target, EMPTY});
                            break;
                        }
                        nf += d[0];
                        nr += d[1];
                    }
                }
            };
            if (type == BISHOP || type == QUEEN) slide(DIAGONALS);
            if (type == ROOK || type == QUEEN) slide(STRAIGHTS);
        }
    }

    // Castling: rights available, squares between empty, and the king doesn't start in,
    // pass through, or land on an attacked square (landing is checked by the legality filter)
    int kingSq = us == WHITE ? 4 : 60;
    if (squares_[kingSq] == us * KING && !isSquareAttacked(kingSq, -us)) {
        uint8_t kingside = us == WHITE ? WHITE_KINGSIDE : BLACK_KINGSIDE;
        uint8_t queenside = us == WHITE ? WHITE_QUEENSIDE : BLACK_QUEENSIDE;

        if ((castling_ & kingside) && squares_[kingSq + 1] == EMPTY && squares_[kingSq + 2] == EMPTY &&
            squares_[kingSq + 3] == us * ROOK && !isSquareAttacked(kingSq + 1, -us)) {
            moves.push_back({kingSq, kingSq + 2, EMPTY});
        }
        if ((castling_ & queenside) && squares_[kingSq - 1] == EMPTY && squares_[kingSq - 2] == EMPTY &&
            squares_[kingSq - 3] == EMPTY && squares_[kingSq - 4] == us * ROOK &&
            !isSquareAttacked(kingSq - 1, -us)) {
            moves.push_back({kingSq, kingSq - 2, EMPTY});
        }
    }
}

Board Board::afterMove(const Move& move) const {
    Board next = *this;
    int piece = squares_[move.from];
    int type = std::abs(piece);
    int captured = squares_[move.to];

    next.squares_[move.to] = static_cast<int8_t>(move.promotion != EMPTY ? side_ * move.promotion : piece);
    next.squares_[move.from] = EMPTY;

    // En passant: remove the pawn that just moved two squares
    if (type == PAWN && move.to == epSquare_) {
        next.squares_[move.to - 8 * side_] = EMPTY;
    }

    // Castling: move the rook too
    if (type == KING && std::abs(move.to - move.from) == 2) {
        if (move.to > move.from) {
            next.squares_[move.from + 1] = next.squares_[move.from + 3];
            next.squares_[move.from + 3] = EMPTY;
        } else {
            next.squares_[move.from - 1] = next.squares_[move.from - 4];
            next.squares_[move.from - 4] = EMPTY;
        }
    }

    // A pawn moving two squares creates an en passant target behind it
    next.epSquare_ = (type == PAWN && std::abs(move.to - move.from) == 16) ? (move.from + move.to) / 2 : -1;

    // Moving the king loses both castling rights; moving or capturing a rook loses that side's right
    if (type == KING) {
        next.castling_ &= side_ == WHITE ? ~(WHITE_KINGSIDE | WHITE_QUEENSIDE) : ~(BLACK_KINGSIDE | BLACK_QUEENSIDE);
    }
    auto clearRookRight = [&](int sq) {
        if (sq == 0) next.castling_ &= ~WHITE_QUEENSIDE;
        if (sq == 7) next.castling_ &= ~WHITE_KINGSIDE;
        if (sq == 56) next.castling_ &= ~BLACK_QUEENSIDE;
        if (sq == 63) next.castling_ &= ~BLACK_KINGSIDE;
    };
    clearRookRight(move.from);
    clearRookRight(move.to);

    next.halfmove_ = (type == PAWN || captured != EMPTY) ? 0 : halfmove_ + 1;
    if (side_ == BLACK) ++next.fullmove_;
    next.side_ = -side_;
    return next;
}

std::vector<Move> Board::legalMoves() const {
    std::vector<Move> pseudo;
    pseudo.reserve(64);
    generatePseudoLegal(pseudo);

    std::vector<Move> legal;
    legal.reserve(pseudo.size());
    for (const Move& move : pseudo) {
        Board next = afterMove(move);
        if (!next.isSquareAttacked(next.kingSquare(side_), -side_)) legal.push_back(move);
    }
    return legal;
}

bool Board::inCheck() const {
    return isSquareAttacked(kingSquare(side_), -side_);
}

bool Board::parseUci(const std::string& uci, Move& out) const {
    for (const Move& move : legalMoves()) {
        if (moveToUci(move) == uci) {
            out = move;
            return true;
        }
    }
    return false;
}