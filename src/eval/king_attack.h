#pragma once
#include "../board.h"
#include "../atlas.h"

/* ── King-attack scoring weights ─────────────────────────────────────────── *
 * X-ray alignment: piece shares file/rank (R/Q) or diagonal (B/Q) with the  *
 * enemy king, regardless of interposed pieces.                               *
 * Zone attack: piece actually attacks a square in the 3×3 king area.        *
 * ─────────────────────────────────────────────────────────────────────────── */
#define XRAY_ROOK_WT    10
#define XRAY_BISHOP_WT   8
#define XRAY_QUEEN_WT   15
#define KING_ZONE_WT     3

static inline int eval_king_attack(const Board *b, int color)
{
    int      enemy = 1 - color;
    int      ksq   = LSB(b->pieces[enemy][KING]);
    int      kf    = FILE(ksq);
    int      kr    = RANK(ksq);
    Bitboard zone  = ATLAS.king[ksq] | BB(ksq);  /* 3×3 area around enemy king */
    Bitboard occ   = b->all;
    int      score = 0;
    Bitboard bb;

    /* Rooks — file or rank alignment */
    bb = b->pieces[color][ROOK];
    while (bb) {
        int sq = LSB(bb); POPLSB(bb);
        if (FILE(sq) == kf || RANK(sq) == kr)
            score += XRAY_ROOK_WT;
        score += POPCNT(rook_attacks(sq, occ) & zone) * KING_ZONE_WT;
    }

    /* Bishops — diagonal alignment (df == dr: main diag; df == -dr: anti-diag) */
    bb = b->pieces[color][BISHOP];
    while (bb) {
        int sq = LSB(bb); POPLSB(bb);
        int df = FILE(sq) - kf;
        int dr = RANK(sq) - kr;
        if (df == dr || df == -dr)
            score += XRAY_BISHOP_WT;
        score += POPCNT(bishop_attacks(sq, occ) & zone) * KING_ZONE_WT;
    }

    /* Queens — file, rank, or diagonal */
    bb = b->pieces[color][QUEEN];
    while (bb) {
        int sq = LSB(bb); POPLSB(bb);
        int df = FILE(sq) - kf;
        int dr = RANK(sq) - kr;
        if (FILE(sq) == kf || RANK(sq) == kr || df == dr || df == -dr)
            score += XRAY_QUEEN_WT;
        score += POPCNT(queen_attacks(sq, occ) & zone) * KING_ZONE_WT;
    }

    return score;
}
