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

    // Optional: bounds check
    if (!in_bounds(from_x, from_y) || !in_bounds(to_x, to_y))
        throw std::out_of_range("Move coordinates out of bounds");

    HistoryMove m;
    m.from = move->from;
    m.to   = move->to;
    m.p    = move->p;
    m.captured = board[to_x][to_y];

    // Move the piece
    board[from_x][from_y].type = PieceType::NONE;
    board[to_x][to_y] = move->p;

    // Castling
    if (move->p.type == PieceType::KING)
    {
        if (from_x == 4 && to_x == 6)
        { // Kingside
            uint8_t rook_from_x = 7;
            uint8_t rook_to_x   = 5;
            uint8_t y = from_y;

            // Save rook in history
            m.captured_rook_from = rook_from_x;
            m.captured_rook_to   = rook_to_x;
            m.captured_rook_piece = board[rook_from_x][y];

            // Move rook
            board[rook_to_x][y] = board[rook_from_x][y];
            board[rook_from_x][y].type = PieceType::NONE;

            if (move->p.color == PieceColor::WHITE) white_kingside_rook_moved = true;
            else black_kingside_rook_moved = true;
        }
        else if (from_x == 4 && to_x == 2)
        { // Queenside
            uint8_t rook_from_x = 0;
            uint8_t rook_to_x   = 3;
            uint8_t y = from_y;

            m.captured_rook_from = rook_from_x;
            m.captured_rook_to   = rook_to_x;
            m.captured_rook_piece = board[rook_from_x][y];

            board[rook_to_x][y] = board[rook_from_x][y];
            board[rook_from_x][y].type = PieceType::NONE;

            if (move->p.color == PieceColor::WHITE) white_queenside_rook_moved = true;
            else black_queenside_rook_moved = true;
        }
    }
    else if (move->p.type == PieceType::PAWN)
    {
        if (move->p.color == PieceColor::WHITE)
        {
            if (to_y == 7)
            {
                board[to_x][to_y].type = PieceType::QUEEN;
                board[to_x][to_y].color = PieceColor::WHITE;
            }
        }
        else if (move->p.color == PieceColor::BLACK)
        {
            if (to_y == 0)
            {
                board[to_x][to_y].type = PieceType::QUEEN;
                board[to_x][to_y].color = PieceColor::BLACK;
            }
        }
    }

    history.push_back(m);

    // Switch turn
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

    // Undo move
    board[from_x][from_y] = m.p;
    board[to_x][to_y] = m.captured;

    // Undo rook move if castling
    if (m.captured_rook_piece.type != PieceType::NONE)
    {
        uint8_t y = (m.p.color == PieceColor::WHITE) ? 0 : 7;
        board[m.captured_rook_from][y] = m.captured_rook_piece;
        board[m.captured_rook_to][y].type = PieceType::NONE;
    }

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

    // 1. Generate normal 1-square moves
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

            ChessPiece target = board[nx][ny];
            if (target.type == PieceType::NONE || target.color != p.color)
            {
                moves.push_back({p, compact_coords(nx, ny), compact_coords(x, y)});
            }
        }
    }

    // 2. Generate castling moves (pseudo-legal, ignore check for now)
    if (p.color == PieceColor::WHITE && !white_king_moved)
    {
        // Kingside
        if (!white_kingside_rook_moved &&
            board[5][0].type == PieceType::NONE &&
            board[6][0].type == PieceType::NONE)
        {
            moves.push_back({p, compact_coords(6,0), compact_coords(x,y)});
        }

        // Queenside
        if (!white_queenside_rook_moved &&
            board[1][0].type == PieceType::NONE &&
            board[2][0].type == PieceType::NONE &&
            board[3][0].type == PieceType::NONE)
        {
            moves.push_back({p, compact_coords(2,0), compact_coords(x,y)});
        }
    }
    else if (p.color == PieceColor::BLACK && !black_king_moved)
    {
        // Kingside
        if (!black_kingside_rook_moved &&
            board[5][7].type == PieceType::NONE &&
            board[6][7].type == PieceType::NONE)
        {
            moves.push_back({p, compact_coords(6,7), compact_coords(x,y)});
        }

        // Queenside
        if (!black_queenside_rook_moved &&
            board[1][7].type == PieceType::NONE &&
            board[2][7].type == PieceType::NONE &&
            board[3][7].type == PieceType::NONE)
        {
            moves.push_back({p, compact_coords(2,7), compact_coords(x,y)});
        }
    }

    return moves;
}

// will_be_check now only tests the king's destination
bool ChessBoard::will_be_check(const Move* move)
{
    // Temporarily make the move
    uint8_t to_x   = move->to >> 4;
    uint8_t to_y   = move->to & 0x0F;
    uint8_t from_x = move->from >> 4;
    uint8_t from_y = move->from & 0x0F;

    ChessPiece captured = board[to_x][to_y];
    board[to_x][to_y] = move->p;
    board[from_x][from_y].type = PieceType::NONE;

    // Find the king's position after the move
    int kx = -1, ky = -1;
    if (move->p.type == PieceType::KING)
    {
        kx = to_x;
        ky = to_y;
    }
    else
    {
        for (int i = 0; i < 64; i++)
        {
            int x = i % 8;
            int y = i / 8;
            if (board[x][y].type == PieceType::KING &&
                board[x][y].color == move->p.color)
            {
                kx = x;
                ky = y;
                break;
            }
        }
    }

    PieceColor enemy = (move->p.color == PieceColor::WHITE) ? PieceColor::BLACK : PieceColor::WHITE;

    // Check if any enemy attacks the king
    bool in_check = false;
    for (int i = 0; i < 64; i++)
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
            case PieceType::KING:   moves = get_king_moves(x, y, p); break; // safe, pseudo-legal only
            default: break;
        }

        for (auto& m : moves)
        {
            if (m.to == compact_coords(kx, ky))
            {
                in_check = true;
                break;
            }
        }
        if (in_check) break;
    }

    // Undo move
    board[from_x][from_y] = move->p;
    board[to_x][to_y] = captured;

    return in_check;
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
