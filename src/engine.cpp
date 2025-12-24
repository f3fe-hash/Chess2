#include "engine.hpp"

static const float PAWN_VALUE   = 1.00;
static const float KNIGHT_VALUE = 2.93;
static const float BISHOP_VALUE = 3.00;
static const float ROOK_VALUE   = 4.56;
static const float QUEEN_VALUE  = 9.05;
static const float KING_VALUE = 1000.0f;

constexpr float MATE_SCORE = 100000.0f; // Score for checkmate

// Interesting move config
static const float INTERESTING_MOVE_THRESHHOLD = 1.0f; // If interesting score less than this, decrease depth
static const float CHECK_WEIGHT   = 2.0f;
static const float CAPTURE_WEIGHT = 0.5f;

static const int QUIESCENCE_MAX = 3;

static const float BOARD_SCALING = 10.00; // divide table values by this
static const int depth = 5;

// Pawn
static const int PAWN_TABLE[8][8] =
{
    { 0,   0,   0,   0,   0,   0,   0,   0 },
    { 10,  10,  10,  10,  10,  10,  10,  10 },
    { 5,   5,   10,  25,  25,  10,  5,   5 },
    { 0,   0,   0,   20,  20,  0,   0,   0 },
    { 5,  -5,  -10,  0,   0,  -10, -5,  5 },
    { 5,   10,  10, -20, -20,  10,  10,  5 },
    { 10,  10,  20, -20, -20,  20,  10,  10 },
    { 0,   0,   0,   0,   0,   0,   0,   0 }
};

// Knight
static const int KNIGHT_TABLE[8][8] =
{
    {-50, -40, -30, -30, -30, -30, -40, -50},
    {-40, -20, 0,   5,   5,   0,  -20, -40},
    {-30,  5,  10, 15,  15, 10,   5,  -30},
    {-30,  0,  15, 20,  20, 15,   0,  -30},
    {-30,  5,  15, 20,  20, 15,   5,  -30},
    {-30,  0,  10, 15,  15, 10,   0,  -30},
    {-40, -20, 0,   0,   0,   0,  -20, -40},
    {-50, -40, -30, -30, -30, -30, -40, -50}
};

// Bishop
static const int BISHOP_TABLE[8][8] =
{
    {-20, -10, -10, -10, -10, -10, -10, -20},
    {-10,   5,  0,   0,   0,   0,   5, -10},
    {-10,  10, 10,  10,  10,  10,  10, -10},
    {-10,   0, 10,  10,  10,  10,   0, -10},
    {-10,   5,  5,  10,  10,   5,   5, -10},
    {-10,   0,  5,  10,  10,   5,   0, -10},
    {-10,   0,  0,   0,   0,   0,   0, -10},
    {-20, -10, -10, -10, -10, -10, -10, -20}
};

// Rook
static const int ROOK_TABLE[8][8] =
{
    { 0,   0,   0,   5,   5,   0,   0,   0 },
    {-5,   0,   0,   0,   0,   0,   0,  -5 },
    {-5,   0,   0,   0,   0,   0,   0,  -5 },
    {-5,   0,   0,   0,   0,   0,   0,  -5 },
    {-5,   0,   0,   0,   0,   0,   0,  -5 },
    {-5,   0,   0,   0,   0,   0,   0,  -5 },
    { 5,  10,  10,  10,  10,  10,  10,   5 },
    { 0,   0,   0,   0,   0,   0,   0,   0 }
};

// Queen
static const int QUEEN_TABLE[8][8] =
{
    {-20, -10, -10, -5,  -5, -10, -10, -20},
    {-10,   0,   5,  0,   0,   0,   0, -10},
    {-10,   5,   5,  5,   5,   5,   0, -10},
    { 0,    0,   5,  5,   5,   5,   0,  -5},
    {-5,    0,   5,  5,   5,   5,   0,  -5},
    {-10,   0,   5,  5,   5,   5,   0, -10},
    {-10,   0,   0,   0,   0,   0,   0, -10},
    {-20, -10, -10, -5,  -5, -10, -10, -20}
};

// King (middle game)
static const int KING_TABLE[8][8] =
{
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-20, -30, -30, -40, -40, -30, -30, -20},
    {-10, -20, -20, -20, -20, -20, -20, -10},
    { 20,  20,   0,   0,   0,   0,  20,  20},
    { 20,  30,  10,   0,   0,  10,  30,  20}
};

ChessEngine::ChessEngine()
{}

ChessEngine::~ChessEngine()
{}

Move ChessEngine::make_move(const ChessBoard* board)
{
    const Move m = search(board, depth);
    return m;
}

float ChessEngine::eval(const ChessBoard* position)
{
    float score = 0.0f;

    for (int x = 0; x < 8; ++x)
    {
        for (int y = 0; y < 8; ++y)
        {
            const ChessPiece& p = position->board[x][y];
            if (p.type == PieceType::NONE)
                continue;

            // Flip tables for black
            int ty = (p.color == PieceColor::WHITE) ? y : 7 - y;

            float value = 0.00f;
            switch (p.type)
            {
                case PieceType::PAWN:
                    value = PAWN_VALUE;
                    value += PAWN_TABLE[x][ty] / BOARD_SCALING;
                    break;

                case PieceType::KNIGHT:
                    value = KNIGHT_VALUE;
                    value += KNIGHT_TABLE[x][ty] / BOARD_SCALING;
                    break;

                case PieceType::BISHOP:
                    value = BISHOP_VALUE;
                    value += BISHOP_TABLE[x][ty] / BOARD_SCALING;
                    break;

                case PieceType::ROOK:
                    value = ROOK_VALUE;
                    value += ROOK_TABLE[x][ty] / BOARD_SCALING;
                    break;
                    
                case PieceType::QUEEN:
                    value = QUEEN_VALUE;
                    value += QUEEN_TABLE[x][ty] / BOARD_SCALING;
                    break;

                case PieceType::KING:
                {
                    value = KING_VALUE;
                    value += KING_TABLE[x][ty] / BOARD_SCALING;

                    int shield = 0;
                    for (int dx = -1; dx <= 1; dx++)
                    for (int dy = -1; dy <= 1; dy++)
                    {
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx < 0 || nx >= 8 || ny < 0 || ny >= 8) continue;
                        if (position->board[nx][ny].type == PieceType::PAWN &&
                            position->board[nx][ny].color == p.color)
                            shield++;
                    }

                    value += shield * 0.1f;
                    break;
                }

                default:
                    break;
            }

            if (p.color == PieceColor::WHITE)
                score += value;
            else
                score -= value;
        }
    }

    return score;
}

float ChessEngine::quiescence(ChessBoard* board, float alpha, float beta, int depth)
{
    float stand_pat = eval(board);
    int turn_mul = (board->turn == PieceColor::WHITE) ? 1 : -1;
    stand_pat *= turn_mul;

    if (depth == 0)
        return stand_pat;

    if (stand_pat >= beta)
        return beta;
    if (stand_pat > alpha)
        alpha = stand_pat;

    PieceColor us = board->turn;

    for (auto& m : board->get_moves())
    {
        if (m.p.color != us)
            continue;

        // Only captures
        uint8_t tx = (m.to >> 4) & 0xF;
        uint8_t ty = m.to & 0xF;
        if (board->board[tx][ty].type == PieceType::NONE)
            continue;

        board->make_move(&m);
        if (board->is_check(us))
        {
            board->undo_move();
            continue;
        }

        float score = -quiescence(board, -beta, -alpha, depth - 1);
        board->undo_move();

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

Move ChessEngine::search(const ChessBoard* position, int depth)
{
    ChessBoard board = *position; // copy board
    Move best_move{};
    float best_score = -1e9f;

    auto moves = board.get_moves();
    if (moves.empty())
    {
        return Move{}; // or throw, or mark as resign
    }
    for (auto& m : moves)
    {
        if (m.p.color != position->turn)
            continue;

        PieceColor us = m.p.color;

        board.make_move(&m);
        if (board.is_check(us))
        {
            board.undo_move();
            continue;
        }

        float score = -negamax(
            &board,
            depth - 1,
            -1e9f,
            1e9f
        );

        board.undo_move();

        if (score > best_score)
        {
            best_score = score;
            best_move = m;
        }
    }

    return best_move;
}

static inline float is_interesting(
    ChessBoard* board,
    const Move& m)
{
    float interesting = 0.00f;
    uint8_t capture_x = (m.to >> 4) & 0x0f;
    uint8_t capture_y = m.to & 0x0f;
    ChessPiece captured = board->board[capture_x][capture_y];

    // Captures
    if (captured.type != PieceType::NONE)
        interesting += CAPTURE_WEIGHT;

    // Promotions
    //if (promotion != PieceType::NONE)
    //    return true;

    // Checks
    PieceColor them =
        (m.p.color == PieceColor::WHITE)
            ? PieceColor::BLACK
            : PieceColor::WHITE;

    board->make_move((Move*)&m);
    bool gives_check = board->is_check(them);
    board->undo_move();

    interesting += gives_check * CHECK_WEIGHT;
    return interesting;
}

static inline int move_score(ChessBoard* board, const Move& m)
{
    int score = 0;

    uint8_t tx = (m.to >> 4) & 0xF;
    uint8_t ty = m.to & 0xF;
    ChessPiece cap = board->board[tx][ty];

    // Captures first
    if (cap.type != PieceType::NONE)
        score += 1000 + int(cap.type) * 10;

    // Promotions
    if (m.p.type == PieceType::PAWN &&
        (ty == 0 || ty == 7))
        score += 800;

    return score;
}

float ChessEngine::negamax(
    ChessBoard* board,
    int depth,
    float alpha,
    float beta)
{
    const int turn_multiplier = (board->turn == PieceColor::WHITE) ? 1 : -1;
    if (depth == 0)
        return quiescence(board, alpha, beta, QUIESCENCE_MAX);

    PieceColor us = board->turn;
    auto moves = board->get_moves();

    bool has_legal = false;
    for (auto& m : moves)
    {
        if (m.p.color != us)
            continue;

        board->make_move(&m);
        if (!board->is_check(us))
            has_legal = true;
        board->undo_move();

        if (has_legal)
            break;
    }

    if (!has_legal)
    {
        if (board->is_check(us))
            return turn_multiplier * (MATE_SCORE + depth); // mate sooner is better
        else
            return 0.0f; // stalemate
    }

    float best = -1e9f;
    int move_index = 0;
    std::sort(moves.begin(), moves.end(),
        [&](const Move& a, const Move& b)
        {
            return move_score(board, a) > move_score(board, b);
        });
    for (auto& m : moves)
    {
        if (m.p.color != us)
            continue;

        board->make_move(&m);
        if (board->is_check(us))
        {
            board->undo_move();
            continue;
        }

        float interesting = is_interesting(board, m);

        int new_depth = depth - 1;

        // Late Move Reduction:
        if (interesting < INTERESTING_MOVE_THRESHHOLD && depth >= 3 && move_index >= 3)
        {
            new_depth -= 1; // reduce by 1 ply
        }

        float score = -negamax(
            board,
            new_depth,
            -beta,
            -alpha
        );

        board->undo_move();

        best = std::max(best, score);
        alpha = std::max(alpha, score);

        if (alpha >= beta)
            break;

        move_index++;
    }


    return best;
}
