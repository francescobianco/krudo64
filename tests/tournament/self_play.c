#include <stdio.h>
#include <string.h>
#include "../../src/atlas.h"
#include "../../src/board.h"
#include "../../src/movegen.h"
#include "../../src/search.h"
#include "../../src/uci.h"
#include "../../src/types.h"

int main(void)
{
    atlas_init();

    Board board;
    board_init(&board);

    printf("==> self-play tournament (5 moves each side)\n");
    board_print(&board);

    SearchInfo info;
    SearchResult result;
    int total_moves = 10; /* 5 each side */
    int legal_count = 0;

    for (int move_num = 0; move_num < total_moves; move_num++) {
        Move legal[MAX_MOVES];
        int nlegal = movegen_legal(&board, legal);
        if (nlegal == 0) {
            if (board_in_check(&board, board.side))
                printf("Checkmate! %s wins.\n", board.side == WHITE ? "Black" : "White");
            else
                printf("Stalemate!\n");
            break;
        }

        memset(&info, 0, sizeof info);
        memset(&result, 0, sizeof result);
        info.depth = 4; /* shallow depth for speed */

        search_run(&board, &info, &result);

        if (!result.best_move) {
            printf("Engine returned no move — stopping.\n");
            break;
        }

        /* Verify the returned move is legal */
        int found = 0;
        for (int i = 0; i < nlegal; i++) {
            if (legal[i] == result.best_move) { found = 1; break; }
        }
        if (!found) {
            printf("FAIL: engine returned illegal move %s\n",
                   move_to_uci(result.best_move));
            return 1;
        }

        printf("Move %d (%s): %s\n",
               move_num + 1,
               board.side == WHITE ? "white" : "black",
               move_to_uci(result.best_move));

        Undo u;
        board_make(&board, result.best_move, &u);
        legal_count++;
    }

    board_print(&board);
    char fen[128];
    board_to_fen(&board, fen);
    printf("Final FEN: %s\n", fen);
    printf("[self-play] %d legal moves played — no crashes\n", legal_count);
    return 0;
}
