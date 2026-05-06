#include <stdio.h>
#include <stdint.h>
#include "../../src/atlas.h"
#include "../../src/board.h"
#include "../../src/movegen.h"
#include "../../src/types.h"

/* ── Perft ────────────────────────────────────────────────────────────────── */
static uint64_t perft(Board *b, int depth)
{
    if (depth == 0) return 1ULL;

    Move moves[MAX_MOVES];
    int  n = movegen_legal(b, moves);

    if (depth == 1) return (uint64_t)n;

    uint64_t nodes = 0;
    Undo u;
    for (int i = 0; i < n; i++) {
        board_make(b, moves[i], &u);
        nodes += perft(b, depth - 1);
        board_undo(b, moves[i], &u);
    }
    return nodes;
}

typedef struct {
    const char *name;
    const char *fen;
    int         depth;
    uint64_t    expected;
} PerftCase;

static const PerftCase CASES[] = {
    /* Starting position */
    { "start d1", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 1, 20ULL },
    { "start d2", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 2, 400ULL },
    { "start d3", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 3, 8902ULL },
    { "start d4", "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 4, 197281ULL },

    /* Kiwipete */
    { "kiwi d1", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 1, 48ULL },
    { "kiwi d2", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 2, 2039ULL },
    { "kiwi d3", "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", 3, 97862ULL },

    /* Position 3 */
    { "pos3 d1", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 1, 14ULL },
    { "pos3 d2", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 2, 191ULL },
    { "pos3 d3", "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", 3, 2812ULL },
};

int main(void)
{
    atlas_init();

    int pass = 0, fail = 0;

    for (int i = 0; i < (int)(sizeof CASES / sizeof CASES[0]); i++) {
        const PerftCase *tc = &CASES[i];
        Board b;
        board_from_fen(&b, tc->fen);
        uint64_t got = perft(&b, tc->depth);
        if (got == tc->expected) {
            printf("OK  %-12s depth=%d  %llu\n", tc->name, tc->depth,
                   (unsigned long long)got);
            pass++;
        } else {
            printf("FAIL %-12s depth=%d  expected=%llu  got=%llu\n",
                   tc->name, tc->depth,
                   (unsigned long long)tc->expected,
                   (unsigned long long)got);
            fail++;
        }
    }

    printf("\n[perft] %d/%d passed\n", pass, pass + fail);
    return (fail == 0) ? 0 : 1;
}
