#include "engine.hpp"

static const float PAWN_VALUE   = 1.00;
static const float KNIGHT_VALUE = 2.93;
static const float BISHOP_VALUE = 3.00;
static const float ROOK_VALUE   = 4.56;
static const float QUEEN_VALUE  = 9.05;

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
            int ty = (position->turn == PieceColor::WHITE) ? y : 7 - y;

            float value = 0.0f;
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
                    value = KING_TABLE[x][ty] / BOARD_SCALING;
                    break;

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

        board.make_move(&m);

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

float ChessEngine::negamax(
    ChessBoard* board,
    int depth,
    float alpha,
    float beta)
{
    if (depth == 0)
        return eval(board);

    if (board->is_checkmate())
        return -100000.0f; // losing position

    float best = -1e9f;

    auto moves = board->get_moves();
    for (auto& m : moves)
    {
        if (m.p.color != board->turn)
            continue;

        board->make_move(&m);

        float score = -negamax(
            board,
            depth - 1,
            -beta,
            -alpha
        );

        board->undo_move();

        best = std::max(best, score);
        alpha = std::max(alpha, score);

        if (alpha >= beta)
            break; // beta cutoff
    }

    return best;
}
