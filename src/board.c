#include "board.h"
#include "atlas.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* ── Piece characters ─────────────────────────────────────────────────────── */
static const char PIECE_CHARS[2][6] = {
    { 'P', 'N', 'B', 'R', 'Q', 'K' },   /* WHITE */
    { 'p', 'n', 'b', 'r', 'q', 'k' }    /* BLACK */
};

/* ── Castling rook squares ────────────────────────────────────────────────── *
 * ROOK_FROM[color][0=kingside, 1=queenside]
 * ROOK_TO  [color][0=kingside, 1=queenside]
 * ─────────────────────────────────────────────────────────────────────────── */
static const int ROOK_FROM[2][2] = { { H1, A1 }, { H8, A8 } };
static const int ROOK_TO[2][2]   = { { F1, D1 }, { F8, D8 } };

/* ── board_init ───────────────────────────────────────────────────────────── */
void board_init(Board *b)
{
    board_from_fen(b, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

/* ── board_from_fen ───────────────────────────────────────────────────────── */
void board_from_fen(Board *b, const char *fen)
{
    memset(b, 0, sizeof *b);
    b->ep = NO_SQ;

    /* ── Piece placement ──────────────────────────────────────────────────── */
    int sq = 56; /* start at a8 */
    const char *p = fen;
    while (*p && *p != ' ') {
        char c = *p++;
        if (c == '/') {
            sq -= 16; /* go down one rank */
        } else if (c >= '1' && c <= '8') {
            sq += (c - '0');
        } else {
            int color = (c >= 'a') ? BLACK : WHITE;
            int piece;
            switch (c | 0x20) { /* to lower */
                case 'p': piece = PAWN;   break;
                case 'n': piece = KNIGHT; break;
                case 'b': piece = BISHOP; break;
                case 'r': piece = ROOK;   break;
                case 'q': piece = QUEEN;  break;
                case 'k': piece = KING;   break;
                default:  piece = NONE;   break;
            }
            if (piece != NONE)
                b->pieces[color][piece] |= BB(sq);
            sq++;
        }
    }

    /* ── Side to move ─────────────────────────────────────────────────────── */
    if (*p) p++; /* skip space */
    b->side = (*p == 'b') ? BLACK : WHITE;
    if (*p) p++;

    /* ── Castling rights ──────────────────────────────────────────────────── */
    if (*p) p++; /* skip space */
    b->castle = 0;
    while (*p && *p != ' ') {
        switch (*p++) {
            case 'K': b->castle |= CR_WK; break;
            case 'Q': b->castle |= CR_WQ; break;
            case 'k': b->castle |= CR_BK; break;
            case 'q': b->castle |= CR_BQ; break;
        }
    }

    /* ── En-passant square ────────────────────────────────────────────────── */
    if (*p) p++; /* skip space */
    if (*p && *p != '-') {
        int ef = *p - 'a';
        p++;
        int er = *p - '1';
        p++;
        b->ep = SQ(ef, er);
    } else {
        b->ep = NO_SQ;
        if (*p) p++;
    }

    /* ── Halfmove clock ───────────────────────────────────────────────────── */
    if (*p) p++; /* skip space */
    b->halfmove = 0;
    if (*p && *p != ' ')
        b->halfmove = (int)strtol(p, (char **)&p, 10);

    /* ── Full move number ─────────────────────────────────────────────────── */
    if (*p) p++; /* skip space */
    b->fullmove = 1;
    if (*p)
        b->fullmove = (int)strtol(p, NULL, 10);

    /* ── Derived occupancy ────────────────────────────────────────────────── */
    b->occ[WHITE] = b->occ[BLACK] = 0;
    for (int pc = 0; pc < 6; pc++) {
        b->occ[WHITE] |= b->pieces[WHITE][pc];
        b->occ[BLACK] |= b->pieces[BLACK][pc];
    }
    b->all = b->occ[WHITE] | b->occ[BLACK];
}

/* ── board_to_fen ─────────────────────────────────────────────────────────── */
void board_to_fen(const Board *b, char *out)
{
    char *p = out;

    /* ── Piece placement ──────────────────────────────────────────────────── */
    for (int rank = 7; rank >= 0; rank--) {
        int empty = 0;
        for (int file = 0; file < 8; file++) {
            int sq = SQ(file, rank);
            int found = 0;
            for (int color = 0; color < 2 && !found; color++) {
                for (int pc = 0; pc < 6 && !found; pc++) {
                    if (b->pieces[color][pc] & BB(sq)) {
                        if (empty) { *p++ = (char)('0' + empty); empty = 0; }
                        *p++ = PIECE_CHARS[color][pc];
                        found = 1;
                    }
                }
            }
            if (!found) empty++;
        }
        if (empty) { *p++ = (char)('0' + empty); }
        if (rank > 0) *p++ = '/';
    }

    /* ── Side, castling, EP, clocks ──────────────────────────────────────── */
    *p++ = ' ';
    *p++ = (b->side == WHITE) ? 'w' : 'b';
    *p++ = ' ';

    if (b->castle) {
        if (b->castle & CR_WK) *p++ = 'K';
        if (b->castle & CR_WQ) *p++ = 'Q';
        if (b->castle & CR_BK) *p++ = 'k';
        if (b->castle & CR_BQ) *p++ = 'q';
    } else {
        *p++ = '-';
    }

    *p++ = ' ';
    if (b->ep != NO_SQ) {
        *p++ = (char)('a' + FILE(b->ep));
        *p++ = (char)('1' + RANK(b->ep));
    } else {
        *p++ = '-';
    }

    p += sprintf(p, " %d %d", b->halfmove, b->fullmove);
    *p = '\0';
}

/* ── board_piece_at ───────────────────────────────────────────────────────── */
int board_piece_at(const Board *b, int sq, int color)
{
    Bitboard mask = BB(sq);
    for (int pc = 0; pc < 6; pc++)
        if (b->pieces[color][pc] & mask)
            return pc;
    return NONE;
}

/* ── board_sq_attacked ────────────────────────────────────────────────────── */
bool board_sq_attacked(const Board *b, int sq, int by)
{
    /* Pawn attacks: if a pawn of 'by' would attack sq, check the opposite     */
    /* perspective: does a pawn of (by^1) on sq see a pawn of 'by'?           */
    if (ATLAS.pawn[by ^ 1][sq] & b->pieces[by][PAWN])   return true;
    if (ATLAS.knight[sq]       & b->pieces[by][KNIGHT])  return true;
    if (ATLAS.king[sq]         & b->pieces[by][KING])    return true;
    if (bishop_attacks(sq, b->all) & (b->pieces[by][BISHOP] | b->pieces[by][QUEEN]))
        return true;
    if (rook_attacks(sq, b->all) & (b->pieces[by][ROOK] | b->pieces[by][QUEEN]))
        return true;
    return false;
}

/* ── board_in_check ───────────────────────────────────────────────────────── */
bool board_in_check(const Board *b, int side)
{
    Bitboard king_bb = b->pieces[side][KING];
    if (!king_bb) return false;
    int ksq = LSB(king_bb);
    return board_sq_attacked(b, ksq, side ^ 1);
}

/* ── board_make ───────────────────────────────────────────────────────────── */
void board_make(Board *b, Move m, Undo *u)
{
    /* Save irreversible state */
    u->ep       = b->ep;
    u->castle   = b->castle;
    u->halfmove = b->halfmove;

    int from  = MV_FROM(m);
    int to    = MV_TO(m);
    int pc    = MV_PIECE(m);
    int cap   = MV_CAP(m);
    int promo = MV_PROMO(m);
    int us    = b->side;
    int them  = us ^ 1;

    /* ── Remove our piece from 'from' ─────────────────────────────────────── */
    b->pieces[us][pc] ^= BB(from);
    b->occ[us]        ^= BB(from);
    b->all            ^= BB(from);

    /* ── Handle capture ───────────────────────────────────────────────────── */
    if (cap != NONE) {
        int csq = MV_IS_EP(m)
                  ? (us == WHITE ? to - 8 : to + 8)
                  : to;
        b->pieces[them][cap] ^= BB(csq);
        b->occ[them]         ^= BB(csq);
        b->all               ^= BB(csq);
    }

    /* ── Place our piece at 'to' ──────────────────────────────────────────── */
    int placed = (promo != NONE) ? promo : pc;
    b->pieces[us][placed] ^= BB(to);
    b->occ[us]            ^= BB(to);
    b->all                ^= BB(to);

    /* ── Castling: move rook ──────────────────────────────────────────────── */
    if (MV_IS_CASTLE(m)) {
        int kside = (to > from) ? 0 : 1;
        int rf    = ROOK_FROM[us][kside];
        int rt    = ROOK_TO[us][kside];
        b->pieces[us][ROOK] ^= BB(rf) | BB(rt);
        b->occ[us]          ^= BB(rf) | BB(rt);
        b->all              ^= BB(rf) | BB(rt);
    }

    /* ── Update en-passant square ─────────────────────────────────────────── */
    b->ep = MV_IS_DBL(m) ? (from + to) / 2 : NO_SQ;

    /* ── Update castling rights ───────────────────────────────────────────── */
    static const int CASTLE_MASK[64] = {
        [A1] = ~CR_WQ, [E1] = ~(CR_WK | CR_WQ), [H1] = ~CR_WK,
        [A8] = ~CR_BQ, [E8] = ~(CR_BK | CR_BQ), [H8] = ~CR_BK
    };
    /* All squares not listed have mask 0xFF (no rights removed) */
    static int mask_init = 0;
    static int castle_masks[64];
    if (!mask_init) {
        for (int i = 0; i < 64; i++) castle_masks[i] = 0xF;
        castle_masks[A1] &= ~CR_WQ;
        castle_masks[H1] &= ~CR_WK;
        castle_masks[E1] &= ~(CR_WK | CR_WQ);
        castle_masks[A8] &= ~CR_BQ;
        castle_masks[H8] &= ~CR_BK;
        castle_masks[E8] &= ~(CR_BK | CR_BQ);
        mask_init = 1;
    }
    b->castle &= castle_masks[from] & castle_masks[to];
    (void)CASTLE_MASK; /* suppress unused warning from the static const */

    /* ── Halfmove clock ───────────────────────────────────────────────────── */
    if (pc == PAWN || cap != NONE)
        b->halfmove = 0;
    else
        b->halfmove++;

    /* ── Full move number ─────────────────────────────────────────────────── */
    if (us == BLACK)
        b->fullmove++;

    /* ── Flip side ────────────────────────────────────────────────────────── */
    b->side ^= 1;
}

/* ── board_undo ───────────────────────────────────────────────────────────── */
void board_undo(Board *b, Move m, const Undo *u)
{
    /* Flip side back first */
    b->side ^= 1;
    int us   = b->side;
    int them = us ^ 1;

    int from  = MV_FROM(m);
    int to    = MV_TO(m);
    int pc    = MV_PIECE(m);
    int cap   = MV_CAP(m);
    int promo = MV_PROMO(m);

    /* ── Restore fullmove ─────────────────────────────────────────────────── */
    if (us == BLACK)
        b->fullmove--;

    /* ── Remove placed piece from 'to' ───────────────────────────────────── */
    int placed = (promo != NONE) ? promo : pc;
    b->pieces[us][placed] ^= BB(to);
    b->occ[us]            ^= BB(to);
    b->all                ^= BB(to);

    /* ── Put original piece back at 'from' ───────────────────────────────── */
    b->pieces[us][pc] ^= BB(from);
    b->occ[us]        ^= BB(from);
    b->all            ^= BB(from);

    /* ── Restore captured piece ───────────────────────────────────────────── */
    if (cap != NONE) {
        int csq = MV_IS_EP(m)
                  ? (us == WHITE ? to - 8 : to + 8)
                  : to;
        b->pieces[them][cap] ^= BB(csq);
        b->occ[them]         ^= BB(csq);
        b->all               ^= BB(csq);
    }

    /* ── Restore rook if castling ─────────────────────────────────────────── */
    if (MV_IS_CASTLE(m)) {
        int kside = (to > from) ? 0 : 1;
        int rf    = ROOK_FROM[us][kside];
        int rt    = ROOK_TO[us][kside];
        b->pieces[us][ROOK] ^= BB(rf) | BB(rt);
        b->occ[us]          ^= BB(rf) | BB(rt);
        b->all              ^= BB(rf) | BB(rt);
    }

    /* ── Restore irreversible state ───────────────────────────────────────── */
    b->ep       = u->ep;
    b->castle   = u->castle;
    b->halfmove = u->halfmove;
}

/* ── board_print ──────────────────────────────────────────────────────────── */
void board_print(const Board *b)
{
    printf("\n  +---+---+---+---+---+---+---+---+\n");
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d |", rank + 1);
        for (int file = 0; file < 8; file++) {
            int sq = SQ(file, rank);
            char c = '.';
            for (int color = 0; color < 2; color++) {
                for (int pc = 0; pc < 6; pc++) {
                    if (b->pieces[color][pc] & BB(sq))
                        c = PIECE_CHARS[color][pc];
                }
            }
            printf(" %c |", c);
        }
        printf("\n  +---+---+---+---+---+---+---+---+\n");
    }
    printf("    a   b   c   d   e   f   g   h\n");
    printf("\nSide: %s  EP: ", b->side == WHITE ? "white" : "black");
    if (b->ep != NO_SQ)
        printf("%c%d", 'a' + FILE(b->ep), 1 + RANK(b->ep));
    else
        printf("-");
    printf("  Castle: %c%c%c%c  HM: %d  FM: %d\n",
           (b->castle & CR_WK) ? 'K' : '-',
           (b->castle & CR_WQ) ? 'Q' : '-',
           (b->castle & CR_BK) ? 'k' : '-',
           (b->castle & CR_BQ) ? 'q' : '-',
           b->halfmove, b->fullmove);
}
