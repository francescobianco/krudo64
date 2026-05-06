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

Dipende da python-chess per il tracking preciso dello stato di gioco.
  pip install chess
"""

import subprocess
import sys
import os
import shutil

try:
    import chess
except ImportError:
    sys.exit("python-chess richiesto: pip install chess")

# ── UCI engine wrapper ────────────────────────────────────────────────────────

class UCIEngine:
    def __init__(self, path: str, name: str = "engine"):
        self.name = name
        self._proc = subprocess.Popen(
            [path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            bufsize=1,
        )

    def _send(self, cmd: str) -> None:
        self._proc.stdin.write(cmd + "\n")
        self._proc.stdin.flush()

    def _wait_for(self, prefix: str) -> str | None:
        """Legge righe fino a trovarne una che inizia con *prefix*."""
        while True:
            line = self._proc.stdout.readline()
            if not line:
                return None
            line = line.strip()
            if line.startswith(prefix):
                return line[len(prefix):]

    def init(self) -> None:
        self._send("uci");    self._wait_for("uciok")
        self._send("isready"); self._wait_for("readyok")

    def new_game(self) -> None:
        self._send("ucinewgame")
        self._send("isready"); self._wait_for("readyok")

    def bestmove(self, board: chess.Board, depth: int) -> chess.Move | None:
        """Chiede all'engine la mossa migliore per la posizione *board*."""
        # Costruisce il comando position con lista mosse
        moves = [m.uci() for m in board.move_stack]
        if moves:
            pos = "position startpos moves " + " ".join(moves)
        else:
            pos = "position startpos"

        self._send(pos)
        self._send(f"go depth {depth}")

        while True:
            line = self._proc.stdout.readline()
            if not line:
                return None
            line = line.strip()
            if line.startswith("bestmove"):
                tokens = line.split()
                if len(tokens) < 2 or tokens[1] in ("(none)", "0000"):
                    return None
                try:
                    mv = chess.Move.from_uci(tokens[1])
                    return mv if mv in board.legal_moves else None
                except ValueError:
                    return None

    def quit(self) -> None:
        try:
            self._send("quit")
            self._proc.wait(timeout=5)
        except Exception:
            self._proc.kill()


# ── game logic ────────────────────────────────────────────────────────────────

MAX_MOVES = 200  # mosse totali prima di dichiarare patta per troppo prolungamento


def play_game(
    white_path: str, white_depth: int,
    black_path: str, black_depth: int,
) -> str:
    """
    Gioca una partita completa tra due engine.
    Ritorna 'white', 'black' o 'draw'.
    """
    board  = chess.Board()
    white  = UCIEngine(white_path, "white")
    black  = UCIEngine(black_path, "black")
    white.init();  black.init()
    white.new_game(); black.new_game()

    try:
        for _ in range(MAX_MOVES):
            if board.is_game_over():
                break

            engine = white if board.turn == chess.WHITE else black
            depth  = white_depth if board.turn == chess.WHITE else black_depth

            mv = engine.bestmove(board, depth)
            if mv is None:
                break   # nessuna mossa: uscita anticipata (come stallo)

            board.push(mv)

        result = board.result(claim_draw=True)
        if result == "1-0":  return "white"
        if result == "0-1":  return "black"
        return "draw"

    finally:
        white.quit()
        black.quit()


# ── helpers ───────────────────────────────────────────────────────────────────

def find_exec(*names: str) -> str | None:
    for name in names:
        for candidate in (f"./{name}", f"../{name}", shutil.which(name) or ""):
            if candidate and os.access(candidate, os.X_OK):
                return candidate
    return None


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

    fruit_won_at: int | None = None

    for fd in range(1, 21):
        # partita 1: krudo=bianco(d5) vs fruit=nero(fd)
        r1 = play_game(krudo, KRUDO_DEPTH, fruit, fd)
        # partita 2: fruit=bianco(fd) vs krudo=nero(d5)
        r2 = play_game(fruit, fd, krudo, KRUDO_DEPTH)

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
