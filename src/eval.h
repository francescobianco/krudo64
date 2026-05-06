#pragma once
#include "board.h"

typedef enum {
    EVAL_FEAT_MOBILITY,
    EVAL_FEAT_PAWN_STRUCT,
    EVAL_FEAT_CLUSTERS,
} EvalFeature;

/* Swap a feature function-pointer to its real impl (on=1) or zero-stub (on=0). */
void eval_set_feature(EvalFeature feat, int on);

int eval(const Board *b);
