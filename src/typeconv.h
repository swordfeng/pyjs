#pragma once
#include "common.h"
#include "python-util.h"
void TypeConvInit();
v8::Local<v8::Value> PyToJs(PyObjectBorrowed pyObject, bool implicit = true);
PyObjectWithRef JsToPy(v8::Local<v8::Value> jsValue);
