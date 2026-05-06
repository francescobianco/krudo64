#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int _tests = 0, _passed = 0;

#define ASSERT(cond, ...) do { \
    _tests++; \
    if (cond) { \
        _passed++; \
    } else { \
        printf("FAIL %s:%d: " #cond "\n", __FILE__, __LINE__); \
    } \
} while(0)

#define TEST_SUMMARY() do { \
    printf("[%s] %d/%d passed\n", __FILE__, _passed, _tests); \
    return (_passed == _tests) ? 0 : 1; \
} while(0)
