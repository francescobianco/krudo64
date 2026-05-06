#include "../common.h"
#include "../../src/atlas.h"
#include "../../src/types.h"

int main(void)
{
    atlas_init();

    /* ── Knight attacks from e4 (sq=28) ──────────────────────────────────── */
    /* e4 = file4, rank3 (0-indexed). Knight from e4 can reach:
     * d2(11), f2(13), c3(18), g3(22), c5(34), g5(38), d6(43), f6(45) */
    Bitboard kn_e4 = ATLAS.knight[E4];
    ASSERT(kn_e4 & BB(D2), "knight e4 attacks d2");
    ASSERT(kn_e4 & BB(F2), "knight e4 attacks f2");
    ASSERT(kn_e4 & BB(C3), "knight e4 attacks c3");
    ASSERT(kn_e4 & BB(G3), "knight e4 attacks g3");
    ASSERT(kn_e4 & BB(C5), "knight e4 attacks c5");
    ASSERT(kn_e4 & BB(G5), "knight e4 attacks g5");
    ASSERT(kn_e4 & BB(D6), "knight e4 attacks d6");
    ASSERT(kn_e4 & BB(F6), "knight e4 attacks f6");
    ASSERT(POPCNT(kn_e4) == 8, "knight e4 has exactly 8 attacks");

    /* ── Knight attacks from a1 (corner, only 2 attacks) ─────────────────── */
    Bitboard kn_a1 = ATLAS.knight[A1];
    ASSERT(kn_a1 & BB(B3), "knight a1 attacks b3");
    ASSERT(kn_a1 & BB(C2), "knight a1 attacks c2");
    ASSERT(POPCNT(kn_a1) == 2, "knight a1 has exactly 2 attacks");

    /* ── King attacks (e4 has 8 neighbors) ───────────────────────────────── */
    Bitboard kg_e4 = ATLAS.king[E4];
    ASSERT(POPCNT(kg_e4) == 8, "king e4 has 8 neighbors");
    ASSERT(kg_e4 & BB(D3), "king e4 attacks d3");
    ASSERT(kg_e4 & BB(E3), "king e4 attacks e3");
    ASSERT(kg_e4 & BB(F3), "king e4 attacks f3");
    ASSERT(kg_e4 & BB(D4), "king e4 attacks d4");
    ASSERT(kg_e4 & BB(F4), "king e4 attacks f4");
    ASSERT(kg_e4 & BB(D5), "king e4 attacks d5");
    ASSERT(kg_e4 & BB(E5), "king e4 attacks e5");
    ASSERT(kg_e4 & BB(F5), "king e4 attacks f5");

    /* ── King attacks from corner a1 (only 3) ────────────────────────────── */
    Bitboard kg_a1 = ATLAS.king[A1];
    ASSERT(POPCNT(kg_a1) == 3, "king a1 has 3 neighbors");

    /* ── Pawn attacks ─────────────────────────────────────────────────────── */
    /* White pawn on e4 attacks d5 and f5 */
    Bitboard pw_e4 = ATLAS.pawn[WHITE][E4];
    ASSERT(pw_e4 & BB(D5), "white pawn e4 attacks d5");
    ASSERT(pw_e4 & BB(F5), "white pawn e4 attacks f5");
    ASSERT(POPCNT(pw_e4) == 2, "white pawn e4 has 2 attacks");

    /* Black pawn on e5 attacks d4 and f4 */
    Bitboard pb_e5 = ATLAS.pawn[BLACK][E5];
    ASSERT(pb_e5 & BB(D4), "black pawn e5 attacks d4");
    ASSERT(pb_e5 & BB(F4), "black pawn e5 attacks f4");
    ASSERT(POPCNT(pb_e5) == 2, "black pawn e5 has 2 attacks");

    /* ── Ray from a1 going North ──────────────────────────────────────────── */
    Bitboard ray_a1_n = ATLAS.ray[A1][DIR_N];
    ASSERT(ray_a1_n & BB(A2), "ray a1 North hits a2");
    ASSERT(ray_a1_n & BB(A3), "ray a1 North hits a3");
    ASSERT(ray_a1_n & BB(A8), "ray a1 North hits a8");
    ASSERT(POPCNT(ray_a1_n) == 7, "ray a1 North has 7 squares");

    /* ── Between table ────────────────────────────────────────────────────── */
    Bitboard btw = ATLAS.between[A1][A4];
    ASSERT(btw & BB(A2), "between a1-a4 contains a2");
    ASSERT(btw & BB(A3), "between a1-a4 contains a3");
    ASSERT(!(btw & BB(A1)), "between a1-a4 does not contain a1");
    ASSERT(!(btw & BB(A4)), "between a1-a4 does not contain a4");
    ASSERT(POPCNT(btw) == 2, "between a1-a4 has exactly 2 squares");

    /* Non-aligned squares → empty between */
    ASSERT(ATLAS.between[A1][B3] == 0, "between a1-b3 is 0 (not aligned)");

    /* ── Bishop attacks on empty board from e4 ─────────────────────────────── */
    Bitboard ba_empty = bishop_attacks(E4, 0ULL);
    /* Should hit all 4 diagonals */
    ASSERT(ba_empty & BB(D3), "bishop e4 empty attacks d3");
    ASSERT(ba_empty & BB(C2), "bishop e4 empty attacks c2");
    ASSERT(ba_empty & BB(B1), "bishop e4 empty attacks b1");
    ASSERT(ba_empty & BB(F5), "bishop e4 empty attacks f5");
    ASSERT(ba_empty & BB(G6), "bishop e4 empty attacks g6");
    ASSERT(ba_empty & BB(H7), "bishop e4 empty attacks h7");
    ASSERT(ba_empty & BB(D5), "bishop e4 empty attacks d5");
    ASSERT(ba_empty & BB(F3), "bishop e4 empty attacks f3");

    /* ── Rook attacks with blocker ────────────────────────────────────────── */
    /* Rook on e4, blocker on e6 — North ray should stop at e6 inclusive */
    Bitboard occ = BB(E6);
    Bitboard ra = rook_attacks(E4, occ);
    ASSERT(ra & BB(E5), "rook e4 w/blocker e6 attacks e5");
    ASSERT(ra & BB(E6), "rook e4 w/blocker e6 attacks e6 (inclusive)");
    ASSERT(!(ra & BB(E7)), "rook e4 w/blocker e6 does NOT attack e7");
    ASSERT(!(ra & BB(E8)), "rook e4 w/blocker e6 does NOT attack e8");

    TEST_SUMMARY();
}
