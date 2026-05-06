#include "eval.h"
#include "types.h"

/* ── Material values (centipawns) ─────────────────────────────────────────── */
const int MATERIAL[6] = { 100, 320, 330, 500, 900, 0 };

/* ── Piece-Square Tables (Michniewski Simplified Evaluation) ──────────────── *
 *  Tables are written rank-8 first (a8=index 0 .. h1=index 63).
 *  To look up a WHITE piece on square sq:
 *      pst_index = FILE(sq) + (7 - RANK(sq)) * 8
 *  For BLACK, mirror by XOR-ing sq with 56 (flip rank), then use same index:
 *      pst_index = FILE(sq ^ 56) + (7 - RANK(sq ^ 56)) * 8  == sq ^ 56
 *  ─────────────────────────────────────────────────────────────────────────── */
static const int PST_PAWN[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,  /* rank 8 */
    50, 50, 50, 50, 50, 50, 50, 50,  /* rank 7 */
    10, 10, 20, 30, 30, 20, 10, 10,  /* rank 6 */
     5,  5, 10, 25, 25, 10,  5,  5,  /* rank 5 */
     0,  0,  0, 20, 20,  0,  0,  0,  /* rank 4 */
     5, -5,-10,  0,  0,-10, -5,  5,  /* rank 3 */
     5, 10, 10,-20,-20, 10, 10,  5,  /* rank 2 */
     0,  0,  0,  0,  0,  0,  0,  0   /* rank 1 */
};

static const int PST_KNIGHT[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50
};

static const int PST_BISHOP[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20
};

static const int PST_ROOK[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  5,  5,  0,  0,  0
};

static const int PST_QUEEN[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

static const int PST_KING_MG[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

static const int * const PST[6] = {
    PST_PAWN, PST_KNIGHT, PST_BISHOP,
    PST_ROOK, PST_QUEEN,  PST_KING_MG
};

/* ── eval ─────────────────────────────────────────────────────────────────── */
int eval(const Board *b)
{
    int score[2] = {0, 0};

    for (int color = 0; color < 2; color++) {
        for (int pc = 0; pc < 6; pc++) {
            Bitboard bb = b->pieces[color][pc];
            while (bb) {
                int sq = LSB(bb);
                POPLSB(bb);

                score[color] += MATERIAL[pc];

                /* PST index: for WHITE flip rank; for BLACK use sq directly  */
                /* (sq ^ 56 maps a1→a8 perspective, matching the table above) */
                int pst_idx = (color == WHITE)
                              ? (FILE(sq) + (7 - RANK(sq)) * 8)
                              : (sq ^ 56);
                score[color] += PST[pc][pst_idx];
            }
        }
    }

    int s = score[WHITE] - score[BLACK];
    return (b->side == WHITE) ? s : -s;
}
