#include "../common.h"
#include "../../src/atlas.h"
#include "../../src/board.h"
#include "../../src/movegen.h"
#include "../../src/types.h"

/* Helper: check if a UCI move string is in the move list */
static int has_move(const Move *moves, int n, int from, int to)
{
    for (int i = 0; i < n; i++)
        if (MV_FROM(moves[i]) == from && MV_TO(moves[i]) == to)
            return 1;
    return 0;
}

int main(void)
{
    atlas_init();

    /* ── Starting position: exactly 20 legal moves ───────────────────────── */
    Board b;
    board_init(&b);
    Move moves[MAX_MOVES];
    int n = movegen_legal(&b, moves);
    ASSERT(n == 20, "start: exactly 20 legal moves");

    /* ── After 1.e4: black has 20 moves ──────────────────────────────────── */
    board_from_fen(&b, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    n = movegen_legal(&b, moves);
    ASSERT(n == 20, "after e4: black has 20 legal moves");
    /* e5 and d5 pushes should be available */
    ASSERT(has_move(moves, n, E7, E5), "black e7e5 available");
    ASSERT(has_move(moves, n, D7, D5), "black d7d5 available");

    /* ── King in check: only moves resolving check ────────────────────────── */
    /* Fool's mate: white queen on h4, rook on h8 threatening... simpler: */
    /* fen with white king in check from a rook */
    board_from_fen(&b, "4k3/8/8/8/8/8/8/r3K3 w - - 0 1");
    n = movegen_legal(&b, moves);
    /* King on e1, rook on a1 attacks rank 1 — king must move off rank 1 */
    for (int i = 0; i < n; i++) {
        /* Verify no move leaves king in check */
        Board tmp = b;
        Undo u;
        board_make(&tmp, moves[i], &u);
        ASSERT(!board_in_check(&tmp, WHITE), "check-resolution move is legal");
    }
    ASSERT(n > 0 && n < 20, "in check: limited moves available");

    /* ── En passant capture generated ───────────────────────────────────────  */
    /* white pawn on e5, black just played d7d5, ep=d6 */
    board_from_fen(&b, "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2");
    n = movegen_legal(&b, moves);
    ASSERT(has_move(moves, n, E5, D6), "en passant e5xd6 generated");

    /* ── Castling: starting position has no castling (pieces blocking) ───── */
    board_init(&b);
    n = movegen_legal(&b, moves);
    int castle_count = 0;
    for (int i = 0; i < n; i++)
        if (MV_IS_CASTLE(moves[i])) castle_count++;
    ASSERT(castle_count == 0, "start: no castling available (blocked)");

    /* ── Castling: open position ─────────────────────────────────────────── */
    board_from_fen(&b, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    n = movegen_legal(&b, moves);
    castle_count = 0;
    for (int i = 0; i < n; i++)
        if (MV_IS_CASTLE(moves[i])) castle_count++;
    ASSERT(castle_count == 2, "open position: 2 castling moves available for white");

    /* ── Promotion moves ─────────────────────────────────────────────────── */
    board_from_fen(&b, "8/P7/8/8/8/8/8/4K1k1 w - - 0 1");
    n = movegen_legal(&b, moves);
    int promo_count = 0;
    for (int i = 0; i < n; i++)
        if (MV_PROMO(moves[i]) != NONE) promo_count++;
    ASSERT(promo_count == 4, "a7 pawn has 4 promotion moves");

    TEST_SUMMARY();
}
