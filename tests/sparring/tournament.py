#!/usr/bin/env python3
"""
tests/sparring/tournament.py

Torneo round-robin all'italiana tra tutte le 16 combinazioni di feature di krudo64.

Formato
-------
  Ogni coppia (A, B) gioca due partite:
    1. A=bianco  vs  B=nero
    2. B=bianco  vs  A=nero
  16 giocatori × 15 avversari × 2 = 240 partite totali.
  Punteggio: vittoria=2, patta=1, sconfitta=0  (max 60 pts/giocatore).

Opzioni
-------
  --depth N   profondità di ricerca per entrambi i motori (default 5)

Output
------
  Classifica finale + tests/sparring/tournament.pgn con tutte le partite.
"""

import sys
import os
import argparse

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from engine import play_game, find_exec, write_pgn       # noqa: E402
from feature_matrix import COMBOS, FEATURES, combo_opts  # noqa: E402


def main() -> int:
    ap = argparse.ArgumentParser(description="krudo64 round-robin feature tournament")
    ap.add_argument("--depth", type=int, default=5,
                    help="search depth for both engines (default 5)")
    args = ap.parse_args()
    depth = args.depth

    krudo = find_exec("krudo64")
    if not krudo:
        print("SKIP — krudo64 non trovato (eseguire: make build)")
        return 0

    n           = len(COMBOS)
    total_games = n * (n - 1)
    max_pts     = (n - 1) * 4   # (n-1) avversari × 2 partite × 2 pts max

    print(f"krudo64   : {krudo}  depth={depth}")
    print(f"giocatori : {n}")
    print(f"partite   : {total_games}  (max {max_pts} pts/giocatore)")
    print()

    scores    = [0] * n
    w_cnt     = [0] * n
    d_cnt     = [0] * n
    l_cnt     = [0] * n
    all_games = []
    game_num  = 0

    for i in range(n):
        for j in range(n):
            if i == j:
                continue
            game_num += 1
            li = COMBOS[i][0]
            lj = COMBOS[j][0]
            oi = combo_opts(list(COMBOS[i][1:]))
            oj = combo_opts(list(COMBOS[j][1:]))

            print(f"  [{game_num:>3}/{total_games}]  {li:>13} ── W ──  vs  ── B ──  {lj:<13}  ... ",
                  end="", flush=True)

            r, g = play_game(
                krudo, depth, li,
                krudo, depth, lj,
                f"{game_num:03d}",
                white_opts=oi,
                black_opts=oj,
                event="krudo64 round-robin",
            )
            all_games.append(g)

            if r == "white":
                scores[i] += 2;  w_cnt[i] += 1;  l_cnt[j] += 1
                print(f"  {li}")
            elif r == "black":
                scores[j] += 2;  w_cnt[j] += 1;  l_cnt[i] += 1
                print(f"  {lj}")
            else:
                scores[i] += 1;  d_cnt[i] += 1
                scores[j] += 1;  d_cnt[j] += 1
                print("  ½-½")

    print()

    # ── Classifica ──────────────────────────────────────────────────────────────
    ranking = sorted(range(n), key=lambda x: (scores[x], w_cnt[x]), reverse=True)

    W = 13
    print(f"  {'#':>2}  {'feat':^4}  {'combo':^{W}}  |  {'G':>3}  {'W':>3}  {'D':>3}  {'L':>3}  | {'pts':>4}/{max_pts}  pct")
    print("  " + "─" * (W + 46))

    for rank, idx in enumerate(ranking, 1):
        label    = COMBOS[idx][0]
        flags    = list(COMBOS[idx][1:])
        feat_str = "".join(k if on else "-" for (k, _), on in zip(FEATURES, flags))
        g        = w_cnt[idx] + d_cnt[idx] + l_cnt[idx]
        pct      = scores[idx] / max_pts * 100 if max_pts else 0
        print(
            f"  {rank:>2}. [{feat_str}]  {label:^{W}}  |"
            f"  {g:>3}  {w_cnt[idx]:>3}  {d_cnt[idx]:>3}  {l_cnt[idx]:>3}"
            f"  | {scores[idx]:>4}  {pct:5.1f}%"
        )

    print()
    best = ranking[0]
    print(f"Vincitore del torneo: {COMBOS[best][0]}  ({scores[best]}/{max_pts} pts)")

    pgn_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "tournament.pgn")
    write_pgn(all_games, pgn_path)

    return 0


if __name__ == "__main__":
    sys.exit(main())
