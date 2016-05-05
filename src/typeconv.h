#pragma once
#include <node.h>
#include <nan.h>
#include <Python.h>
#include "python-util.h"
v8::Local<v8::Value> PyToJs(PyObjectBorrowed pyObject, bool implicit = true);
PyObjectWithRef JsToPy(v8::Local<v8::Value> jsValue);
//v8::Local<v8::Value> PyTupleToJsArray(PyObjectBorrowed pyObject);
//PyObjectWithRef JsArrayToPyTuple(v8::Local<v8::Value> jsValue);
