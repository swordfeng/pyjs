#include <Python.h>
#include <node.h>
#include <nan.h>
#include <uv.h>

namespace JsPyModule {

static uv_async_t functionHandle;
static uv_mutex_t functionHandleLock;
static uv_mutex_t functionCallLock;
static uv_cond_t functionCallCond;

typedef struct {
    PyObject_HEAD
    Nan::Persistent<v8::Function> savedFunction;
} JsFunction;

static void JsFunction_dealloc(JsFunction *self) {
    self->savedFunction.Reset();
}

static PyObject *JsFunction_call(JsFuncion *self, PyObject *args) {
    //
}

static PyMethodDef JsFunction_methods[] = {
    {"__call__", (PyCFunction)JsFunction_call, METH_VARARGS,
     "Return the name, combining the first and last name"
    },
    {NULL}  /* Sentinel */
};

static PyTypeObject JsFunctionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "pyjs.JsFunction",                                      /* tp_name */
    sizeof(JsFunction),                                     /* tp_basicsize */
    0,                                                      /* tp_itemsize */
    (destructor)JsFunction_dealloc,                         /* tp_dealloc */
    0,                                                      /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "Javascript functions",           /* tp_doc */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)0,      /* tp_init */
    0,                         /* tp_alloc */
    0,                 /* tp_new */
};

} // namespace JsPyModule
