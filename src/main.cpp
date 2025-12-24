#include "chess.hpp"
#include "engine.hpp"
#include <iostream>
#include <string>
#include <cctype>
#include <stdexcept>

// Convert algebraic like "e2e4" -> from/to packed coordinates
Move parse_move(const std::string& input, const ChessBoard& board)
{
    if (input.length() != 4)
        throw std::invalid_argument("Move must be 4 characters (e.g., e2e4)");

    auto to_coord = [](char file, char rank) -> uint8_t
    {
        int x = file - 'a';
        int y = rank - '1';
        if (x < 0 || x >= 8 || y < 0 || y >= 8)
            throw std::out_of_range("Invalid coordinates");
        return (x << 4) | y;
    };

    uint8_t from = to_coord(input[0], input[1]);
    uint8_t to   = to_coord(input[2], input[3]);
    uint8_t fx = from >> 4, fy = from & 0x0F;

    ChessPiece p = board.board[fx][fy];
    if (p.type == PieceType::NONE)
        throw std::runtime_error("No piece at the source square");

    return Move{p, to, from};
}

// Convert packed coordinates -> algebraic like "e2"
std::string coord_to_alg(uint8_t coord)
{
    int x = coord >> 4;
    int y = coord & 0x0F;
    std::string s;
    s += ('a' + x);
    s += ('1' + y);
    return s;
}

int main()
{
    ChessBoard board;
    ChessEngine engine;

    std::cout << "Chess Console" << std::endl;
    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w"); // start position
    board.print();

    std::string input;
    while (true)
    {
        std::cout << (board.turn == PieceColor::WHITE ? "White" : "Black") << "> ";
        std::getline(std::cin, input);

        if (input == "exit")
            break;
        else if (input == "print")
            board.print();
        else if (input == "undo")
        {
            board.undo_move();
            board.print();
        }
        else if (input == "engine")
        {
            Move m = engine.make_move(&board);
            board.make_move(&m);
            std::cout << "Engine moves: " << coord_to_alg(m.from) << coord_to_alg(m.to) << "\n";
            board.print();
        }
        else if (input == "fen")
        {
            std::cout << "Enter FEN: ";
            std::string fen_input;
            std::getline(std::cin, fen_input);

            try
            {
                board.load_fen(fen_input);
                board.print();
            }
            catch (const std::exception& e)
            {
                std::cout << "Invalid FEN: " << e.what() << "\n";
            }
        }
        else if (input == "eval")
        {
            std::cout << "Engine eval: " << engine.eval(&board) << std::endl;
        }
        else if (input.length() == 4)
        {
            try
            {
                Move m = parse_move(input, board);
                if (!board.is_valid_move(&m))
                {
                    std::cout << "Invalid move!\n";
                    continue;
                }

                board.make_move(&m);
                board.print();
            }
            catch (const std::exception& e)
            {
                std::cout << "Error: " << e.what() << "\n";
            }
        }
        else
        {
            std::cout << "Unknown command\n";
        }

        if (board.is_checkmate())
        {
            std::cout << "Checkmate! ";
            std::cout << (board.turn == PieceColor::WHITE ? "Black wins!\n" : "White wins!\n");
            break;
        }
        else if (board.is_check(board.turn))
        {
            std::cout << "Check!\n";
        }
    }

    return 0;
}
