#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "../../src/atlas.h"
#include "../../src/board.h"
#include "../../src/movegen.h"
#include "../../src/uci.h"

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

int main(int argc, char *argv[])
{
    int depth = (argc >= 2) ? atoi(argv[1]) : 5;
    if (depth < 1) depth = 1;

    const char *fen = (argc >= 3)
        ? argv[2]
        : "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    atlas_init();

    Board b;
    board_from_fen(&b, fen);
    board_print(&b);
    printf("depth %d\n\n", depth);

    Move moves[MAX_MOVES];
    int  n = movegen_legal(&b, moves);

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);

    uint64_t total = 0;
    Undo u;
    for (int i = 0; i < n; i++) {
        board_make(&b, moves[i], &u);
        uint64_t sub = perft(&b, depth - 1);
        board_undo(&b, moves[i], &u);
        printf("%-6s %llu\n", move_to_uci(moves[i]), (unsigned long long)sub);
        total += sub;
    }

    clock_gettime(CLOCK_MONOTONIC, &t1);
    long ms = (t1.tv_sec - t0.tv_sec) * 1000L
            + (t1.tv_nsec - t0.tv_nsec) / 1000000L;

    printf("\nnodes  %llu\ntime   %ld ms\n",
           (unsigned long long)total, ms);
    if (ms > 0)
        printf("nps    %llu\n",
               (unsigned long long)(total * 1000ULL / (uint64_t)ms));

    return 0;
}
