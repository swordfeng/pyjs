#include "error.h"
#include <frameobject.h>
#include <sstream>
#include <string>
#include <vector>
#include "typeconv.h"
#include "debug.h"

v8::Local<v8::Value> makeJsErrorObject() {
    PyObject *ptype, *pvalue, *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
    ASSERT(ptype);
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
        if (*lineCString != 0) stackStream << "    >>" << lineCString;
    }

    for (auto it = frames.rbegin(); it != frames.rend(); it++) {
        stackStream << "    at " << *it << std::endl;
    }

    stackStream << "    ==== FFI Boundary ====" << std::endl;

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
    Nan::SetPrototype(o, jsError->Get(Nan::New("prototype").ToLocalChecked())).ToChecked();
    ASSERT(!PyErr_Occurred());
    return scope.Escape(o);
}

void makePyError(Nan::TryCatch &trycatch) {
    //v8::Local<v8::String> message = trycatch.Message()->Get();
    //LOG("JS: %s\n", *Nan::Utf8String(message));
    v8::Local<v8::Value> e = trycatch.Exception();
    if (e->IsObject()) {
        v8::Local<v8::Object> exc = e->ToObject();
        v8::Local<v8::Value> name = exc->Get(Nan::New("name").ToLocalChecked());
        v8::Local<v8::Value> message = exc->Get(Nan::New("message").ToLocalChecked());
        v8::Local<v8::Value> stack = exc->Get(Nan::New("stack").ToLocalChecked());
        if (name->IsString()) {
            LOG("name: %s\n", *Nan::Utf8String(name));
        }
        if (message->IsString()) {
            LOG("message: %s\n", *Nan::Utf8String(message));
        }
        if (stack->IsString()) {
            LOG("stack: %s\n", *Nan::Utf8String(stack));
        }
        LOG("current:\n");
        PyErr_SetString(PyExc_Exception, *Nan::Utf8String(stack));
    } else {
        PyErr_SetString(PyExc_Exception, *Nan::Utf8String(e->ToString()));
    }
    trycatch.Reset();
}
