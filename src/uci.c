#include "uci.h"
#include "eval.h"
#include "movegen.h"
#include "search.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ── move_to_uci ──────────────────────────────────────────────────────────── */
char *move_to_uci(Move m)
{
    static char buf[6];
    if (!m) {
        buf[0] = '0'; buf[1] = '0'; buf[2] = '0'; buf[3] = '0'; buf[4] = '\0';
        return buf;
    }
    int from  = MV_FROM(m);
    int to    = MV_TO(m);
    int promo = MV_PROMO(m);

    buf[0] = (char)('a' + FILE(from));
    buf[1] = (char)('1' + RANK(from));
    buf[2] = (char)('a' + FILE(to));
    buf[3] = (char)('1' + RANK(to));

    if (promo != NONE) {
        const char *pchars = "pnbrqk";
        buf[4] = pchars[promo];
        buf[5] = '\0';
    } else {
        buf[4] = '\0';
    }
    return buf;
}

/* ── uci_to_move ──────────────────────────────────────────────────────────── */
Move uci_to_move(const Board *b, const char *str)
{
    if (!str || strlen(str) < 4) return 0;

    int from_f = str[0] - 'a';
    int from_r = str[1] - '1';
    int to_f   = str[2] - 'a';
    int to_r   = str[3] - '1';

    if (from_f < 0 || from_f > 7 || from_r < 0 || from_r > 7 ||
        to_f   < 0 || to_f   > 7 || to_r   < 0 || to_r   > 7)
        return 0;

    int from   = SQ(from_f, from_r);
    int to     = SQ(to_f, to_r);
    int promo  = NONE;

    if (str[4] && str[4] != ' ' && str[4] != '\n') {
        switch (tolower((unsigned char)str[4])) {
            case 'n': promo = KNIGHT; break;
            case 'b': promo = BISHOP; break;
            case 'r': promo = ROOK;   break;
            case 'q': promo = QUEEN;  break;
        }
    }

    Move moves[MAX_MOVES];
    int  nm = movegen_legal(b, moves);

    for (int i = 0; i < nm; i++) {
        if (MV_FROM(moves[i]) == from &&
            MV_TO(moves[i])   == to   &&
            (promo == NONE || MV_PROMO(moves[i]) == promo))
            return moves[i];
    }
    return 0;
}

/* ── uci_loop ─────────────────────────────────────────────────────────────── */
void uci_loop(void)
{
    Board      board;
    SearchInfo info;
    char       line[4096];

    board_init(&board);
    memset(&info, 0, sizeof info);

    while (fgets(line, sizeof line, stdin)) {
        /* Strip newline */
        char *nl = strchr(line, '\n');
        if (nl) *nl = '\0';
        char *cr = strchr(line, '\r');
        if (cr) *cr = '\0';

        if (strcmp(line, "uci") == 0) {
            printf("id name krudo64\n");
            printf("id author Francesco Bianco\n");
            printf("option name UseMobility type check default true\n");
            printf("option name UsePawnStructure type check default true\n");
            printf("option name UseClusters type check default true\n");
            printf("uciok\n");
            fflush(stdout);

        } else if (strcmp(line, "isready") == 0) {
            printf("readyok\n");
            fflush(stdout);

        } else if (strncmp(line, "setoption", 9) == 0) {
            const char *np = strstr(line, " name ");
            const char *vp = strstr(line, " value ");
            if (np && vp) {
                np += 6;  vp += 7;
                int on = (strncmp(vp, "true", 4) == 0) ? 1 : 0;
                if      (strncmp(np, "UseMobility",     11) == 0) eval_set_feature(EVAL_FEAT_MOBILITY,    on);
                else if (strncmp(np, "UsePawnStructure", 16) == 0) eval_set_feature(EVAL_FEAT_PAWN_STRUCT, on);
                else if (strncmp(np, "UseClusters",     11) == 0) eval_set_feature(EVAL_FEAT_CLUSTERS,    on);
            }

        } else if (strcmp(line, "ucinewgame") == 0) {
            board_init(&board);
            memset(&info, 0, sizeof info);

        } else if (strncmp(line, "position", 8) == 0) {
            const char *ptr = line + 8;
            while (*ptr == ' ') ptr++;

            if (strncmp(ptr, "startpos", 8) == 0) {
                board_init(&board);
                ptr += 8;
            } else if (strncmp(ptr, "fen", 3) == 0) {
                ptr += 3;
                while (*ptr == ' ') ptr++;
                board_from_fen(&board, ptr);
                /* Skip past the FEN fields (up to 6 tokens) */
                int spaces = 0;
                while (*ptr && spaces < 6) {
                    if (*ptr == ' ') {
                        spaces++;
                        while (*ptr == ' ') ptr++;
                        /* After 6 spaces we're past the FEN */
                        if (spaces == 6) break;
                        /* Check for "moves" keyword */
                        if (strncmp(ptr, "moves", 5) == 0) break;
                    } else {
                        ptr++;
                    }
                }
            }

            /* Parse moves */
            const char *mptr = strstr(ptr, "moves");
            if (mptr) {
                mptr += 5;
                while (*mptr) {
                    while (*mptr == ' ') mptr++;
                    if (!*mptr) break;
                    Move m = uci_to_move(&board, mptr);
                    if (m) {
                        Undo u;
                        board_make(&board, m, &u);
                    }
                    /* Skip this token */
                    while (*mptr && *mptr != ' ') mptr++;
                }
            }

        } else if (strncmp(line, "go", 2) == 0) {
            memset(&info, 0, sizeof info);
            info.stop = 0;

            const char *ptr = line + 2;
            while (*ptr) {
                while (*ptr == ' ') ptr++;
                if (!*ptr) break;

                if (strncmp(ptr, "depth", 5) == 0) {
                    ptr += 5;
                    info.depth = (int)strtol(ptr, (char **)&ptr, 10);
                } else if (strncmp(ptr, "movetime", 8) == 0) {
                    ptr += 8;
                    info.movetime = (int)strtol(ptr, (char **)&ptr, 10);
                } else if (strncmp(ptr, "wtime", 5) == 0) {
                    ptr += 5;
                    info.wtime = (int)strtol(ptr, (char **)&ptr, 10);
                } else if (strncmp(ptr, "btime", 5) == 0) {
                    ptr += 5;
                    info.btime = (int)strtol(ptr, (char **)&ptr, 10);
                } else if (strncmp(ptr, "winc", 4) == 0) {
                    ptr += 4;
                    info.winc = (int)strtol(ptr, (char **)&ptr, 10);
                } else if (strncmp(ptr, "binc", 4) == 0) {
                    ptr += 4;
                    info.binc = (int)strtol(ptr, (char **)&ptr, 10);
                } else if (strncmp(ptr, "movestogo", 9) == 0) {
                    ptr += 9;
                    info.movestogo = (int)strtol(ptr, (char **)&ptr, 10);
                } else if (strncmp(ptr, "infinite", 8) == 0) {
                    ptr += 8;
                    info.depth = MAX_PLY;
                } else {
                    /* Unknown token: skip */
                    while (*ptr && *ptr != ' ') ptr++;
                }
            }

            SearchResult result = { 0, 0, 0 };
            search_run(&board, &info, &result);
            printf("bestmove %s\n", move_to_uci(result.best_move));
            fflush(stdout);

        } else if (strcmp(line, "stop") == 0) {
            info.stop = 1;

        } else if (strcmp(line, "d") == 0) {
            board_print(&board);
            char fen[128];
            board_to_fen(&board, fen);
            printf("FEN: %s\n", fen);
            fflush(stdout);

        } else if (strcmp(line, "quit") == 0) {
            break;
        }
    }
}
