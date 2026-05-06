#!/usr/bin/env python3
"""
tests/sparring/engine.py
Shared UCI engine wrapper, game runner, and utilities for sparring scripts.
"""
import subprocess
import shutil
import os
import datetime

try:
    import chess
    import chess.pgn
except ImportError:
    import sys
    sys.exit("python-chess richiesto: pip install chess")

MAX_MOVES = 200


class UCIEngine:
    def __init__(self, path: str, name: str = "engine", options: dict = None):
        self.name = name
        self._options = options or {}
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
        while True:
            line = self._proc.stdout.readline()
            if not line:
                return None
            line = line.strip()
            if line.startswith(prefix):
                return line[len(prefix):]

    def init(self) -> None:
        self._send("uci")
        self._wait_for("uciok")
        for name, val in self._options.items():
            self._send(f"setoption name {name} value {val}")
        self._send("isready")
        self._wait_for("readyok")

    def new_game(self) -> None:
        self._send("ucinewgame")
        self._send("isready")
        self._wait_for("readyok")

    def bestmove(self, board: chess.Board, depth: int) -> chess.Move | None:
        moves = [m.uci() for m in board.move_stack]
        pos = ("position startpos moves " + " ".join(moves)) if moves else "position startpos"
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


def play_game(
    white_path: str, white_depth: int, white_label: str,
    black_path:  str, black_depth:  int, black_label:  str,
    round_tag: str,
    white_opts: dict = None,
    black_opts: dict = None,
    event: str = "krudo64 sparring",
) -> tuple[str, chess.pgn.Game]:
    """
    Plays a complete game. Returns (outcome, pgn_game).
    outcome is 'white' | 'black' | 'draw'.
    """
    board = chess.Board()
    white = UCIEngine(white_path, white_label, white_opts)
    black = UCIEngine(black_path,  black_label, black_opts)
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
                break
            board.push(mv)

        result_str = board.result(claim_draw=True)

        game = chess.pgn.Game()
        game.headers["Event"]  = event
        game.headers["Site"]   = "local"
        game.headers["Date"]   = datetime.date.today().strftime("%Y.%m.%d")
        game.headers["Round"]  = round_tag
        game.headers["White"]  = white_label
        game.headers["Black"]  = black_label
        game.headers["Result"] = result_str

        node = game
        for mv in board.move_stack:
            node = node.add_variation(mv)

        if   result_str == "1-0": outcome = "white"
        elif result_str == "0-1": outcome = "black"
        else:                     outcome = "draw"

        return outcome, game

    finally:
        white.quit()
        black.quit()


def find_exec(*names: str) -> str | None:
    for name in names:
        for candidate in (f"./{name}", f"../{name}", shutil.which(name) or ""):
            if candidate and os.access(candidate, os.X_OK):
                return candidate
    return None


def write_pgn(games: list, path: str) -> None:
    with open(path, "w") as f:
        for game in games:
            print(game, file=f)
            print(file=f)
    print(f"PGN salvato → {path}  ({len(games)} partite)")
