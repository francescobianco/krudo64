#!/usr/bin/env python3
"""
tests/sparring/vs_fruit.py

Sfida progressiva krudo64 vs fruit.

Regole
------
- krudo64 gioca sempre a profondità KRUDO_DEPTH (default 5)
- fruit parte da profondità 1 e sale di 1 a ogni round
- ogni round = 2 partite: krudo=bianco e krudo=nero
- il ciclo si ferma quando fruit vince ENTRAMBE le partite dello stesso round
- PASS se la profondità di fruit necessaria è > THRESHOLD (6)
- FAIL altrimenti

Al termine sovrascrive tests/sparring/vs_fruit.pgn con tutte le partite
giocate (committare per analisi esterna).

Dipende da python-chess:  pip install chess
"""

import sys
import os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from engine import UCIEngine, play_game, find_exec, write_pgn  # noqa: E402


# ── main ──────────────────────────────────────────────────────────────────────

def main() -> int:
    krudo = find_exec("krudo64")
    fruit = find_exec("fruit")

    if not krudo:
        print("SKIP — krudo64 non trovato (eseguire: make build)")
        return 0
    if not fruit:
        print("SKIP — fruit non trovato (sudo apt install fruit)")
        return 0

    print(f"krudo64 : {krudo}")
    print(f"fruit   : {fruit}")
    print()

    KRUDO_DEPTH = 5
    THRESHOLD   = 6
    col         = 22

    print(f"  {'d':>2}   {'krudo=bianco':^{col}}   {'krudo=nero':^{col}}")
    print("  " + "-" * (col * 2 + 12))

    pgn_games:    list = []
    fruit_won_at: int | None = None

    for fd in range(1, 21):
        kl = f"krudo64 (d{KRUDO_DEPTH})"
        fl = f"fruit (d{fd})"

        r1, g1 = play_game(krudo, KRUDO_DEPTH, kl, fruit, fd, fl,
                           f"{fd}.1", event="krudo64 sparring vs fruit")
        pgn_games.append(g1)

        r2, g2 = play_game(fruit, fd, fl, krudo, KRUDO_DEPTH, kl,
                           f"{fd}.2", event="krudo64 sparring vs fruit")
        pgn_games.append(g2)

        s1 = ("krudo vince" if r1 == "white"
              else "fruit vince" if r1 == "black"
              else "patta")
        s2 = ("krudo vince" if r2 == "black"
              else "fruit vince" if r2 == "white"
              else "patta")

        fruit_wins_both = (r1 == "black") and (r2 == "white")
        marker = "  ← fruit vince entrambe" if fruit_wins_both else ""

        print(f"  {fd:>2}   {s1:^{col}}   {s2:^{col}}{marker}")
        sys.stdout.flush()

        if fruit_wins_both:
            fruit_won_at = fd
            break

    print()

    pgn_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "vs_fruit.pgn")
    write_pgn(pgn_games, pgn_path)
    print()

    if fruit_won_at is None:
        print("PASS — krudo64 ha tenuto contro fruit fino alla profondità 20")
        return 0
    if fruit_won_at > THRESHOLD:
        print(
            f"PASS — fruit ha impiegato profondità {fruit_won_at}"
            f" (> {THRESHOLD}) per vincere entrambi i lati"
        )
        return 0
    print(
        f"FAIL — fruit ha vinto entrambi a profondità {fruit_won_at}"
        f" (<= {THRESHOLD})"
    )
    return 1


if __name__ == "__main__":
    sys.exit(main())
