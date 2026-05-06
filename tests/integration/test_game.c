#include "../common.h"
#include "../../src/atlas.h"
#include "../../src/board.h"
#include "../../src/movegen.h"
#include "../../src/types.h"

/* Helper: apply a UCI string move */
static int apply_uci(Board *b, const char *uci)
{
    int from_f = uci[0] - 'a';
    int from_r = uci[1] - '1';
    int to_f   = uci[2] - 'a';
    int to_r   = uci[3] - '1';
    int from   = SQ(from_f, from_r);
    int to     = SQ(to_f, to_r);

    Move moves[MAX_MOVES];
    int  n = movegen_legal(b, moves);
    for (int i = 0; i < n; i++) {
        if (MV_FROM(moves[i]) == from && MV_TO(moves[i]) == to) {
            Undo u;
            board_make(b, moves[i], &u);
            return 1;
        }
    }
    return 0; /* move not found */
}

int main(void)
{
    atlas_init();

    /* ── Scholar's mate sequence ──────────────────────────────────────────── *
     * 1.e4 e5 2.Bc4 Nc6 3.Qh5 Nf6?? 4.Qxf7#
     * After Qxf7#, black is in checkmate: in check and 0 legal moves.
     * ─────────────────────────────────────────────────────────────────────── */
    Board b;
    board_init(&b);

    ASSERT(apply_uci(&b, "e2e4"), "1.e4 applied");
    ASSERT(apply_uci(&b, "e7e5"), "1...e5 applied");
    ASSERT(apply_uci(&b, "f1c4"), "2.Bc4 applied");
    ASSERT(apply_uci(&b, "b8c6"), "2...Nc6 applied");
    ASSERT(apply_uci(&b, "d1h5"), "3.Qh5 applied");
    ASSERT(apply_uci(&b, "g8f6"), "3...Nf6 applied");
    ASSERT(apply_uci(&b, "h5f7"), "4.Qxf7# applied");

    /* Black king is in check */
    ASSERT(board_in_check(&b, BLACK), "black king in check after Qxf7#");

    /* Black has 0 legal moves → checkmate */
    Move legal[MAX_MOVES];
    int nlegal = movegen_legal(&b, legal);
    ASSERT(nlegal == 0, "black has 0 legal moves after Qxf7# (checkmate)");

    /* ── Fifty-move rule counter ───────────────────────────────────────────── */
    board_init(&b);
    int hm0 = b.halfmove; /* should be 0 */
    ASSERT(hm0 == 0, "halfmove starts at 0");

    /* Knight move increments halfmove clock */
    apply_uci(&b, "g1f3");
    ASSERT(b.halfmove == 1, "halfmove=1 after Nf3");

    /* Pawn move resets it */
    apply_uci(&b, "e7e5");
    ASSERT(b.halfmove == 0, "halfmove=0 after pawn move e7e5");

    /* ── Illegal move not in legal list ───────────────────────────────────── *
     * In the starting position e1 cannot move to e2 (own pawn there).       */
    board_init(&b);
    Move moves[MAX_MOVES];
    int  n = movegen_legal(&b, moves);
    int  found_e1e2 = 0;
    for (int i = 0; i < n; i++)
        if (MV_FROM(moves[i]) == E1 && MV_TO(moves[i]) == E2)
            found_e1e2 = 1;
    ASSERT(!found_e1e2, "e1e2 (king into own pawn) not in legal list");

    /* ── Fool's mate ──────────────────────────────────────────────────────── */
    board_init(&b);
    apply_uci(&b, "f2f3");
    apply_uci(&b, "e7e5");
    apply_uci(&b, "g2g4");
    apply_uci(&b, "d8h4"); /* Qh4# */
    ASSERT(board_in_check(&b, WHITE), "white in check after Qh4#");
    nlegal = movegen_legal(&b, legal);
    ASSERT(nlegal == 0, "white has 0 legal moves after Qh4# (checkmate)");

    TEST_SUMMARY();
}
