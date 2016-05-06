#pragma once
#include <Python.h>

class GILStateHolder {
public:
    GILStateHolder(): gstate(PyGILState_Ensure()) {}
    ~GILStateHolder() { PyGILState_Release(gstate); }
private:
    PyGILState_STATE gstate;
};

typedef PyObject *PyObjectBorrowed;

class PyObjectWithRef { // PyObject with one reference
public:
    PyObjectWithRef(): _object(nullptr) {}
    explicit PyObjectWithRef(PyObject * const &object): _object(object) {} // steal one reference
    ~PyObjectWithRef() {
        GILStateHolder holder;
        Py_XDECREF(_object);
    }
    PyObjectWithRef(const PyObjectWithRef &objWithRef): _object(objWithRef._object) {
        Py_XINCREF(_object);
    }
    PyObjectWithRef(PyObjectWithRef &&objWithRef): _object(objWithRef._object) {
        objWithRef._object = nullptr;
    }
    PyObjectWithRef &operator=(const PyObjectWithRef &objWithRef) {
        GILStateHolder holder;
        Py_XDECREF(_object);
        _object = objWithRef._object;
        Py_XINCREF(_object);
        return *this;
    }
    PyObjectWithRef &operator=(PyObjectWithRef &&objWithRef) {
        GILStateHolder holder;
        Py_XDECREF(_object);
        _object = objWithRef._object;
        objWithRef._object = nullptr;
        return *this;
    }
    operator PyObjectBorrowed() { // borrow
        return _object;
    }
    PyObject *borrow() {
        return _object;
    }
    PyObject *escape() { // new reference
        PyObject *object = _object;
        _object = nullptr;
        return object;
    }
private:
    PyObject *_object;
};

static inline PyObjectWithRef PyObjectMakeRef(const PyObjectBorrowed &object) {
    GILStateHolder gilholder;
    Py_XINCREF(object);
    return PyObjectWithRef(object);
}
