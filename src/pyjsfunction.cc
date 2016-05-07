#include "pyjsfunction.h"
#include "typeconv.h"
#include "debug.h"
#include "error.h"
#include <vector>

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
    std::vector<v8::Local<v8::Value>> argv(argc);
    for (ssize_t i = 0; i < argc; i++) {
        argv[i] = PyToJs(PyTuple_GetItem(self->obj, i));
    }
    v8::Local<v8::Function> func = Nan::New(self->savedFunction);
    Nan::TryCatch trycatch;
    v8::Local<v8::Value> result = func->Call(Nan::Undefined(), argc, argv.data());
    if (trycatch.HasCaught()) {
        makePyError(trycatch);
        self->obj = nullptr;
    } else self->obj = JsToPy(result).escape();
}

void functionCallCallback(uv_async_t *async) {
    JsFunction *self = (JsFunction *) async->data;
    uv_mutex_lock(&functionCallLock);
    performFunction(self);
    uv_cond_signal(&functionCallCond);
    uv_mutex_unlock(&functionCallLock);
}

void functionRefChangedCallback(uv_async_t *async) {
    LOG("function ref count changed to %lu\n", functionRefCount);
    if (functionRefCount == 0) {
        uv_unref((uv_handle_t *)&functionHandle);
    } else {
        uv_ref((uv_handle_t *)&functionHandle);
    }
}

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
        Py_BEGIN_ALLOW_THREADS
        uv_cond_wait(&functionCallCond, &functionCallLock);
        Py_END_ALLOW_THREADS
        uv_mutex_unlock(&functionCallLock);
    }
    uv_mutex_unlock(&functionHandleLock);
    // deal with returned value
    return self->obj;
}

PyTypeObject JsFunctionType; // static variable inits to 0

void JsFunction_Init() {
    JsFunctionType.ob_base = {PyObject_HEAD_INIT(NULL) 0};
    JsFunctionType.tp_name = "pyjs.JsFunction";
    JsFunctionType.tp_basicsize = sizeof(JsFunction);
    JsFunctionType.tp_dealloc = (destructor) JsFunction_dealloc;
    JsFunctionType.tp_flags = Py_TPFLAGS_DEFAULT;
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
