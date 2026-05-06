#include "eval.h"
#include "eval/pawn.h"
#include "eval/king_attack.h"
#include "atlas.h"
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

/* ── Mobility ─────────────────────────────────────────────────────────────── */
#define MOBILITY_WEIGHT 2

static int mobility_count(const Board *b, int color)
{
    Bitboard free = ~b->occ[color];
    Bitboard occ  = b->all;
    int      n    = 0;
    Bitboard bb;

    bb = b->pieces[color][KNIGHT];
    while (bb) { int sq = LSB(bb); POPLSB(bb);
                 n += POPCNT(ATLAS.knight[sq] & free); }

    bb = b->pieces[color][BISHOP];
    while (bb) { int sq = LSB(bb); POPLSB(bb);
                 n += POPCNT(bishop_attacks(sq, occ) & free); }

    bb = b->pieces[color][ROOK];
    while (bb) { int sq = LSB(bb); POPLSB(bb);
                 n += POPCNT(rook_attacks(sq, occ) & free); }

    bb = b->pieces[color][QUEEN];
    while (bb) { int sq = LSB(bb); POPLSB(bb);
                 n += POPCNT(queen_attacks(sq, occ) & free); }

    return n;
}

/* ── Feature functions ────────────────────────────────────────────────────── */
static int _zero      (const Board *b) { (void)b; return 0; }

static int _mobility  (const Board *b) {
    return MOBILITY_WEIGHT * (mobility_count(b, WHITE) - mobility_count(b, BLACK));
}
static int _pawn_struct(const Board *b) {
    return PAWN_STRUCTURE[pawn_file_mask(b->pieces[WHITE][PAWN])]
         - PAWN_STRUCTURE[pawn_file_mask(b->pieces[BLACK][PAWN])];
}
static int _clusters  (const Board *b) {
    return eval_pawn_clusters(b->pieces[WHITE][PAWN], WHITE)
         - eval_pawn_clusters(b->pieces[BLACK][PAWN], BLACK);
}
static int _king_attack(const Board *b) {
    return eval_king_attack(b, WHITE) - eval_king_attack(b, BLACK);
}

/* ── Feature pointers (swapped via eval_set_feature, no if in hot path) ───── */
static int (*fp_mobility)   (const Board *) = _mobility;
static int (*fp_pawn_struct)(const Board *) = _pawn_struct;
static int (*fp_clusters)   (const Board *) = _clusters;
static int (*fp_king_attack)(const Board *) = _king_attack;

void eval_set_feature(EvalFeature feat, int on)
{
    switch (feat) {
        case EVAL_FEAT_MOBILITY:
            fp_mobility    = on ? _mobility    : _zero; break;
        case EVAL_FEAT_PAWN_STRUCT:
            fp_pawn_struct = on ? _pawn_struct : _zero; break;
        case EVAL_FEAT_CLUSTERS:
            fp_clusters    = on ? _clusters    : _zero; break;
        case EVAL_FEAT_KING_ATTACK:
            fp_king_attack = on ? _king_attack : _zero; break;
    }
}

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
                int pst_idx = (color == WHITE)
                              ? (FILE(sq) + (7 - RANK(sq)) * 8)
                              : (sq ^ 56);
                score[color] += PST[pc][pst_idx];
            }
        }
    }

    int s = (score[WHITE] - score[BLACK])
          + fp_mobility(b)
          + fp_pawn_struct(b)
          + fp_clusters(b)
          + fp_king_attack(b);
    return (b->side == WHITE) ? s : -s;
}
