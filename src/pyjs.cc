#include <node.h>
#include <nan.h>
#include <Python.h>
#include <iostream>
#include <functional>
#include <memory>
#include <cassert>

#include "jsobject.h"
#include "typeconv.h"
#include "python-util.h"
#include "error.h"
#include "gil-lock.h"

#include "pymodule.h"


class AtExit {
public:
    AtExit(std::function<void (void)> atExit): atExit(atExit) {}
    ~AtExit() { atExit(); }
private:
    std::function<void (void)> atExit;
};

void Builtins(v8::Local<v8::String> name, const Nan::PropertyCallbackInfo<v8::Value> &args) {
    Nan::HandleScope scope;
    PyObjectWithRef object(PyImport_ImportModule("builtins"));
    args.GetReturnValue().Set(JsPyWrapper::NewInstance(object));
}

void Import(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    Nan::HandleScope scope;
    if (args.Length() == 0 || !args[0]->IsString()) return Nan::ThrowTypeError("invalid module name");
    PyObjectWithRef object(PyImport_ImportModule(*static_cast<v8::String::Utf8Value>(args[0]->ToString())));
    CHECK_PYTHON_ERROR;
    args.GetReturnValue().Set(JsPyWrapper::NewInstance(object));
}

void Eval(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    Nan::HandleScope scope;
    if (args.Length() == 0 || !args[0]->IsString()) return Nan::ThrowTypeError("invalid Python code");
    PyThreadState *threadState = PyThreadState_Get();

    PyObjectWithRef global(PyDict_New()), local(PyDict_New());
    PyObjectWithRef object(PyRun_String(*Nan::Utf8String(args[0]), Py_eval_input, global, local));
    CHECK_PYTHON_ERROR;
    args.GetReturnValue().Set(PyToJs(object, JsPyWrapper::implicitConversionEnabled));
}

void Module(v8::Local<v8::String> name, const Nan::PropertyCallbackInfo<v8::Value> &args) {
    Nan::HandleScope scope;
    PyObjectWithRef object = PyObjectMakeRef(JsPyModule::GetModule());
    args.GetReturnValue().Set(JsPyWrapper::NewInstance(object));
}

void Init(v8::Local<v8::Object> exports) {

    // find process.argv first
    v8::Local<v8::Array> jsArgv = Nan::GetCurrentContext()->Global()
        ->Get(Nan::New("process").ToLocalChecked())->ToObject()
        ->Get(Nan::New("argv").ToLocalChecked()).As<v8::Array>();

    if (jsArgv->Length() > 0) {
        wchar_t *name = Py_DecodeLocale(*Nan::Utf8String(jsArgv->Get(0)->ToString()), nullptr);
        Py_SetProgramName(name);
        PyMem_RawFree(name);
    }

    // python initialize
    Py_Initialize();
    // not working?
    //node::AtExit([] (void *) { Py_Finalize(); std::cout << "exit" << std::endl; });
    static AtExit exitHandler(Py_Finalize);

    GILLock::Init();

    JsPyWrapper::Init(exports);

    // init sys.argv
    int argc = jsArgv->Length();
    argc && --argc;
    std::unique_ptr<wchar_t *[]> argv(new wchar_t *[argc]);
    for (int i = 0; i < argc; i++) {
        argv[i] = Py_DecodeLocale(*Nan::Utf8String(jsArgv->Get(i + 1)->ToString()), nullptr);
    }
    PySys_SetArgv(argc, argv.get());
    for (int i = 0; i < argc; i++) {
        PyMem_RawFree(argv[i]);
    }

    // py module init
    JsPyModule::JsFunction_Init();

    Nan::SetAccessor(exports, Nan::New("module").ToLocalChecked(), Module);
    Nan::SetAccessor(exports, Nan::New("builtins").ToLocalChecked(), Builtins);
    Nan::SetMethod(exports, "import", Import);
    Nan::SetMethod(exports, "eval", Eval);
}

NODE_MODULE(pyjs, Init)
