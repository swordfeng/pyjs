#pragma once
#include <unistd.h>
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
            _exit(1); \
        } \
    } while(0)

#define LOG(...) \
    do {} while (0)

#ifdef __cplusplus
}
#endif
