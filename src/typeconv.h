#pragma once
#include <climits>
#include "common.h"
#include "python-util.h"
//#include "debug.h"
void TypeConvInit();
v8::Local<v8::Value> PyToJs(PyObjectBorrowed pyObject, bool implicit = true);
PyObjectWithRef JsToPy(v8::Local<v8::Value> jsValue);

static inline int ssize_cast(ssize_t size) {
    //ASSERT(size >= 0);
    //ASSERT(size <= INT_MAX);
    return static_cast<int>(size);
}
