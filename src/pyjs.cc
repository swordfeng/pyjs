#include <node.h>
#include <nan.h>
#include <Python.h>
#include <iostream>
#include <cassert>

#include "pyjsobject.h"
#include "pyjstypeconv.h"

using namespace Nan;
using v8::Value;
using v8::Local;
using v8::Object;
using v8::FunctionTemplate;
using v8::String;

void PyjsEval(const FunctionCallbackInfo<Value>& args) {
    Local<String> script = args[0]->ToString();
    PyObject *dict = PyDict_New();
    PyObject *object = PyRun_String(*static_cast<String::Utf8Value>(script), Py_single_input, dict, dict);
    Py_DECREF(dict);
    if (object == nullptr) {
        // exception
        PyErr_Print();
        PyErr_Clear();
    }
    args.GetReturnValue().Set(PyjsObject::NewInstance(object));
}

void PyjsBuiltins(const FunctionCallbackInfo<Value> &args) {
    PyObject *builtins = PyEval_GetBuiltins();
    assert(builtins);
    args.GetReturnValue().Set(PyToJs(builtins));
}

void PyjsImport(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    Local<String> name = args[0]->ToString();
    PyObject *object = PyImport_ImportModule(*static_cast<String::Utf8Value>(name));
    args.GetReturnValue().Set(PyjsObject::NewInstance(object));
}

void Init(Local<Object> exports) {
    // python initialize
    Py_Initialize();
    /*
    PyObject *sysPath = PySys_GetObject("path");
    PyObject *path = PyUnicode_FromString("");
    int result = PyList_Insert(sysPath, 0, path);
    assert(result != -1);
    Py_DECREF(path);
    */
    PyjsObject::Init(exports);

    Nan::SetMethod(exports, "eval", PyjsEval);
    Nan::SetMethod(exports, "builtins", PyjsBuiltins);
    Nan::SetMethod(exports, "import", PyjsImport);
}

NODE_MODULE(pyjs, Init)
