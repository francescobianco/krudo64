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

static inline int emit_quiet(Move *moves, int n, int from, int pc, Bitboard tgt)
{
    while (tgt) {
        int to = LSB(tgt); POPLSB(tgt);
        n = add_move(moves, n, from, to, pc, NONE, NONE, 0, 0, 0);
    }
    return n;
}

static inline int emit_captures(Move *moves, int n, int from, int pc,
                                 Bitboard tgt, const Board *b, int them)
{
    while (tgt) {
        int to  = LSB(tgt); POPLSB(tgt);
        int cap = board_piece_at(b, to, them);
        n = add_move(moves, n, from, to, pc, cap, NONE, 0, 0, 0);
    }
    return n;
}

static inline int emit_promos(Move *moves, int n, int from, int to,
                               int cap, int ep)
{
    for (int promo = QUEEN; promo >= KNIGHT; promo--)
        n = add_move(moves, n, from, to, PAWN, cap, promo, ep, 0, 0);
    return n;
}

/* ── Color-indexed pawn tables ───────────────────────────────────────────── *
 * These turn every color branch in pawn generation into a table lookup.     */
static const int      PUSH_DELTA[2]  = {+8, -8};
static const Bitboard START_MASK[2]  = {RANK_2, RANK_7};
static const Bitboard PROMO_MASK[2]  = {RANK_8, RANK_1};

/* ── Color-indexed castling tables ──────────────────────────────────────── */
static const int CASTLE_RIGHT[2][2] = { {CR_WK, CR_WQ}, {CR_BK, CR_BQ} };

/* King from/to per [color][kingside=0 / queenside=1] */
static const int CASTLE_KING[2][2][2] = {
    { {E1, G1}, {E1, C1} },
    { {E8, G8}, {E8, C8} },
};

/* Gap squares that must be empty */
static const Bitboard CASTLE_GAP[2][2] = {
    { BB(F1)|BB(G1),           BB(B1)|BB(C1)|BB(D1) },
    { BB(F8)|BB(G8),           BB(B8)|BB(C8)|BB(D8) },
};

/* Three squares that must not be attacked */
static const int CASTLE_SAFE[2][2][3] = {
    { {E1, F1, G1}, {E1, D1, C1} },
    { {E8, F8, G8}, {E8, D8, C8} },
};

/* ── Per-piece attack function table ────────────────────────────────────── *
 * ATK_FN[piece](sq, occ) → attack bitboard.                                *
 * Index 0 (PAWN) is NULL — pawns are handled separately by gen_pawn.       *
 * This table removes all piece-type dispatch from the hot movegen loop.    */
typedef Bitboard (*AtkFn)(int, Bitboard);
static Bitboard _kn(int sq, Bitboard o) { (void)o; return ATLAS.knight[sq]; }
static Bitboard _bi(int sq, Bitboard o) { return bishop_attacks(sq, o); }
static Bitboard _ro(int sq, Bitboard o) { return rook_attacks  (sq, o); }
static Bitboard _qu(int sq, Bitboard o) { return queen_attacks (sq, o); }
static Bitboard _ki(int sq, Bitboard o) { (void)o; return ATLAS.king[sq]; }

static const AtkFn ATK_FN[6] = { NULL, _kn, _bi, _ro, _qu, _ki };

/* ── Pawn move generator (color-agnostic) ───────────────────────────────── *
 * PUSH_DELTA/START_MASK/PROMO_MASK replace every if(us==WHITE) branch.     *
 * The only remaining ifs are: push-is-possible and promo-vs-quiet split.   */
static int gen_pawn(Move *moves, int n, const Board *b,
                    int us, int them, int from,
                    Bitboard them_occ, Bitboard empty)
{
    int      delta = PUSH_DELTA[us];
    int      to1   = from + delta;
    Bitboard dst1  = BB(to1) & empty;           /* single push, if clear    */

    if (dst1) {
        if (dst1 & PROMO_MASK[us]) {            /* AND: on promotion rank   */
            n = emit_promos(moves, n, from, to1, NONE, 0);
        } else {
            n = add_move(moves, n, from, to1, PAWN, NONE, NONE, 0, 0, 0);
            /* Double push gated by start-rank AND clear second square. */
            Bitboard on_start = BB(from) & START_MASK[us];
            Bitboard dst2     = BB(to1 + delta) & empty;
            /* Convert on_start to an all-ones mask (0 or ~0) to avoid if. */
            Bitboard gate     = (Bitboard)(-(int64_t)(on_start != 0));
            dst2             &= gate;
            if (dst2)
                n = add_move(moves, n, from, to1 + delta, PAWN, NONE, NONE, 0, 0, 1);
        }
    }

    /* Captures: atlas already encodes color — no color check here. */
    Bitboard atk = ATLAS.pawn[us][from] & them_occ;
    while (atk) {
        int to  = LSB(atk); POPLSB(atk);
        int cap = board_piece_at(b, to, them);
        if (BB(to) & PROMO_MASK[us])
            n = emit_promos(moves, n, from, to, cap, 0);
        else
            n = add_move(moves, n, from, to, PAWN, cap, NONE, 0, 0, 0);
    }

    /* En passant: atlas check is an AND operation, no color branch. */
    if (b->ep != NO_SQ && (ATLAS.pawn[us][from] & BB(b->ep)))
        n = add_move(moves, n, from, b->ep, PAWN, PAWN, NONE, 1, 0, 0);

    return n;
}

/* ── movegen_pseudo ─────────────────────────────────────────────────────── *
 * Core idea: iterate pl[us] (already color-filtered), look up each piece   *
 * in ATK_FN (no piece-type if in loop), emit targets from AND-chains.      *
 * The only remaining dispatch is PAWN vs rest (different move shape).      */
int movegen_pseudo(const Board *b, Move *moves)
{
    int      n        = 0;
    int      us       = b->side;
    int      them     = us ^ 1;
    Bitboard them_occ = b->occ[them];
    Bitboard all      = b->all;
    Bitboard empty    = ~all;

    for (int i = 0; i < b->pl_n[us]; i++) {
        int from  = PL_SQ(b->pl[us][i]);
        int piece = PL_PC(b->pl[us][i]);

        if (piece == PAWN) {
            n = gen_pawn(moves, n, b, us, them, from, them_occ, empty);
        } else {
            /* ATK_FN[piece] returns all squares the piece attacks.          *
             * AND with ~us_occ is split into captures (& them_occ) and     *
             * quiets (& empty) — two AND operations, no explicit if.       */
            Bitboard atk = ATK_FN[piece](from, all);
            n = emit_captures(moves, n, from, piece, atk & them_occ, b, them);
            n = emit_quiet   (moves, n, from, piece, atk & empty);
        }
    }

    /* ── Castling ─────────────────────────────────────────────────────────── *
     * Fully table-driven: CASTLE_RIGHT, CASTLE_GAP, CASTLE_SAFE indexed by  *
     * [us][side]. The loop eliminates the if(us==WHITE)/else block.          */
    for (int side = 0; side < 2; side++) {
        /* AND-chain: rights bit set AND gap empty AND no square attacked.   */
        if (!(b->castle & CASTLE_RIGHT[us][side])) continue;
        if (all & CASTLE_GAP[us][side])            continue;
        int safe = 1;
        for (int s = 0; s < 3 && safe; s++)
            safe &= !board_sq_attacked(b, CASTLE_SAFE[us][side][s], them);
        if (safe)
            n = add_move(moves, n,
                         CASTLE_KING[us][side][0],
                         CASTLE_KING[us][side][1],
                         KING, NONE, NONE, 0, 1, 0);
    }

    return n;
}

/* ── movegen_legal ────────────────────────────────────────────────────────── */
int movegen_legal(const Board *b, Move *moves)
{
    Move pseudo[MAX_MOVES];
    int  np = movegen_pseudo(b, pseudo);
    Board tmp;
    Undo  u;
    int   n  = 0;
    int   us = b->side;
    for (int i = 0; i < np; i++) {
        tmp = *b;
        board_make(&tmp, pseudo[i], &u);
        if (!board_in_check(&tmp, us))
            moves[n++] = pseudo[i];
    }
    return n;
}

/* ── movegen_captures ─────────────────────────────────────────────────────── */
int movegen_captures(const Board *b, Move *moves)
{
    Move pseudo[MAX_MOVES];
    int  np = movegen_pseudo(b, pseudo);
    Board tmp;
    Undo  u;
    int   n  = 0;
    int   us = b->side;
    for (int i = 0; i < np; i++) {
        Move m = pseudo[i];
        if (MV_CAP(m) == NONE && MV_PROMO(m) == NONE)
            continue;
        tmp = *b;
        board_make(&tmp, m, &u);
        if (!board_in_check(&tmp, us))
            moves[n++] = m;
    }
    return n;
}
