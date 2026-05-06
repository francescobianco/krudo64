# krudo64

A minimalist UCI chess engine written in C.

## Philosophy

krudo64 is built around two principles:

1. **Minimalism** — clean, readable C11 code with no external dependencies.
2. **Movement Atlas** — all piece attack tables (knight jumps, king moves, sliding rays, pawn attacks) are pre-computed once at startup into a single global `Atlas` struct stored in RAM. Move generation queries this atlas at zero allocation cost.

## Architecture

```
atlas (pre-computed maps)
  └── movegen (pseudo-legal + legal filtering)
        ├── eval (material + piece-square tables)
        └── search (iterative deepening alpha-beta + quiescence)
              └── uci (stdin/stdout UCI protocol loop)
```

### src/types.h
Core types, enums, and macros. Defines `Bitboard` (`uint64_t`), `Move` (`uint32_t`), square/piece/color constants, move encoding macros, and bitboard helpers.

### src/atlas.h / src/atlas.c
The Movement Atlas. `atlas_init()` fills:
- `pawn[2][64]` — pawn attack squares per color
- `knight[64]` — knight jump targets
- `king[64]` — king one-step moves
- `ray[64][8]` — full directional rays in all 8 directions
- `bishop/rook/queen[64]` — combined empty-board sliding rays
- `between[64][64]` — squares strictly between two aligned squares

Sliding attacks with blockers use `ray ^ ray[blocker][dir]` to efficiently clip rays at the first occupied square.

### src/board.h / src/board.c
Board state (bitboards per color/piece, occupancy, castling rights, en passant, clocks) with FEN parsing/generation, `board_make` / `board_undo` (exact inverses), and attack/check detection.

### src/movegen.h / src/movegen.c
Generates pseudo-legal moves via atlas lookups, then filters with `board_in_check` to produce the legal move list. Separate `movegen_captures` path feeds quiescence search.

### src/eval.h / src/eval.c
Static evaluation: material + Michniewski Simplified Evaluation Function piece-square tables. Returns score in centipawns from the side-to-move perspective.

### src/search.h / src/search.c
Iterative-deepening negamax alpha-beta with quiescence search. Move ordering by MVV-LVA (Most Valuable Victim – Least Valuable Attacker). Prints UCI `info` lines per depth iteration.

### src/uci.h / src/uci.c
UCI protocol loop: handles `uci`, `isready`, `ucinewgame`, `position`, `go`, `stop`, `quit`, and the debug command `d`.

## Build

Requires a C11 compiler (GCC or Clang) and GNU make.

```bash
make build        # compile → ./krudo64
make run          # compile and launch the UCI loop
make clean        # remove build artifacts
```

## Tests

```bash
make test                # run all test suites
make test-unit           # atlas, board, movegen unit tests
make test-functional     # perft node-count validation
make test-integration    # full game sequences (Scholar's mate, Fool's mate)
make test-challenge      # tactical puzzles (mate-in-1, mate-in-2)
make test-tournament     # self-play 10 moves, no crashes
```

### Perft reference values

| Position | Depth | Nodes   |
|----------|-------|---------|
| Start    | 1     | 20      |
| Start    | 2     | 400     |
| Start    | 3     | 8 902   |
| Start    | 4     | 197 281 |
| Kiwipete | 1     | 48      |
| Kiwipete | 2     | 2 039   |
| Kiwipete | 3     | 97 862  |
| Pos 3    | 1     | 14      |
| Pos 3    | 2     | 191     |
| Pos 3    | 3     | 2 812   |

## UCI Usage

Connect krudo64 to any UCI-compatible GUI (Arena, CuteChess, Banksia, etc.) by pointing it to the `krudo64` binary.

Or drive it by hand:

```
$ ./krudo64
uci
id name krudo64
id author Francesco Bianco
uciok
isready
readyok
position startpos moves e2e4 e7e5
go depth 6
info depth 1 score cp 25 nodes 20 ...
info depth 6 score cp 15 nodes 84231 ...
bestmove g1f3
quit
```

The `d` command prints an ASCII board:

```
d
  +---+---+---+---+---+---+---+---+
8 | r | n | b | q | k | b | n | r |
  +---+---+---+---+---+---+---+---+
...
```

## Contributing

- Keep source files self-contained and dependency-free.
- All changes to move generation must pass the perft suite (`make test-functional`).
- New features should come with a test in the appropriate `tests/` sub-directory.
- Code style: 4-space indent, K&R braces, 100-column soft limit.
