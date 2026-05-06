#pragma once
#include "types.h"
#include "board.h"

int movegen_pseudo(const Board *b, Move *moves);
int movegen_legal(const Board *b, Move *moves);
int movegen_captures(const Board *b, Move *moves);
