#pragma once

#include "chess.hpp"

class ChessEngine
{
    Move search(const ChessBoard* position, int depth);
    float negamax(
        ChessBoard* board,
        int depth,
        float alpha,
        float beta);

public:
    ChessEngine();
    ~ChessEngine();

    float eval(const ChessBoard* position);
    Move make_move(const ChessBoard* board);
};
