#include "../common.h"
#include "../../src/atlas.h"
#include "../../src/board.h"
#include "../../src/movegen.h"
#include "../../src/search.h"
#include "../../src/uci.h"
#include "../../src/types.h"
#include <string.h>
#include <stdio.h>

int main(void)
{
    atlas_init();

    SearchInfo info;
    SearchResult result;

    /* ── Tactic 1: Scholar's mate threat — Qxf7# ─────────────────────────── *
     * FEN: r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq -
     * Best move should be Qxf7# (h5f7)
     * ─────────────────────────────────────────────────────────────────────── */
    Board b;
    board_from_fen(&b, "r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq -");

    memset(&info, 0, sizeof info);
    memset(&result, 0, sizeof result);
    info.depth = 3;

    search_run(&b, &info, &result);

    int from  = MV_FROM(result.best_move);
    int to    = MV_TO(result.best_move);
    printf("Tactic 1: engine plays %s (expected h5f7)\n", move_to_uci(result.best_move));
    ASSERT(from == H5 && to == F7, "tactic 1: Qxf7# (h5f7)");

    /* ── Tactic 2: back-rank mate — Re8# ─────────────────────────────────── *
     * FEN: 7k/pp4pp/8/8/8/8/PP4PP/4R1K1 w - -
     * King on h8 trapped by g7/h7 pawns. Best move: Re8# (e1e8)
     * ─────────────────────────────────────────────────────────────────────── */
    board_from_fen(&b, "7k/pp4pp/8/8/8/8/PP4PP/4R1K1 w - -");

    memset(&info, 0, sizeof info);
    memset(&result, 0, sizeof result);
    info.depth = 3;

    search_run(&b, &info, &result);

    from = MV_FROM(result.best_move);
    to   = MV_TO(result.best_move);
    printf("Tactic 2: engine plays %s (expected e1e8)\n", move_to_uci(result.best_move));
    ASSERT(from == E1 && to == E8, "tactic 2: Re8# (e1e8)");

    TEST_SUMMARY();
}
