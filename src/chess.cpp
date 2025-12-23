#include "chess.hpp"

static inline uint8_t compact_coords(uint8_t x, uint8_t y)
{
    return (x << 4) | y;
}

static inline bool in_bounds(int x, int y)
{
    return (unsigned)x < 8 && (unsigned)y < 8;
}

ChessBoard::ChessBoard()
{
    for (int x = 0; x < 8; x ++)
    {
        for (int y = 0; y < 8; y ++)
        {
            board[x][y] = {PieceType::NONE, PieceColor::WHITE};
        }
    }
}

ChessBoard::~ChessBoard()
{}

void ChessBoard::make_move(const Move* move)
{
    uint8_t to_x   = move->to >> 4;
    uint8_t to_y   = move->to & 0x0F;
    uint8_t from_x = move->from >> 4;
    uint8_t from_y = move->from & 0x0F;

    HistoryMove m;
    m.from = move->from;
    m.to   = move->to;
    m.p    = move->p;
    m.captured = board[to_x][to_y];

    board[from_x][from_y].type = PieceType::NONE;
    board[to_x][to_y] = move->p;

    history.push_back(m);

    turn = (turn == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;
}

void ChessBoard::undo_move()
{
    HistoryMove m = history.back();
    history.pop_back();

    uint8_t to_x   = m.to >> 4;
    uint8_t to_y   = m.to & 0x0F;
    uint8_t from_x = m.from >> 4;
    uint8_t from_y = m.from & 0x0F;

    board[from_x][from_y] = m.p;
    board[to_x][to_y] = m.captured;
    turn = (turn == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;
}

void ChessBoard::load_fen(const std::string& fen)
{
    // Clear board
    for (int x = 0; x < 8; ++x)
        for (int y = 0; y < 8; ++y)
            board[x][y] = {PieceType::NONE, PieceColor::WHITE};

    history.clear();

    std::istringstream ss(fen);
    std::string board_part;
    ss >> board_part;

    int x = 0;
    int y = 7;

    for (char c : board_part)
    {
        if (c == '/')
        {
            x = 0;
            y--;
            if (y < 0)
                throw std::runtime_error("Invalid FEN: too many ranks");
            continue;
        }

        if (std::isdigit(c))
        {
            x += c - '0';
            if (x > 8)
                throw std::runtime_error("Invalid FEN: rank overflow");
            continue;
        }

        ChessPiece p;
        p.color = std::isupper(c) ? PieceColor::WHITE : PieceColor::BLACK;

        switch (std::tolower(c))
        {
            case 'p': p.type = PieceType::PAWN; break;
            case 'n': p.type = PieceType::KNIGHT; break;
            case 'b': p.type = PieceType::BISHOP; break;
            case 'r': p.type = PieceType::ROOK; break;
            case 'q': p.type = PieceType::QUEEN; break;
            case 'k': p.type = PieceType::KING; break;
            default:
                throw std::runtime_error("Invalid FEN: unknown piece");
        }

        if (x >= 8 || y < 0)
            throw std::runtime_error("Invalid FEN: board overflow");

        board[x][y] = p;
        x++;
    }

    std::string turn;
    ss >> turn;
    this->turn = (turn == "w") ? PieceColor::WHITE : PieceColor::BLACK;

    if (y != 0 || x != 8)
        throw std::runtime_error("Invalid FEN: incomplete board");
}

std::vector<Move> ChessBoard::get_moves()
{
    std::vector<Move> moves;

    for (uint8_t i = 0; i < 64; i ++)
    {
        uint8_t x = i % 8;
        uint8_t y = i / 8;
        const ChessPiece p = this->board[x][y];

        if (p.type == PieceType::NONE)
            continue;
        
        std::vector<Move> m;
        switch (p.type)
        {
            // All moves for pawns
            case PieceType::PAWN:
            {
                m = get_pawn_moves(x, y, p);
                break;
            }
            
            // All moves for knights
            case PieceType::KNIGHT:
            {
                m = get_knight_moves(x, y, p);
                break;
            }

            case PieceType::BISHOP:
            {
                m = get_bishop_moves(x, y, p);
                break;
            }

            case PieceType::ROOK:
            {
                m = get_rook_moves(x, y, p);
                break;
            }

            case PieceType::QUEEN:
            {
                m = get_queen_moves(x, y, p);
                break;
            }

            case PieceType::KING:
            {
                m = get_king_moves(x, y, p);
                break;
            }

            default:
                break;
        }

        moves.insert(moves.end(), m.begin(), m.end());
    }

    return moves;
}

std::vector<Move> ChessBoard::get_pawn_moves(uint8_t x, uint8_t y, ChessPiece p)
{
    std::vector<Move> moves;
    if (p.color == PieceColor::WHITE)
    {
        if (in_bounds(x, y + 1) && board[x][y + 1].type == PieceType::NONE)
        {
            moves.push_back({p, compact_coords(x, y + 1), compact_coords(x, y)});

            if (y == 1 &&
                board[x][y + 1].type == PieceType::NONE &&
                board[x][y + 2].type == PieceType::NONE)
            {
                moves.push_back({p, compact_coords(x, y + 2), compact_coords(x, y)});
            }

        }

        for (int dx : {-1, 1})
        {
            int nx = x + dx;
            int ny = y + 1;

            if (in_bounds(nx, ny) &&
                board[nx][ny].type != PieceType::NONE &&
                board[nx][ny].color == PieceColor::BLACK)
            {
                moves.push_back({p,
                    compact_coords(nx, ny),
                    compact_coords(x, y)});
            }
        }
    }
    else if (p.color == PieceColor::BLACK)
    {
        if (in_bounds(x, y - 1) && board[x][y - 1].type == PieceType::NONE)
        {
            moves.push_back({p, compact_coords(x, y - 1), compact_coords(x, y)});

            if (y == 6 &&
                board[x][y - 1].type == PieceType::NONE &&
                board[x][y - 2].type == PieceType::NONE)
            {
                moves.push_back({p, compact_coords(x, y - 2), compact_coords(x, y)});
            }

        }

        for (int dx : {-1, 1})
        {
            int nx = x + dx;
            int ny = y - 1;

            if (in_bounds(nx, ny) &&
                board[nx][ny].type != PieceType::NONE &&
                board[nx][ny].color == PieceColor::WHITE)
            {
                moves.push_back({p,
                    compact_coords(nx, ny),
                    compact_coords(x, y)});
            }
        }
    }

    return moves;
}

std::vector<Move> ChessBoard::get_knight_moves(uint8_t x, uint8_t y, ChessPiece p)
{
    std::vector<Move> moves;
    moves.reserve(8);
    static constexpr int8_t offsets[8][2] = {
        { 2,  1}, { 1,  2},
        {-1,  2}, {-2,  1},
        {-2, -1}, {-1, -2},
        { 1, -2}, { 2, -1}
    };

    for (const auto& o : offsets)
    {
        int nx = x + o[0];
        int ny = y + o[1];

        if (!in_bounds(nx, ny))
            continue;

        ChessPiece target = board[nx][ny];

        if (target.type == PieceType::NONE ||
            target.color != p.color)
        {
            moves.push_back({p,
                compact_coords(nx, ny),
                compact_coords(x, y)});
        }
    }

    return moves;
}

std::vector<Move> ChessBoard::get_bishop_moves(uint8_t x, uint8_t y, ChessPiece p)
{
    std::vector<Move> moves;
    static constexpr int8_t dirs[4][2] = {
        { 1,  1}, {-1,  1},
        { 1, -1}, {-1, -1}
    };

    for (auto& d : dirs)
    {
        int cx = x + d[0];
        int cy = y + d[1];

        while (in_bounds(cx, cy))
        {
            if (board[cx][cy].type == PieceType::NONE)
            {
                moves.push_back({p,
                    compact_coords(cx, cy),
                    compact_coords(x, y)});
            }
            else
            {
                if (board[cx][cy].color != p.color)
                {
                    moves.push_back({p,
                        compact_coords(cx, cy),
                        compact_coords(x, y)});
                }
                break;
            }
            cx += d[0];
            cy += d[1];
        }
    }

    return moves;
}

std::vector<Move> ChessBoard::get_rook_moves(uint8_t x, uint8_t y, ChessPiece p)
{
    std::vector<Move> moves;
    static constexpr int8_t dirs[4][2] = {
        { 1,  0}, {-1,  0},
        { 0,  1}, { 0, -1}
    };

    for (auto& d : dirs)
    {
        int cx = x + d[0];
        int cy = y + d[1];

        while (in_bounds(cx, cy))
        {
            if (board[cx][cy].type == PieceType::NONE)
            {
                moves.push_back({p,
                    compact_coords(cx, cy),
                    compact_coords(x, y)});
            }
            else
            {
                if (board[cx][cy].color != p.color)
                {
                    moves.push_back({p,
                        compact_coords(cx, cy),
                        compact_coords(x, y)});
                }
                break;
            }
            cx += d[0];
            cy += d[1];
        }
    }

    return moves;
}

std::vector<Move> ChessBoard::get_queen_moves(uint8_t x, uint8_t y, ChessPiece p)
{
    std::vector<Move> moves;
    static constexpr int8_t dirs[8][2] = {
        { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1},
        { 1,  1}, {-1,  1}, { 1, -1}, {-1, -1}
    };

    for (auto& d : dirs)
    {
        int cx = x + d[0];
        int cy = y + d[1];

        while (in_bounds(cx, cy))
        {
            if (board[cx][cy].type == PieceType::NONE)
            {
                moves.push_back({p,
                    compact_coords(cx, cy),
                    compact_coords(x, y)});
            }
            else
            {
                if (board[cx][cy].color != p.color)
                {
                    moves.push_back({p,
                        compact_coords(cx, cy),
                        compact_coords(x, y)});
                }
                break;
            }
            cx += d[0];
            cy += d[1];
        }
    }

    return moves;
}

std::vector<Move> ChessBoard::get_king_moves(uint8_t x, uint8_t y, ChessPiece p)
{
    std::vector<Move> moves;
    for (int dx = -1; dx <= 1; dx++)
    {
        for (int dy = -1; dy <= 1; dy++)
        {
            if (dx == 0 && dy == 0)
                continue;

            int nx = x + dx;
            int ny = y + dy;

            if (!in_bounds(nx, ny))
                continue;

            if (board[nx][ny].type == PieceType::NONE ||
                board[nx][ny].color != p.color)
            {
                moves.push_back({p,
                    compact_coords(nx, ny),
                    compact_coords(x, y)});
            }
        }
    }

    return moves;
}

bool ChessBoard::is_valid_move(const Move* move)
{
    // Ensure the "from" square actually has the piece
    uint8_t from_x = move->from >> 4;
    uint8_t from_y = move->from & 0x0F;
    const ChessPiece& p = board[from_x][from_y];

    if (p.type == PieceType::NONE)
        return false;

    if (p.color != turn)
        return false;

    // Generate all possible moves for this piece
    std::vector<Move> possible_moves;
    switch (p.type)
    {
        case PieceType::PAWN:   possible_moves = get_pawn_moves(from_x, from_y, p); break;
        case PieceType::KNIGHT: possible_moves = get_knight_moves(from_x, from_y, p); break;
        case PieceType::BISHOP: possible_moves = get_bishop_moves(from_x, from_y, p); break;
        case PieceType::ROOK:   possible_moves = get_rook_moves(from_x, from_y, p); break;
        case PieceType::QUEEN:  possible_moves = get_queen_moves(from_x, from_y, p); break;
        case PieceType::KING:   possible_moves = get_king_moves(from_x, from_y, p); break;
        default: return false;
    }

    // Check if the move is among the possible moves
    bool found = false;
    for (auto& m : possible_moves)
    {
        if (m.to == move->to && m.from == move->from)
        {
            found = true;
            break;
        }
    }

    if (!found)
        return false;

    // Make the move temporarily to check if king is in check
    make_move(move);
    bool leaves_king_in_check = is_check();
    undo_move();

    return !leaves_king_in_check;
}

bool ChessBoard::is_check()
{
    int kx = -1, ky = -1;

    // Find king
    for (int i = 0; i < 64; ++i)
    {
        int x = i % 8;
        int y = i / 8;
        if (board[x][y].type == PieceType::KING &&
            board[x][y].color == turn)
        {
            kx = x;
            ky = y;
            break;
        }
    }

    if (kx == -1)
        return false; // no king found (invalid board)

    PieceColor enemy =
        (turn == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;

    // Generate enemy moves only
    for (int i = 0; i < 64; ++i)
    {
        int x = i % 8;
        int y = i / 8;
        ChessPiece p = board[x][y];

        if (p.type == PieceType::NONE || p.color != enemy)
            continue;

        std::vector<Move> moves;
        switch (p.type)
        {
            case PieceType::PAWN:   moves = get_pawn_moves(x, y, p); break;
            case PieceType::KNIGHT: moves = get_knight_moves(x, y, p); break;
            case PieceType::BISHOP: moves = get_bishop_moves(x, y, p); break;
            case PieceType::ROOK:   moves = get_rook_moves(x, y, p); break;
            case PieceType::QUEEN:  moves = get_queen_moves(x, y, p); break;
            case PieceType::KING:   moves = get_king_moves(x, y, p); break;
            default:
                break;
        }

        for (auto& m : moves)
        {
            if (m.to == compact_coords(kx, ky))
                return true;
        }
    }
    return false;
}

bool ChessBoard::is_checkmate()
{
    if (!is_check())
        return false;

    auto moves = get_moves();
    for (auto& m : moves)
    {
        if (m.p.color != turn)
            continue;

        make_move(&m);
        bool still_in_check = is_check();
        undo_move();

        if (!still_in_check)
            return false;
    }
    return true;
}

static char piece_to_char(const ChessPiece& p)
{
    if (p.type == PieceType::NONE)
        return '.';

    char c = '?';
    switch (p.type)
    {
        case PieceType::PAWN:   c = 'P'; break;
        case PieceType::KNIGHT: c = 'N'; break;
        case PieceType::BISHOP: c = 'B'; break;
        case PieceType::ROOK:   c = 'R'; break;
        case PieceType::QUEEN:  c = 'Q'; break;
        case PieceType::KING:   c = 'K'; break;
        default: break;
    }

    if (p.color == PieceColor::BLACK)
        c = std::tolower(c);

    return c;
}

void ChessBoard::print() const
{
    std::cout << "\n";
    for (int y = 7; y >= 0; --y)
    {
        std::cout << (y + 1) << "  ";
        for (int x = 0; x < 8; ++x)
        {
            std::cout << piece_to_char(board[x][y]) << " ";
        }
        std::cout << "\n";
    }

    std::cout << "\n   a b c d e f g h\n\n";
}