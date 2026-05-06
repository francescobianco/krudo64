#pragma once
#include "board.h"
#include "types.h"

void  uci_loop(void);
char *move_to_uci(Move m);
Move  uci_to_move(const Board *b, const char *str);
