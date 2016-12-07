#pragma once
#include "common.h"

// May return!
#define CHECK_PYTHON_ERROR \
    if (PyErr_Occurred()) { \
        return Nan::ThrowError(makeJsErrorObject()); \
    }

v8::Local<v8::Value> makeJsErrorObject();
void makePyError(Nan::TryCatch &trycatch);
