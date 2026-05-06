#define _POSIX_C_SOURCE 200809L
#include "search.h"
#include "movegen.h"
#include "eval.h"
#include "uci.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

/* ── Timing ───────────────────────────────────────────────────────────────── */
static long now_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long)(ts.tv_sec * 1000L + ts.tv_nsec / 1000000L);
}

/* ── MVV-LVA move scoring ─────────────────────────────────────────────────── */
static const int MV_VAL[7] = { 100, 320, 330, 500, 900, 0, 0 };

static int move_score(Move m)
{
    int cap   = MV_CAP(m);
    int promo = MV_PROMO(m);
    int pc    = MV_PIECE(m);

    if (promo != NONE)
        return 20000 + MV_VAL[promo];
    if (cap != NONE)
        return 10000 + MV_VAL[cap] - MV_VAL[pc] / 10;
    return 0;
}

/* Simple insertion sort */
static void sort_moves(Move *moves, int n)
{
    for (int i = 1; i < n; i++) {
        Move key  = moves[i];
        int  kscore = move_score(key);
        int  j    = i - 1;
        while (j >= 0 && move_score(moves[j]) < kscore) {
            moves[j + 1] = moves[j];
            j--;
        }
        moves[j + 1] = key;
    }
}

/* ── Quiescence search ────────────────────────────────────────────────────── */
static int quiesce(Board *b, int alpha, int beta,
                   SearchInfo *info, SearchResult *result)
{
    result->nodes++;

    int stand_pat = eval(b);
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;

    Move caps[MAX_MOVES];
    int  ncaps = movegen_captures(b, caps);
    sort_moves(caps, ncaps);

    for (int i = 0; i < ncaps; i++) {
        if (info->stop) break;

        Undo u;
        board_make(b, caps[i], &u);
        int score = -quiesce(b, -beta, -alpha, info, result);
        board_undo(b, caps[i], &u);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }
    return alpha;
}

/* ── Negamax alpha-beta ───────────────────────────────────────────────────── */
static int negamax(Board *b, int depth, int alpha, int beta, int ply,
                   SearchInfo *info, SearchResult *result)
{
    if (info->stop) return 0;

    result->nodes++;

    if (depth == 0)
        return quiesce(b, alpha, beta, info, result);

    Move moves[MAX_MOVES];
    int  nm = movegen_legal(b, moves);

    if (nm == 0) {
        /* No legal moves: checkmate or stalemate */
        return board_in_check(b, b->side)
               ? (-MATE + ply)
               : 0;
    }

    sort_moves(moves, nm);

    Move best = 0;
    for (int i = 0; i < nm; i++) {
        if (info->stop) break;

        Undo u;
        board_make(b, moves[i], &u);
        int score = -negamax(b, depth - 1, -beta, -alpha, ply + 1, info, result);
        board_undo(b, moves[i], &u);

        if (score >= beta) return beta;
        if (score > alpha) {
            alpha = score;
            best  = moves[i];
            if (ply == 0)
                result->best_move = best;
        }
    }
    return alpha;
}

/* ── search_run ───────────────────────────────────────────────────────────── */
void search_run(Board *b, SearchInfo *info, SearchResult *result)
{
    result->best_move = 0;
    result->nodes     = 0;
    result->score     = 0;

    /* ── Time management ──────────────────────────────────────────────────── */
    long start_ms    = now_ms();
    long time_limit  = 0;

    if (info->movetime > 0) {
        time_limit = info->movetime;
    } else if (b->side == WHITE && info->wtime > 0) {
        int moves_left = (info->movestogo > 0) ? info->movestogo : 20;
        time_limit = (info->wtime / moves_left) + info->winc;
        if (time_limit > info->wtime - 50)
            time_limit = info->wtime - 50;
    } else if (b->side == BLACK && info->btime > 0) {
        int moves_left = (info->movestogo > 0) ? info->movestogo : 20;
        time_limit = (info->btime / moves_left) + info->binc;
        if (time_limit > info->btime - 50)
            time_limit = info->btime - 50;
    }

    int max_depth = (info->depth > 0) ? info->depth : MAX_PLY;

    for (int depth = 1; depth <= max_depth; depth++) {
        SearchResult cur = { 0, 0, 0 };
        cur.nodes = 0;

        int score = negamax(b, depth, -MATE, MATE, 0, info, &cur);

        if (info->stop) break;

        /* Accept this iteration's result */
        if (cur.best_move)
            result->best_move = cur.best_move;
        result->score  = score;
        result->nodes += cur.nodes;

        long elapsed = now_ms() - start_ms;
        long nps     = elapsed > 0 ? (cur.nodes * 1000L / elapsed) : 0;

        printf("info depth %d score cp %d nodes %ld nps %ld time %ld pv %s\n",
               depth, score, result->nodes, nps, elapsed,
               move_to_uci(result->best_move));
        fflush(stdout);

        /* Check time after each depth */
        if (time_limit > 0 && elapsed >= time_limit)
            break;
    }
}
