#pragma once
#include "types.h"
#include <stdbool.h>

/* ── Board state ──────────────────────────────────────────────────────────── */
typedef struct {
    Bitboard pieces[2][6]; /* [color][piece]              */
    Bitboard occ[2];       /* [color] all pieces of color */
    Bitboard all;          /* all pieces                  */
    int      side;         /* side to move                */
    int      ep;           /* en-passant target sq        */
    int      castle;       /* castling rights bitmask     */
    int      halfmove;     /* fifty-move clock            */
    int      fullmove;     /* full move number            */
} Board;

/* Irreversible state saved before board_make and restored by board_undo */
typedef struct {
    int ep;
    int castle;
    int halfmove;
} Undo;

/* ── API ──────────────────────────────────────────────────────────────────── */
void board_init(Board *b);
void board_from_fen(Board *b, const char *fen);
void board_to_fen(const Board *b, char *out);
void board_make(Board *b, Move m, Undo *u);
void board_undo(Board *b, Move m, const Undo *u);
bool board_in_check(const Board *b, int side);
bool board_sq_attacked(const Board *b, int sq, int by);
int  board_piece_at(const Board *b, int sq, int color);
void board_print(const Board *b);
