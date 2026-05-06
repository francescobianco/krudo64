#include "movegen.h"
#include "atlas.h"
#include <string.h>

/* ── Move builder helpers ─────────────────────────────────────────────────── */
static inline int add_move(Move *moves, int n,
                            int from, int to, int pc, int cap, int promo,
                            int ep, int castle, int dbl)
{
    moves[n] = MK_MOVE(from, to, pc, cap, promo, ep, castle, dbl);
    return n + 1;
}

/* Expand a to-square bitboard into individual moves (no promo, no flags) */
static inline int emit_quiet(Move *moves, int n, int from, int pc,
                              Bitboard targets)
{
    while (targets) {
        int to = LSB(targets);
        POPLSB(targets);
        n = add_move(moves, n, from, to, pc, NONE, NONE, 0, 0, 0);
    }
    return n;
}

static inline int emit_captures(Move *moves, int n, int from, int pc,
                                 Bitboard targets, const Board *b, int them)
{
    while (targets) {
        int to  = LSB(targets);
        POPLSB(targets);
        int cap = board_piece_at(b, to, them);
        n = add_move(moves, n, from, to, pc, cap, NONE, 0, 0, 0);
    }
    return n;
}

/* Add the 4 promotion flavours for a pawn move */
static inline int emit_promos(Move *moves, int n, int from, int to,
                               int cap, int ep)
{
    for (int promo = QUEEN; promo >= KNIGHT; promo--)
        n = add_move(moves, n, from, to, PAWN, cap, promo, ep, 0, 0);
    return n;
}

/* ── movegen_pseudo ───────────────────────────────────────────────────────── */
int movegen_pseudo(const Board *b, Move *moves)
{
    int  n    = 0;
    int  us   = b->side;
    int  them = us ^ 1;

    Bitboard us_occ   = b->occ[us];
    Bitboard them_occ = b->occ[them];
    Bitboard all      = b->all;
    Bitboard empty    = ~all;

    /* ── Pawns ────────────────────────────────────────────────────────────── */
    Bitboard pawns = b->pieces[us][PAWN];
    while (pawns) {
        int from = LSB(pawns);
        POPLSB(pawns);
        Bitboard bb = BB(from);

        if (us == WHITE) {
            /* Single push */
            Bitboard push1 = (bb << 8) & empty;
            if (push1) {
                int to = from + 8;
                if (RANK(to) == 7)
                    n = emit_promos(moves, n, from, to, NONE, 0);
                else
                    n = add_move(moves, n, from, to, PAWN, NONE, NONE, 0, 0, 0);

                /* Double push only if single push was clear */
                if (RANK(from) == 1) {
                    Bitboard push2 = (push1 << 8) & empty;
                    if (push2)
                        n = add_move(moves, n, from, from + 16, PAWN, NONE, NONE, 0, 0, 1);
                }
            }
            /* Captures */
            Bitboard atk = ATLAS.pawn[WHITE][from] & them_occ;
            while (atk) {
                int to = LSB(atk); POPLSB(atk);
                int cap = board_piece_at(b, to, them);
                if (RANK(to) == 7)
                    n = emit_promos(moves, n, from, to, cap, 0);
                else
                    n = add_move(moves, n, from, to, PAWN, cap, NONE, 0, 0, 0);
            }
            /* En passant */
            if (b->ep != NO_SQ && (ATLAS.pawn[WHITE][from] & BB(b->ep)))
                n = add_move(moves, n, from, b->ep, PAWN, PAWN, NONE, 1, 0, 0);
        } else { /* BLACK */
            /* Single push */
            Bitboard push1 = (bb >> 8) & empty;
            if (push1) {
                int to = from - 8;
                if (RANK(to) == 0)
                    n = emit_promos(moves, n, from, to, NONE, 0);
                else
                    n = add_move(moves, n, from, to, PAWN, NONE, NONE, 0, 0, 0);

                if (RANK(from) == 6) {
                    Bitboard push2 = (push1 >> 8) & empty;
                    if (push2)
                        n = add_move(moves, n, from, from - 16, PAWN, NONE, NONE, 0, 0, 1);
                }
            }
            /* Captures */
            Bitboard atk = ATLAS.pawn[BLACK][from] & them_occ;
            while (atk) {
                int to = LSB(atk); POPLSB(atk);
                int cap = board_piece_at(b, to, them);
                if (RANK(to) == 0)
                    n = emit_promos(moves, n, from, to, cap, 0);
                else
                    n = add_move(moves, n, from, to, PAWN, cap, NONE, 0, 0, 0);
            }
            /* En passant */
            if (b->ep != NO_SQ && (ATLAS.pawn[BLACK][from] & BB(b->ep)))
                n = add_move(moves, n, from, b->ep, PAWN, PAWN, NONE, 1, 0, 0);
        }
    }

    /* ── Knights ──────────────────────────────────────────────────────────── */
    Bitboard knights = b->pieces[us][KNIGHT];
    while (knights) {
        int from = LSB(knights); POPLSB(knights);
        Bitboard atk = ATLAS.knight[from];
        n = emit_captures(moves, n, from, KNIGHT, atk & them_occ, b, them);
        n = emit_quiet   (moves, n, from, KNIGHT, atk & empty);
    }

    /* ── Bishops ──────────────────────────────────────────────────────────── */
    Bitboard bishops = b->pieces[us][BISHOP];
    while (bishops) {
        int from = LSB(bishops); POPLSB(bishops);
        Bitboard atk = bishop_attacks(from, all);
        n = emit_captures(moves, n, from, BISHOP, atk & them_occ, b, them);
        n = emit_quiet   (moves, n, from, BISHOP, atk & empty);
    }

    /* ── Rooks ────────────────────────────────────────────────────────────── */
    Bitboard rooks = b->pieces[us][ROOK];
    while (rooks) {
        int from = LSB(rooks); POPLSB(rooks);
        Bitboard atk = rook_attacks(from, all);
        n = emit_captures(moves, n, from, ROOK, atk & them_occ, b, them);
        n = emit_quiet   (moves, n, from, ROOK, atk & empty);
    }

    /* ── Queens ───────────────────────────────────────────────────────────── */
    Bitboard queens = b->pieces[us][QUEEN];
    while (queens) {
        int from = LSB(queens); POPLSB(queens);
        Bitboard atk = queen_attacks(from, all);
        n = emit_captures(moves, n, from, QUEEN, atk & them_occ, b, them);
        n = emit_quiet   (moves, n, from, QUEEN, atk & empty);
    }

    /* ── King ─────────────────────────────────────────────────────────────── */
    {
        Bitboard king = b->pieces[us][KING];
        if (king) {
            int from = LSB(king);
            Bitboard atk = ATLAS.king[from] & ~us_occ;
            n = emit_captures(moves, n, from, KING, atk & them_occ, b, them);
            n = emit_quiet   (moves, n, from, KING, atk & empty);
        }
    }

    /* ── Castling ─────────────────────────────────────────────────────────── */
    if (us == WHITE) {
        if ((b->castle & CR_WK) &&
            !(all & (BB(F1) | BB(G1))) &&
            !board_sq_attacked(b, E1, BLACK) &&
            !board_sq_attacked(b, F1, BLACK) &&
            !board_sq_attacked(b, G1, BLACK))
        {
            n = add_move(moves, n, E1, G1, KING, NONE, NONE, 0, 1, 0);
        }
        if ((b->castle & CR_WQ) &&
            !(all & (BB(B1) | BB(C1) | BB(D1))) &&
            !board_sq_attacked(b, E1, BLACK) &&
            !board_sq_attacked(b, D1, BLACK) &&
            !board_sq_attacked(b, C1, BLACK))
        {
            n = add_move(moves, n, E1, C1, KING, NONE, NONE, 0, 1, 0);
        }
    } else {
        if ((b->castle & CR_BK) &&
            !(all & (BB(F8) | BB(G8))) &&
            !board_sq_attacked(b, E8, WHITE) &&
            !board_sq_attacked(b, F8, WHITE) &&
            !board_sq_attacked(b, G8, WHITE))
        {
            n = add_move(moves, n, E8, G8, KING, NONE, NONE, 0, 1, 0);
        }
        if ((b->castle & CR_BQ) &&
            !(all & (BB(B8) | BB(C8) | BB(D8))) &&
            !board_sq_attacked(b, E8, WHITE) &&
            !board_sq_attacked(b, D8, WHITE) &&
            !board_sq_attacked(b, C8, WHITE))
        {
            n = add_move(moves, n, E8, C8, KING, NONE, NONE, 0, 1, 0);
        }
    }

    return n;
}

/* ── movegen_legal ────────────────────────────────────────────────────────── */
int movegen_legal(const Board *b, Move *moves)
{
    Move pseudo[MAX_MOVES];
    int  np = movegen_pseudo(b, pseudo);

    Board  tmp;
    Undo   u;
    int    n = 0;
    int    us = b->side;

    for (int i = 0; i < np; i++) {
        tmp = *b;
        board_make(&tmp, pseudo[i], &u);
        if (!board_in_check(&tmp, us))
            moves[n++] = pseudo[i];
        /* No need to board_undo since we copied the board */
    }
    return n;
}

/* ── movegen_captures ─────────────────────────────────────────────────────── */
int movegen_captures(const Board *b, Move *moves)
{
    Move pseudo[MAX_MOVES];
    int  np = movegen_pseudo(b, pseudo);

    Board  tmp;
    Undo   u;
    int    n   = 0;
    int    us  = b->side;

    for (int i = 0; i < np; i++) {
        Move m = pseudo[i];
        /* Keep captures and promotions only */
        if (MV_CAP(m) == NONE && MV_PROMO(m) == NONE)
            continue;
        tmp = *b;
        board_make(&tmp, m, &u);
        if (!board_in_check(&tmp, us))
            moves[n++] = m;
    }
    return n;
}
