#include <node.h>
#include <nan.h>
#include <Python.h>
#include <iostream>
#include <cassert>

#include "pyjsobject.h"
#include "pyjstypeconv.h"
#include "python-util.h"
#include "error.h"

void PyjsBuiltins(v8::Local<v8::String> name, const Nan::PropertyCallbackInfo<v8::Value> &args) {
    PyObjectWithRef object(PyImport_ImportModule("builtins"));
    args.GetReturnValue().Set(JsPyWrapper::NewInstance(object));
}

void PyjsImport(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    if (!args[0]->IsString()) return;
    PyObjectWithRef object(PyImport_ImportModule(*static_cast<v8::String::Utf8Value>(args[0]->ToString())));
    CHECK_PYTHON_ERROR;
    args.GetReturnValue().Set(JsPyWrapper::NewInstance(object));
}

void Init(v8::Local<v8::Object> exports) {
    // python initialize
    Py_Initialize();
    PyObjectBorrowed sysPath = PySys_GetObject("path");
    PyObjectWithRef path(PyUnicode_FromString(""));
    int result = PyList_Insert(sysPath, 0, path);
    assert(result != -1);
    JsPyWrapper::Init(exports);

    //Nan::SetMethod(exports, "eval", PyjsEval);
    Nan::SetAccessor(exports, Nan::New("builtins").ToLocalChecked(), PyjsBuiltins);
    Nan::SetMethod(exports, "import", PyjsImport);
}

NODE_MODULE(pyjs, Init)
