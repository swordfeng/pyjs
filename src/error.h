#pragma once
#include <Python.h>
#include <frameobject.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <node.h>
#include <nan.h>
#include "pyjstypeconv.h"

// May return!
#define CHECK_PYTHON_ERROR \
    if (PyErr_Occurred()) { \
        return Nan::ThrowError(makeJsErrorObject()); \
    }

static inline v8::Local<v8::Value> makeJsErrorObject() {
    PyObject *ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
    PyObjectWithRef type(ptype), value(pvalue), traceback(ptraceback);

    std::ostringstream stackStream;

    // name and message
    PyObjectWithRef errName(PyObject_GetAttrString(type, "__name__"));
    PyObjectWithRef errMessage(PyObject_Str(value));
    stackStream << PyUnicode_AsUTF8(errName) << ": " << PyUnicode_AsUTF8(errMessage) << std::endl;

    // python stack
    std::vector<std::string> frames;
    PyTracebackObject *last = nullptr, *tb = reinterpret_cast<PyTracebackObject *>(traceback.borrow());
    while (tb != nullptr) {
        last = tb;
        std::ostringstream frameStringStream;
        frameStringStream << PyUnicode_AsUTF8(tb->tb_frame->f_code->co_name)
                << " (" << PyUnicode_AsUTF8(tb->tb_frame->f_code->co_filename)
                << ":" << tb->tb_lineno << ")";
        frames.push_back(frameStringStream.str());
        tb = tb->tb_next;
    }

    if (last) {
        // print line
        PyObjectWithRef linecache(PyImport_ImportModule("linecache"));
        PyObjectWithRef getline(PyObject_GetAttrString(linecache, "getline"));
        PyObjectWithRef args(PyTuple_New(2));
        PyTuple_SetItem(args, 0, PyObjectMakeRef(last->tb_frame->f_code->co_filename).escape());
        PyTuple_SetItem(args, 1, PyLong_FromLong(last->tb_lineno));
        PyObjectWithRef line(PyObject_CallObject(getline, args));
        const char *lineCString = PyUnicode_AsUTF8(line);
        if (*lineCString != 0) stackStream << "      " << lineCString << std::endl;
    }

    for (auto it = frames.rbegin(); it != frames.rend(); it++) {
        stackStream << "    at " << *it << std::endl;
    }

    stackStream << "  ---- FFI Boundary ----" << std::endl;

    // js stack
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Object> o = Nan::New<v8::Object>();
    v8::Local<v8::Object> jsError = Nan::GetCurrentContext()->Global()
        ->Get(Nan::New("Error").ToLocalChecked())->ToObject();
    v8::Local<v8::Function> captureStackTrace = jsError
        ->Get(Nan::New("captureStackTrace").ToLocalChecked()).As<v8::Function>();
    const int argc = 1;
    v8::Local<v8::Value> argv[argc] = { o };
    captureStackTrace->Call(Nan::Undefined(), argc, argv);
    std::string jsStackString(*Nan::Utf8String(o->Get(Nan::New("stack").ToLocalChecked())));
    size_t firstNewLine = jsStackString.find('\n');
    if (firstNewLine != std::string::npos && firstNewLine != jsStackString.size()) {
        stackStream << jsStackString.substr(firstNewLine + 1);
    }

    // make final Error object
    o->Set(Nan::New("stack").ToLocalChecked(), Nan::New(stackStream.str().c_str()).ToLocalChecked());
    o->Set(Nan::New("name").ToLocalChecked(), PyToJs(errName));
    o->Set(Nan::New("message").ToLocalChecked(), PyToJs(errMessage));
    o->SetPrototype(jsError->Get(Nan::New("prototype").ToLocalChecked()));

    return scope.Escape(o);
}
