#pragma once
#include <node.h>
#include <nan.h>
#include <Python.h>
v8::Local<v8::Value> PyToJs(PyObject *pyObject);
PyObject *JsToPy(v8::Local<v8::Value> jsValue); // new reference
