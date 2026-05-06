#pragma once
#include "types.h"

/* ── Direction indices ────────────────────────────────────────────────────── */
#define DIR_N  0
#define DIR_NE 1
#define DIR_E  2
#define DIR_SE 3
#define DIR_S  4
#define DIR_SW 5
#define DIR_W  6
#define DIR_NW 7

/* ── Movement Atlas ───────────────────────────────────────────────────────── *
 *  All attack maps pre-computed at startup and stored here.
 *  Sliding piece attacks are derived from the ray table at query time.
 * ─────────────────────────────────────────────────────────────────────────── */
typedef struct {
    Bitboard pawn[2][64];     /* pawn attacks [color][sq]              */
    Bitboard knight[64];      /* knight jump targets                   */
    Bitboard king[64];        /* king one-step moves                   */
    Bitboard ray[64][8];      /* full directional rays from each sq    */
    Bitboard bishop[64];      /* bishop empty-board attack rays        */
    Bitboard rook[64];        /* rook empty-board attack rays          */
    Bitboard queen[64];       /* queen empty-board attack rays         */
    Bitboard between[64][64]; /* squares strictly between two squares  */
} Atlas;

extern Atlas ATLAS;

void     atlas_init(void);
Bitboard bishop_attacks(int sq, Bitboard occ);
Bitboard rook_attacks(int sq, Bitboard occ);
Bitboard queen_attacks(int sq, Bitboard occ);
