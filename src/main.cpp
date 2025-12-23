#include <iostream>
#include "chess.hpp"

int main()
{
    ChessBoard board;
    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    board.print();

    return 0;
}
