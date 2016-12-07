#pragma once

#include "common.h"
#include <uv.h>

namespace JsPyModule {
typedef struct {
    PyObject_HEAD
    Nan::Persistent<v8::Function> savedFunction;
    PyObject *obj;
} JsFunction;
void JsFunction_Init();
PyObject *JsFunction_NewFunction(v8::Local<v8::Function> func);
v8::Local<v8::Function> JsFunction_GetFunction(PyObject *object);
void JsFunction_SetFunction(JsFunction *self, v8::Local<v8::Function> func);
extern PyTypeObject JsFunctionType;
} // namespace JsPyModule
