#pragma once
#include <unistd.h>
#include <stdio.h>

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
