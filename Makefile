CC      = cc
CFLAGS  = -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS =
TARGET  = krudo64
SRCS    = $(wildcard src/*.c)
OBJS    = $(patsubst src/%.c, out/%.o, $(SRCS))
# All source files except main.c (for tests that supply their own main)
LIB_SRCS = src/atlas.c src/board.c src/movegen.c src/eval.c src/search.c src/uci.c

depth ?= 5

.PHONY: build run perft test test-unit test-functional test-integration test-challenge test-tournament clean help

build: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

out/%.o: src/%.c | out
	$(CC) $(CFLAGS) -c -o $@ $<

out:
	@mkdir -p out

run: build
	@./$(TARGET)

perft: out/perft_run
	@./out/perft_run $(depth)

out/perft_run: tests/functional/perft_run.c $(LIB_SRCS) | out
	@$(CC) $(CFLAGS) -o $@ $^

test: test-unit test-functional test-integration test-challenge

test-unit: build
	@echo "==> unit tests"
	@$(CC) $(CFLAGS) tests/unit/test_atlas.c src/atlas.c -o /tmp/k64_test_atlas && /tmp/k64_test_atlas
	@$(CC) $(CFLAGS) tests/unit/test_board.c src/atlas.c src/board.c -o /tmp/k64_test_board && /tmp/k64_test_board
	@$(CC) $(CFLAGS) tests/unit/test_movegen.c src/atlas.c src/board.c src/movegen.c -o /tmp/k64_test_movegen && /tmp/k64_test_movegen

test-functional: build
	@echo "==> functional tests (perft)"
	@$(CC) $(CFLAGS) tests/functional/perft.c src/atlas.c src/board.c src/movegen.c -o /tmp/k64_perft && /tmp/k64_perft

test-integration: build
	@echo "==> integration tests"
	@$(CC) $(CFLAGS) tests/integration/test_game.c src/atlas.c src/board.c src/movegen.c -o /tmp/k64_test_game && /tmp/k64_test_game

test-challenge: build
	@echo "==> challenge tests (tactics)"
	@$(CC) $(CFLAGS) tests/challenge/test_tactics.c $(LIB_SRCS) -o /tmp/k64_tactics && /tmp/k64_tactics

test-tournament: build
	@echo "==> tournament (self-play)"
	@$(CC) $(CFLAGS) tests/tournament/self_play.c $(LIB_SRCS) -o /tmp/k64_selfplay && /tmp/k64_selfplay

clean:
	@rm -rf out $(TARGET) /tmp/k64_*
	@echo "cleaned"

help:
	@echo "krudo64 — chess engine"
	@echo ""
	@echo "  make build              compile the engine"
	@echo "  make run                compile and launch UCI loop"
	@echo "  make test               run all test suites"
	@echo "  make test-unit          unit tests (atlas, board, movegen)"
	@echo "  make test-functional    perft node-count tests"
	@echo "  make test-integration   game sequence tests"
	@echo "  make test-challenge     tactical puzzle tests"
	@echo "  make test-tournament    self-play tournament"
	@echo "  make clean              remove build artifacts"
