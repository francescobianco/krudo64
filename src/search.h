#pragma once
#include "board.h"
#include "types.h"

typedef struct {
    int depth;       /* max depth (0 = use time)     */
    int movetime;    /* ms per move (0 = use depth)  */
    int wtime;
    int btime;
    int winc;
    int binc;
    int movestogo;
    volatile int stop; /* set to 1 to stop search */
} SearchInfo;

typedef struct {
    Move best_move;
    int  score;
    long nodes;
} SearchResult;

void search_run(Board *b, SearchInfo *info, SearchResult *result);
