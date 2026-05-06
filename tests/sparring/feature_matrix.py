#!/usr/bin/env python3
"""
tests/sparring/feature_matrix.py

Testa tutte le 8 combinazioni delle feature di valutazione di krudo64
contro fruit a profondità fissa e classifica le combinazioni per punteggio.

Feature:
  M = UseMobility      — penalizza libertà di movimento avversario
  S = UsePawnStructure — struttura pedoni per colonna
  C = UseClusters      — cluster di pedoni adiacenti

Punteggio: vittoria=2, patta=1, sconfitta=0

Dipende da python-chess:  pip install chess
"""

import sys
import os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from engine import play_game, find_exec, write_pgn  # noqa: E402

# ── Configurazione ────────────────────────────────────────────────────────────
KRUDO_DEPTH  = 5            # profondità krudo fissa
FRUIT_DEPTHS = range(1, 6)  # fruit d1..d5 — 1 coppia ciascuna = 10 partite/combo

# Le 8 combinazioni: (label, mob, pawn_struct, clusters)
COMBOS = [
    ("none",      False, False, False),
    ("mob",       True,  False, False),
    ("struct",    False, True,  False),
    ("cluster",   False, False, True),
    ("mob+str",   True,  True,  False),
    ("mob+clu",   True,  False, True),
    ("str+clu",   False, True,  True),
    ("all",       True,  True,  True),
]


def combo_opts(mob: bool, struct: bool, cluster: bool) -> dict:
    return {
        "UseMobility":      "true" if mob     else "false",
        "UsePawnStructure": "true" if struct  else "false",
        "UseClusters":      "true" if cluster else "false",
    }


def main() -> int:
    krudo = find_exec("krudo64")
    fruit = find_exec("fruit")

    if not krudo:
        print("SKIP — krudo64 non trovato (eseguire: make build)")
        return 0
    if not fruit:
        print("SKIP — fruit non trovato (sudo apt install fruit)")
        return 0

    games_per_combo = len(FRUIT_DEPTHS) * 2
    total_games     = len(COMBOS) * games_per_combo
    print(f"krudo64 : {krudo}  depth={KRUDO_DEPTH}")
    print(f"fruit   : {fruit}  depth d{FRUIT_DEPTHS.start}..d{FRUIT_DEPTHS.stop - 1}")
    print(f"partite : {games_per_combo}/combo × {len(COMBOS)} combo = {total_games} totali"
          f"  (max {games_per_combo * 2} pts/combo)")
    print()

    # Header tabella
    W = 10
    print(f"  {'combo':^{W}}  M  S  C  |  W   D   L  | pts/{games_per_combo * 2}")
    print("  " + "─" * (W + 34))

    all_games = []
    results   = []

    for label, mob, struct, cluster in COMBOS:
        opts = combo_opts(mob, struct, cluster)
        kl   = f"krudo({label})"
        wins = draws = losses = 0

        for fd in FRUIT_DEPTHS:
            fl = f"fruit(d{fd})"

            # krudo = bianco
            r1, g1 = play_game(
                krudo, KRUDO_DEPTH, kl,
                fruit, fd, fl,
                f"{label}.d{fd}w",
                white_opts=opts,
                event="krudo64 feature-matrix",
            )
            all_games.append(g1)
            if   r1 == "white": wins   += 1
            elif r1 == "black": losses += 1
            else:               draws  += 1

            # krudo = nero
            r2, g2 = play_game(
                fruit, fd, fl,
                krudo, KRUDO_DEPTH, kl,
                f"{label}.d{fd}b",
                black_opts=opts,
                event="krudo64 feature-matrix",
            )
            all_games.append(g2)
            if   r2 == "black": wins   += 1
            elif r2 == "white": losses += 1
            else:               draws  += 1

        pts = wins * 2 + draws
        results.append((label, mob, struct, cluster, wins, draws, losses, pts))

        ms = "Y" if mob     else "."
        ss = "Y" if struct  else "."
        cs = "Y" if cluster else "."
        print(f"  {label:^{W}}  {ms}  {ss}  {cs}  | {wins:>2}  {draws:>2}  {losses:>2}  | {pts:>3}")
        sys.stdout.flush()

    print("  " + "─" * (W + 34))
    print()

    # Classifica per punti (parità: più vittorie prima)
    results.sort(key=lambda r: (r[7], r[4]), reverse=True)
    print("Classifica:")
    for rank, (lbl, mob, struct, cluster, w, d, l, pts) in enumerate(results, 1):
        ms = "M" if mob     else "-"
        ss = "S" if struct  else "-"
        cs = "C" if cluster else "-"
        print(f"  {rank}. [{ms}{ss}{cs}] {lbl:<{W}}  {pts} pts  ({w}W {d}D {l}L)")
    print()

    best = results[0]
    print(f"Combinazione migliore: {best[0]}  ({best[7]} pts)")

    pgn_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "feature_matrix.pgn")
    write_pgn(all_games, pgn_path)

    return 0


if __name__ == "__main__":
    sys.exit(main())
