#include "atlas.h"
#include <string.h>

Atlas ATLAS;

/* Direction deltas: file-delta and rank-delta for each of the 8 directions.
 * Order: N, NE, E, SE, S, SW, W, NW */
static const int DF[8] = { 0,  1, 1,  1,  0, -1, -1, -1 };
static const int DR[8] = { 1,  1, 0, -1, -1, -1,  0,  1 };

/* Directions 0,1,2,7 go towards higher square indices (positive).
 * Directions 3,4,5,6 go towards lower square indices (negative).
 * For positive directions we want the LSB (nearest) blocker.
 * For negative directions we want the MSB (nearest) blocker.   */
static const int POSITIVE_DIR[8] = { 1, 1, 1, 0, 0, 0, 0, 1 };

/* ── Ray attack with blockers ─────────────────────────────────────────────── */
static Bitboard ray_attack(int sq, int dir, Bitboard occ)
{
    Bitboard full     = ATLAS.ray[sq][dir];
    Bitboard blockers = full & occ;
    if (!blockers)
        return full;

    int blocker_sq;
    if (POSITIVE_DIR[dir])
        blocker_sq = LSB(blockers);
    else
        blocker_sq = MSB(blockers);

    /* Return squares up to and including the blocker */
    return full ^ ATLAS.ray[blocker_sq][dir];
}

/* ── Public sliding attack queries ───────────────────────────────────────── */
Bitboard bishop_attacks(int sq, Bitboard occ)
{
    return ray_attack(sq, DIR_NE, occ)
         | ray_attack(sq, DIR_SE, occ)
         | ray_attack(sq, DIR_SW, occ)
         | ray_attack(sq, DIR_NW, occ);
}

Bitboard rook_attacks(int sq, Bitboard occ)
{
    return ray_attack(sq, DIR_N, occ)
         | ray_attack(sq, DIR_E, occ)
         | ray_attack(sq, DIR_S, occ)
         | ray_attack(sq, DIR_W, occ);
}

Bitboard queen_attacks(int sq, Bitboard occ)
{
    return bishop_attacks(sq, occ) | rook_attacks(sq, occ);
}

/* ── atlas_init ───────────────────────────────────────────────────────────── */
void atlas_init(void)
{
    memset(&ATLAS, 0, sizeof ATLAS);

    for (int sq = 0; sq < 64; sq++) {
        int f = FILE(sq);
        int r = RANK(sq);
        Bitboard bb = BB(sq);

        /* ── Pawn attacks ────────────────────────────────────────────────── */
        /* WHITE pawns attack diagonally upward */
        if (f > 0) ATLAS.pawn[WHITE][sq] |= BB(sq + 7);   /* NW */
        if (f < 7) ATLAS.pawn[WHITE][sq] |= BB(sq + 9);   /* NE */
        /* BLACK pawns attack diagonally downward */
        if (f < 7) ATLAS.pawn[BLACK][sq] |= BB(sq - 7);   /* SE */
        if (f > 0) ATLAS.pawn[BLACK][sq] |= BB(sq - 9);   /* SW */

        /* ── Knight attacks ──────────────────────────────────────────────── */
        if (r < 7 && f > 1) ATLAS.knight[sq] |= BB(sq + 6);
        if (r < 7 && f < 6) ATLAS.knight[sq] |= BB(sq + 10);
        if (r < 6 && f > 0) ATLAS.knight[sq] |= BB(sq + 15);
        if (r < 6 && f < 7) ATLAS.knight[sq] |= BB(sq + 17);
        if (r > 0 && f > 1) ATLAS.knight[sq] |= BB(sq - 10);
        if (r > 0 && f < 6) ATLAS.knight[sq] |= BB(sq - 6);
        if (r > 1 && f > 0) ATLAS.knight[sq] |= BB(sq - 17);
        if (r > 1 && f < 7) ATLAS.knight[sq] |= BB(sq - 15);

        /* ── King attacks ────────────────────────────────────────────────── */
        if (r < 7)           ATLAS.king[sq] |= BB(sq + 8);   /* N  */
        if (r < 7 && f < 7) ATLAS.king[sq] |= BB(sq + 9);   /* NE */
        if (f < 7)           ATLAS.king[sq] |= BB(sq + 1);   /* E  */
        if (r > 0 && f < 7) ATLAS.king[sq] |= BB(sq - 7);   /* SE */
        if (r > 0)           ATLAS.king[sq] |= BB(sq - 8);   /* S  */
        if (r > 0 && f > 0) ATLAS.king[sq] |= BB(sq - 9);   /* SW */
        if (f > 0)           ATLAS.king[sq] |= BB(sq - 1);   /* W  */
        if (r < 7 && f > 0) ATLAS.king[sq] |= BB(sq + 7);   /* NW */

        /* ── Directional rays ────────────────────────────────────────────── */
        for (int d = 0; d < 8; d++) {
            int cf = f + DF[d];
            int cr = r + DR[d];
            while (cf >= 0 && cf < 8 && cr >= 0 && cr < 8) {
                ATLAS.ray[sq][d] |= BB(SQ(cf, cr));
                cf += DF[d];
                cr += DR[d];
            }
        }

        /* Bishop / rook / queen combined rays */
        ATLAS.bishop[sq] = ATLAS.ray[sq][DIR_NE] | ATLAS.ray[sq][DIR_SE]
                         | ATLAS.ray[sq][DIR_SW] | ATLAS.ray[sq][DIR_NW];
        ATLAS.rook[sq]   = ATLAS.ray[sq][DIR_N]  | ATLAS.ray[sq][DIR_E]
                         | ATLAS.ray[sq][DIR_S]  | ATLAS.ray[sq][DIR_W];
        ATLAS.queen[sq]  = ATLAS.bishop[sq] | ATLAS.rook[sq];

        (void)bb; /* suppress unused-variable warning */
    }

    /* ── Between table ───────────────────────────────────────────────────── */
    for (int a = 0; a < 64; a++) {
        for (int b = 0; b < 64; b++) {
            if (a == b) continue;
            for (int d = 0; d < 8; d++) {
                if (ATLAS.ray[a][d] & BB(b)) {
                    /* b is on ray[a][d]: squares strictly between = */
                    /* ray[a][d] & ~ray[b][d] & ~BB(b)              */
                    ATLAS.between[a][b] =
                        ATLAS.ray[a][d] & ~ATLAS.ray[b][d] & ~BB(b);
                    break;
                }
            }
        }
    }
}
