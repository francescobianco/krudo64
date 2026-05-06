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

# Le 4 feature con la loro chiave display e il nome UCI
FEATURES = [
    ("M", "UseMobility"),
    ("S", "UsePawnStructure"),
    ("C", "UseClusters"),
    ("K", "UseKingAttack"),
]

# Genera tutte le 2^4 = 16 combinazioni
def _build_combos():
    combos = []
    for bits in range(1 << len(FEATURES)):
        flags = [bool(bits & (1 << i)) for i in range(len(FEATURES))]
        label = "+".join(k for (k, _), on in zip(FEATURES, flags) if on) or "none"
        combos.append((label, *flags))
    return combos

COMBOS = _build_combos()


def combo_opts(flags: list[bool]) -> dict:
    return {name: ("true" if on else "false")
            for (_, name), on in zip(FEATURES, flags)}


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
    feat_keys       = "  ".join(k for k, _ in FEATURES)
    print(f"krudo64 : {krudo}  depth={KRUDO_DEPTH}")
    print(f"fruit   : {fruit}  depth d{FRUIT_DEPTHS.start}..d{FRUIT_DEPTHS.stop - 1}")
    print(f"combo   : {len(COMBOS)}  |  partite: {games_per_combo}/combo × {len(COMBOS)} = {total_games}"
          f"  (max {games_per_combo * 2} pts/combo)")
    print()

    # Header tabella
    W = 13
    print(f"  {'combo':^{W}}  {feat_keys}  |  W   D   L  | pts/{games_per_combo * 2}")
    sep = "  " + "─" * (W + 4 * len(FEATURES) + 22)
    print(sep)

    all_games = []
    results   = []

    for row in COMBOS:
        label = row[0]
        flags = list(row[1:])
        opts  = combo_opts(flags)
        kl    = f"krudo({label})"
        wins  = draws = losses = 0

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
        results.append((label, flags, wins, draws, losses, pts))

        feat_display = "  ".join("Y" if on else "." for on in flags)
        print(f"  {label:^{W}}  {feat_display}  | {wins:>2}  {draws:>2}  {losses:>2}  | {pts:>3}")
        sys.stdout.flush()

    print(sep)
    print()

    # Classifica per punti (parità: più vittorie prima)
    results.sort(key=lambda r: (r[5], r[2]), reverse=True)
    print("Classifica:")
    for rank, (lbl, flags, w, d, l, pts) in enumerate(results, 1):
        feat_str = "".join(k if on else "-" for (k, _), on in zip(FEATURES, flags))
        print(f"  {rank:>2}. [{feat_str}] {lbl:<{W}}  {pts:>3} pts  ({w}W {d}D {l}L)")
    print()

    best = results[0]
    print(f"Combinazione migliore: {best[0]}  ({best[5]} pts)")

    pgn_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "feature_matrix.pgn")
    write_pgn(all_games, pgn_path)

    return 0


if __name__ == "__main__":
    sys.exit(main())
