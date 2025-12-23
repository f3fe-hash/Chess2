#pragma once

#include <iostream>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <cstdint>
#include <vector>

enum class PieceType
{
    NONE,
    PAWN,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

enum class PieceColor
{
    NONE,
    WHITE,
    BLACK
};

struct ChessPiece
{
    PieceType type;
    PieceColor color;
};

struct Move
{
    ChessPiece p;
    uint8_t to;   // (4-bit x)(4-bit y)
    uint8_t from; // (4-bit x)(4-bit y)
};

struct HistoryMove
{
    ChessPiece p;
    ChessPiece captured;
    uint8_t to;   // (4-bit x)(4-bit y)
    uint8_t from; // (4-bit x)(4-bit y)

    uint8_t captured_rook_from = 0;
    uint8_t captured_rook_to = 0;
    ChessPiece captured_rook_piece = {PieceType::NONE, PieceColor::WHITE};
};

class ChessBoard
{
    std::vector<HistoryMove> history;

    bool white_king_moved = false;
    bool black_king_moved = false;
    bool white_kingside_rook_moved = false;
    bool white_queenside_rook_moved = false;
    bool black_kingside_rook_moved = false;
    bool black_queenside_rook_moved = false;

    std::vector<Move> get_pawn_moves(uint8_t x, uint8_t y, ChessPiece p);
    std::vector<Move> get_knight_moves(uint8_t x, uint8_t y, ChessPiece p);
    std::vector<Move> get_bishop_moves(uint8_t x, uint8_t y, ChessPiece p);
    std::vector<Move> get_rook_moves(uint8_t x, uint8_t y, ChessPiece p);
    std::vector<Move> get_queen_moves(uint8_t x, uint8_t y, ChessPiece p);
    std::vector<Move> get_king_moves(uint8_t x, uint8_t y, ChessPiece p);
    bool will_be_check(const Move* move);

public:
    ChessBoard();
    ~ChessBoard();

    void make_move(const Move* move);
    void undo_move();

    void load_fen(const std::string& FEN);

    bool is_check();
    bool is_checkmate();
    bool is_valid_move(const Move* move);
    std::vector<Move> get_moves();

    void print() const;

    ChessPiece board[8][8];
    PieceColor turn;
};
