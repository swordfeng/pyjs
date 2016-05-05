#include "pyjsfunction.h"
#include "typeconv.h"
#include <iostream>

namespace JsPyModule {

static size_t functionRefCount = 0;
static uv_async_t functionRefChanged;
static uv_async_t functionHandle;
static uv_mutex_t functionHandleLock;
static uv_mutex_t functionCallLock;
static uv_cond_t functionCallCond;

static PyThreadState *mainThread;

void performFunction(JsFunction *self) {
    Nan::HandleScope scope;
    ssize_t argc = PyTuple_Size(self->obj);
    v8::Local<v8::Value> argv[argc];
    for (ssize_t i = 0; i < argc; i++) {
        argv[i] = PyToJs(PyTuple_GetItem(self->obj, i));
    }
    v8::Local<v8::Function> func = Nan::New(self->savedFunction);
    Nan::TryCatch trycatch;
    v8::Local<v8::Value> result = func->Call(Nan::Undefined(), argc, argv);
    self->obj = JsToPy(result).escape();
}

void functionCallCallback(uv_async_t *async) {
    JsFunction *self = (JsFunction *) async->data;
    uv_mutex_lock(&functionCallLock);
    performFunction(self);
    uv_cond_signal(&functionCallCond);
}

void functionRefChangedCallback(uv_async_t *async) {
    std::cout << "function ref count changed to " << functionRefCount << std::endl;
    if (functionRefCount == 0) {
        uv_unref((uv_handle_t *)&functionHandle);
    } else {
        uv_ref((uv_handle_t *)&functionHandle);
    }
}
/*
static PyObject *JsFunction_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    JsFunction *self;
    self = (JsFunction *)type->tp_alloc(type, 0);
    functionRefCount++;
    uv_async_send(&functionRefChanged);
    return (PyObject *)self;
}
*/
PyObject *JsFunction_NewFunction(v8::Local<v8::Function> func) {
    JsFunction *self;
    self = (JsFunction *)JsFunctionType.tp_alloc(&JsFunctionType, 0);
    self->savedFunction.Reset(func);
    functionRefCount++;
    uv_async_send(&functionRefChanged);
    return (PyObject *)self;
}

v8::Local<v8::Function> JsFunction_GetFunction(PyObject *object) {
    Nan::EscapableHandleScope scope;
    JsFunction *self = (JsFunction *) object;
    return scope.Escape(Nan::New(self->savedFunction));
}

static void JsFunction_dealloc(JsFunction *self) {
    self->savedFunction.Reset();
    functionRefCount--;
    uv_async_send(&functionRefChanged);
}

void JsFunction_SetFunction(JsFunction *self, v8::Local<v8::Function> func) {
    self->savedFunction.Reset(func);
}

static PyObject *JsFunction_call(PyObject *obj, PyObject *args, PyObject *kw) {
    JsFunction *self = (JsFunction *)obj;
    uv_mutex_lock(&functionHandleLock);
    self->obj = args;
    if (PyThreadState_Get() == mainThread) {
        performFunction(self);
    } else {
        uv_mutex_lock(&functionCallLock);
        functionHandle.data = self;
        uv_async_send(&functionHandle);
        uv_cond_wait(&functionCallCond, &functionCallLock);
        uv_mutex_unlock(&functionCallLock);
    }
    // deal with returned value
    return self->obj;
}

static PyMethodDef JsFunction_methods[] = {
    /*{"__call__", (PyCFunction)JsFunction_call, METH_VARARGS,
     "call javascript function"
 },*/
    {NULL}  /* Sentinel */
};

PyTypeObject JsFunctionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyjs.JsFunction",                                      /* tp_name */
    sizeof(JsFunction)};

void JsFunction_Init() {
    JsFunctionType.tp_dealloc = (destructor) JsFunction_dealloc;
    JsFunctionType.tp_flags = Py_TPFLAGS_DEFAULT;
    //JsFunctionType.tp_methods = JsFunction_methods;
    //JsFunctionType.tp_new = JsFunction_new;
    JsFunctionType.tp_call = JsFunction_call;

    mainThread = PyThreadState_Get();
    uv_async_init(uv_default_loop(), &functionHandle, functionCallCallback);
    uv_async_init(uv_default_loop(), &functionRefChanged, functionRefChangedCallback);
    uv_unref((uv_handle_t *)&functionRefChanged);
    uv_async_send(&functionRefChanged);
    uv_mutex_init(&functionHandleLock);
    uv_mutex_init(&functionCallLock);
    uv_cond_init(&functionCallCond);
}

} // namespace JsPyModule
