#include "engine.hpp"

static const float PAWN_VALUE   = 1.00;
static const float KNIGHT_VALUE = 2.93;
static const float BISHOP_VALUE = 3.00;
static const float ROOK_VALUE   = 4.56;
static const float QUEEN_VALUE  = 9.05;

static const int depth = 5;

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

            float value = 0.0f;
            switch (p.type)
            {
                case PieceType::PAWN:   value = PAWN_VALUE; break;
                case PieceType::KNIGHT: value = KNIGHT_VALUE; break;
                case PieceType::BISHOP: value = BISHOP_VALUE; break;
                case PieceType::ROOK:   value = ROOK_VALUE; break;
                case PieceType::QUEEN:  value = QUEEN_VALUE; break;
                case PieceType::KING:   value = 0.0f; break;
                default: break;
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
