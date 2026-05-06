#pragma once
#include <stdint.h>
#include <stdbool.h>

/* ── Basic types ──────────────────────────────────────────────────────────── */
typedef uint64_t Bitboard;
typedef uint32_t Move;

/* ── Colors ───────────────────────────────────────────────────────────────── */
#define WHITE 0
#define BLACK 1

/* ── Piece types ──────────────────────────────────────────────────────────── */
#define PAWN   0
#define KNIGHT 1
#define BISHOP 2
#define ROOK   3
#define QUEEN  4
#define KING   5
#define NONE   6

/* ── Squares (A1=0 .. H8=63) ─────────────────────────────────────────────── */
#define A1  0
#define B1  1
#define C1  2
#define D1  3
#define E1  4
#define F1  5
#define G1  6
#define H1  7
#define A2  8
#define B2  9
#define C2  10
#define D2  11
#define E2  12
#define F2  13
#define G2  14
#define H2  15
#define A3  16
#define B3  17
#define C3  18
#define D3  19
#define E3  20
#define F3  21
#define G3  22
#define H3  23
#define A4  24
#define B4  25
#define C4  26
#define D4  27
#define E4  28
#define F4  29
#define G4  30
#define H4  31
#define A5  32
#define B5  33
#define C5  34
#define D5  35
#define E5  36
#define F5  37
#define G5  38
#define H5  39
#define A6  40
#define B6  41
#define C6  42
#define D6  43
#define E6  44
#define F6  45
#define G6  46
#define H6  47
#define A7  48
#define B7  49
#define C7  50
#define D7  51
#define E7  52
#define F7  53
#define G7  54
#define H7  55
#define A8  56
#define B8  57
#define C8  58
#define D8  59
#define E8  60
#define F8  61
#define G8  62
#define H8  63
#define NO_SQ 64

/* ── Castling rights ──────────────────────────────────────────────────────── */
#define CR_WK 1
#define CR_WQ 2
#define CR_BK 4
#define CR_BQ 8

/* ── Move encoding (32-bit) ───────────────────────────────────────────────── *
 *  bits  0- 5  from square
 *  bits  6-11  to square
 *  bits 12-14  piece (0-5)
 *  bits 15-17  captured piece (0-5, NONE=6)
 *  bits 18-20  promotion piece (NONE=6)
 *  bit  21     en-passant flag
 *  bit  22     castling flag
 *  bit  23     double-push flag
 * ─────────────────────────────────────────────────────────────────────────── */
#define MV_FROM(m)       ((int)((m) & 0x3F))
#define MV_TO(m)         ((int)(((m) >> 6) & 0x3F))
#define MV_PIECE(m)      ((int)(((m) >> 12) & 0x7))
#define MV_CAP(m)        ((int)(((m) >> 15) & 0x7))
#define MV_PROMO(m)      ((int)(((m) >> 18) & 0x7))
#define MV_IS_EP(m)      ((int)(((m) >> 21) & 0x1))
#define MV_IS_CASTLE(m)  ((int)(((m) >> 22) & 0x1))
#define MV_IS_DBL(m)     ((int)(((m) >> 23) & 0x1))

#define MK_MOVE(from, to, pc, cap, promo, ep, castle, dbl) \
    ((Move)((from) | ((to) << 6) | ((pc) << 12) | ((cap) << 15) | \
            ((promo) << 18) | ((ep) << 21) | ((castle) << 22) | ((dbl) << 23)))

/* ── Bitboard helpers ─────────────────────────────────────────────────────── */
#define BB(sq)       (1ULL << (sq))
#define FILE(sq)     ((sq) & 7)
#define RANK(sq)     ((sq) >> 3)
#define SQ(f, r)     ((r) * 8 + (f))

#define LSB(bb)      (__builtin_ctzll(bb))
#define MSB(bb)      (63 - __builtin_clzll(bb))
#define POPCNT(bb)   (__builtin_popcountll(bb))
#define POPLSB(bb)   ((bb) &= (bb) - 1)

/* ── File masks ───────────────────────────────────────────────────────────── */
#define FILE_A  0x0101010101010101ULL
#define FILE_B  0x0202020202020202ULL
#define FILE_G  0x4040404040404040ULL
#define FILE_H  0x8080808080808080ULL

/* ── Rank masks ───────────────────────────────────────────────────────────── */
#define RANK_1  0x00000000000000FFULL
#define RANK_2  0x000000000000FF00ULL
#define RANK_3  0x0000000000FF0000ULL
#define RANK_6  0x0000FF0000000000ULL
#define RANK_7  0x00FF000000000000ULL
#define RANK_8  0xFF00000000000000ULL

/* ── Search constants ─────────────────────────────────────────────────────── */
#define MAX_MOVES 256
#define MAX_PLY   128
#define MATE      30000
