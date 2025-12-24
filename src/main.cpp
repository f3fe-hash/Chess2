#include "chess.hpp"
#include "engine.hpp"

#include <ncurses.h>
#include <string>
#include <cctype>
#include <cmath>

char piece_char(const ChessPiece& p)
{
    if (p.type == PieceType::NONE) return '.';
    char c;
    switch (p.type)
    {
        case PieceType::PAWN:   c = 'P'; break;
        case PieceType::KNIGHT: c = 'N'; break;
        case PieceType::BISHOP: c = 'B'; break;
        case PieceType::ROOK:   c = 'R'; break;
        case PieceType::QUEEN:  c = 'Q'; break;
        case PieceType::KING:   c = 'K'; break;
        default: c = '?';
    }

    return (p.color == PieceColor::WHITE) ? c : tolower(c);
}

Move parse_move(const std::string& input, const ChessBoard& board)
{
    auto to_coord = [](char f, char r) -> uint8_t
    {
        int x = f - 'a';
        int y = r - '1';
        if (x < 0 || x >= 8 || y < 0 || y >= 8)
            throw std::out_of_range("Invalid square");
        return (x << 4) | y;
    };

    uint8_t from = to_coord(input[0], input[1]);
    uint8_t to   = to_coord(input[2], input[3]);
    uint8_t fx = from >> 4, fy = from & 0x0F;

    ChessPiece p = board.board[fx][fy];
    if (p.type == PieceType::NONE)
        throw std::runtime_error("No piece at source");

    return Move{p, to, from};
}

std::string coord_to_alg(uint8_t c)
{
    return { char('a' + (c >> 4)), char('1' + (c & 0x0F)) };
}

static inline std::string ltrim(const std::string& s)
{
    size_t i = 0;
    while (i < s.size() && std::isspace(s[i])) i++;
    return s.substr(i);
}

void draw_board(WINDOW* win,
                const ChessBoard& board,
                float eval,
                const Move* best_move,
                const std::string& turn)
{
    werase(win);
    box(win, 0, 0);

    // Draw board
    for (int y = 7; y >= 0; y --)
    {
        mvwprintw(win, 1 + (7 - y), 2, "%d", y + 1);
        for (int x = 0; x < 8; x ++)
        {
            mvwprintw(win,
                      1 + (7 - y),
                      4 + x * 2,
                      "%c",
                      piece_char(board.board[x][y]));
        }
    }

    mvwprintw(win, 10, 4, "a b c d e f g h");

    // Engine info
    int panel_x = 22;

    mvwprintw(win, 1, panel_x, "Engine");
    mvwprintw(win, 2, panel_x, "Best Move:");

    if (best_move)
    {
        std::string mv =
            coord_to_alg(best_move->from) +
            coord_to_alg(best_move->to);
        mvwprintw(win, 3, panel_x + 2, "%s", mv.c_str());
    }
    else
        mvwprintw(win, 3, panel_x + 2, "--");

    // Chess position eval
    mvwprintw(win, 5, panel_x, "Eval: %+0.2f", eval);

    // Current turn
    mvwprintw(win, 7, panel_x, "Turn: %s", turn.c_str());

    wrefresh(win);
}

void draw_command(WINDOW* win, const std::string& input, const std::string& status)
{
    werase(win);
    box(win, 0, 0);

    mvwprintw(win, 1, 2, "Command: ");
    mvwprintw(win, 2, 2, "Status: %s", status.c_str());

    mvwprintw(win, 1, 11, "%s", input.c_str());
    wmove(win, 1, 11 + input.size());

    wrefresh(win);
}

int main()
{
    initscr();
    set_escdelay(25);
    curs_set(1);
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    int term_h, term_w;
    getmaxyx(stdscr, term_h, term_w);

    // Layout sizes
    int board_w = 38;  // Board + side panel
    int board_h = 12;
    int cmd_h   = 4;
    int cmd_y = term_h - cmd_h - 1;

    WINDOW* board_win = newwin(board_h, board_w, 1, 1);
    WINDOW* cmd_win   = newwin(cmd_h, term_w - 1, cmd_y, 1);

    ChessBoard board;
    ChessEngine engine;
    board.load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w");

    std::string input;
    std::string status;
    std::string turn;
    char buffer[64];

    float eval = engine.eval(&board);
    Move best_move{};
    bool has_best = false;

    while (true)
    {
        turn = (board.turn == PieceColor::WHITE) ? "White" : "Black";
        draw_board(board_win, board, eval, has_best ? &best_move : nullptr, turn);
        draw_command(cmd_win, input, status);

        echo();
        curs_set(1);
        wmove(cmd_win, 1, 11);
        wgetnstr(cmd_win, buffer, sizeof(buffer) - 1);
        noecho();
        curs_set(0);

        input = buffer;

        if (input == "exit")
            break;
        else if (input == "undo")
        {
            board.undo_move();
            status = "Move undone";
        }
        else if (input == "engine")
        {
            best_move = engine.make_move(&board);
            board.make_move(&best_move);
            has_best = true;
            status = "Engine played " + coord_to_alg(best_move.from) + coord_to_alg(best_move.to);
        }
        else if (input.size() == 4)
        {
            try
            {
                Move m = parse_move(input, board);
                if (!board.is_valid_move(&m))
                    status = "Invalid move";
                else
                {
                    board.make_move(&m);
                    status = "Played " + input;
                }
            }
            catch (const std::exception& e)
            {
                status = e.what();
            }
        }
        else if (input.rfind("fen", 0) == 0)
        {
            try
            {
                std::string fen = ltrim(input.substr(3));

                if (fen.empty())
                    throw std::runtime_error("Usage: fen <fen-string>");

                board.load_fen(fen);

                eval = engine.eval(&board);
                best_move = engine.make_move(&board);
                has_best = true;

                status = "FEN loaded";
            }
            catch (const std::exception& e)
            {
                status = std::string("FEN error: ") + e.what();
            }
        }
        else
            status = "Unknown command";


        eval = engine.eval(&board);
        best_move = engine.make_move(&board);
        has_best = true;

        if (board.is_checkmate())
        {
            status = "Checkmate!";
            draw_board(board_win, board, eval, has_best ? &best_move : nullptr, turn);
            draw_command(cmd_win, input, status);
            break;
        }
        else if (board.is_check(board.turn))
        {
            status += " (Check)";
        }

        input.clear();
    }

    delwin(board_win);
    delwin(cmd_win);
    endwin();
    return 0;
}
