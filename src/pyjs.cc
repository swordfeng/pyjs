#include <node.h>
#include <nan.h>
#include <Python.h>
#include <iostream>
#include <cassert>

#include "pyjsobject.h"
#include "pyjstypeconv.h"
#include "python-util.h"

using namespace Nan;
using v8::Value;
using v8::Local;
using v8::Object;
using v8::FunctionTemplate;
using v8::String;

void PyjsEval(const FunctionCallbackInfo<Value>& args) {
    Local<String> script = args[0]->ToString();
    PyObjectWithRef dict(PyDict_New());
    PyObjectWithRef object(PyRun_String(*static_cast<String::Utf8Value>(script), Py_single_input, dict, dict));
    if (object == nullptr) {
        // exception
        PyErr_Print();
        PyErr_Clear();
    }
    args.GetReturnValue().Set(PyjsObject::NewInstance(object));
}

void PyjsBuiltins(const FunctionCallbackInfo<Value> &args) {
    PyObjectBorrowed builtins = PyEval_GetBuiltins();
    assert(builtins);
    args.GetReturnValue().Set(PyToJs(builtins));
}

void PyjsImport(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    Local<String> name = args[0]->ToString();
    PyObjectWithRef object(PyImport_ImportModule(*static_cast<String::Utf8Value>(name)));
    args.GetReturnValue().Set(PyjsObject::NewInstance(object));
}

void Init(Local<Object> exports) {
    // python initialize
    Py_Initialize();
    PyObjectBorrowed sysPath = PySys_GetObject("path");
    PyObjectWithRef path(PyUnicode_FromString(""));
    int result = PyList_Insert(sysPath, 0, path);
    assert(result != -1);
    PyjsObject::Init(exports);

    Nan::SetMethod(exports, "eval", PyjsEval);
    Nan::SetMethod(exports, "builtins", PyjsBuiltins);
    Nan::SetMethod(exports, "import", PyjsImport);
}

NODE_MODULE(pyjs, Init)
