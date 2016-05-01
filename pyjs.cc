#include <node.h>
#include <nan.h>
#include <Python.h>

namespace pyjs {
    using namespace Nan;
    using v8::Value;
    using v8::Local;
    using v8::Object;
    using v8::FunctionTemplate;
    using v8::String;

    void eval(const FunctionCallbackInfo<Value>& args) {
        Local<String> script = args[0]->ToString();
        PyRun_SimpleString(static_cast<const char *>(*static_cast<String::Utf8Value>(script)));
    }

    void init(Local<Object> exports) {
        Py_Initialize();
        // add current directory to path
        PyObject *sysPath = PySys_GetObject("path");
        PyObject *path = PyUnicode_FromString("");
        int result = PyList_Insert(sysPath, 0, path);
        exports->Set(Nan::New("eval").ToLocalChecked(),
                Nan::New<FunctionTemplate>(eval)->GetFunction());
    }

    NODE_MODULE(pyjs, init)

}  // namespace pyjs 
