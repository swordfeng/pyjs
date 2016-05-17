#pragma once
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ASSERT(cond) \
    do { \
        if(!(cond)) { \
            fflush(stdout); \
            fprintf(stderr, "\33[1;31m"); \
            fprintf(stderr, "%s:%d: %s: Assertion `%s' failed.\n", __FILE__, __LINE__, __func__, #cond ); \
            fprintf(stderr, "\33[0m\n"); \
            exit(1); \
        } \
    } while(0)

#define LOG(...) \
    do { \
        fflush(stdout); \
        fprintf(stderr, "\33[1;32m"); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\33[0m\n"); \
    } while (0)

#ifdef __cplusplus
}
#endif
