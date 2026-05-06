#include "../common.h"
#include "../../src/atlas.h"
#include "../../src/board.h"
#include "../../src/types.h"

int main(void)
{
    atlas_init();

    /* ── FEN parsing: starting position ──────────────────────────────────── */
    Board b;
    board_init(&b);

    /* White pawns on rank 2 */
    ASSERT(b.pieces[WHITE][PAWN] == RANK_2, "start: white pawns on rank 2");
    /* Black pawns on rank 7 */
    ASSERT(b.pieces[BLACK][PAWN] == RANK_7, "start: black pawns on rank 7");
    /* White knights on b1 and g1 */
    ASSERT(b.pieces[WHITE][KNIGHT] & BB(B1), "start: white knight on b1");
    ASSERT(b.pieces[WHITE][KNIGHT] & BB(G1), "start: white knight on g1");
    /* White king on e1 */
    ASSERT(b.pieces[WHITE][KING] & BB(E1), "start: white king on e1");
    /* Black king on e8 */
    ASSERT(b.pieces[BLACK][KING] & BB(E8), "start: black king on e8");
    /* Side to move is WHITE */
    ASSERT(b.side == WHITE, "start: white to move");
    /* Castling rights: all four */
    ASSERT(b.castle == (CR_WK | CR_WQ | CR_BK | CR_BQ), "start: all castling rights");
    /* No en passant */
    ASSERT(b.ep == NO_SQ, "start: no en passant");

    /* ── board_to_fen round-trip ──────────────────────────────────────────── */
    char fen_out[128];
    board_to_fen(&b, fen_out);
    Board b2;
    board_from_fen(&b2, fen_out);
    char fen_out2[128];
    board_to_fen(&b2, fen_out2);
    ASSERT(strcmp(fen_out, fen_out2) == 0, "FEN round-trip stable");

    /* ── board_in_check: starting position not in check ─────────────────── */
    ASSERT(!board_in_check(&b, WHITE), "start: white not in check");
    ASSERT(!board_in_check(&b, BLACK), "start: black not in check");

    /* ── board_in_check: known check position ─────────────────────────────── */
    /* White queen on h5 checks black king on e8 via nothing... let's use a
     * simpler direct check: rook on e7 with black to move, king on e8.     */
    Board bc;
    board_from_fen(&bc, "4k3/4R3/8/8/8/8/8/4K3 b - - 0 1");
    ASSERT(board_in_check(&bc, BLACK), "black king in check from rook e7");
    ASSERT(!board_in_check(&bc, WHITE), "white not in check");

    /* ── board_make + board_undo: e2e4 from start ────────────────────────── */
    Board bm;
    board_init(&bm);
    /* Build the e2e4 move manually */
    Move e2e4 = MK_MOVE(E2, E4, PAWN, NONE, NONE, 0, 0, 1);
    Undo u;
    Bitboard orig_pawn = bm.pieces[WHITE][PAWN];
    board_make(&bm, e2e4, &u);
    ASSERT(bm.ep == E3, "after e2e4: ep target is e3");
    ASSERT(bm.pieces[WHITE][PAWN] & BB(E4), "after e2e4: pawn on e4");
    ASSERT(!(bm.pieces[WHITE][PAWN] & BB(E2)), "after e2e4: pawn off e2");
    ASSERT(bm.side == BLACK, "after e2e4: black to move");

    board_undo(&bm, e2e4, &u);
    ASSERT(bm.pieces[WHITE][PAWN] == orig_pawn, "after undo e2e4: pawns restored");
    ASSERT(bm.ep == NO_SQ, "after undo e2e4: ep restored to NO_SQ");
    ASSERT(bm.side == WHITE, "after undo e2e4: white to move");

    /* ── board_piece_at ───────────────────────────────────────────────────── */
    board_init(&bm);
    ASSERT(board_piece_at(&bm, E1, WHITE) == KING, "piece_at e1 white = KING");
    ASSERT(board_piece_at(&bm, D1, WHITE) == QUEEN, "piece_at d1 white = QUEEN");
    ASSERT(board_piece_at(&bm, H8, BLACK) == ROOK, "piece_at h8 black = ROOK");
    ASSERT(board_piece_at(&bm, E4, WHITE) == NONE, "piece_at e4 white = NONE");

    /* ── board_print doesn't crash ────────────────────────────────────────── */
    board_init(&bm);
    board_print(&bm);
    ASSERT(1, "board_print does not crash");

    TEST_SUMMARY();
}
