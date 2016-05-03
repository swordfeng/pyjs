#include <Python.h>

class PyObjectWithRef { // PyObject with one reference
public:
    PyObjectWithRef(const PyObject *&object): _object(object) {} // steal one reference
    ~PyObjectWithRef() { Py_XDECREF(_object); }
    PyObjectWithRef(const PyObjectWithRef &objWithRef): _object(objWithRef._object) {
        Py_XINCREF(_object);
    }
    PyObjectWithRef(PyObjectWithRef &&objWithRef): _object(objWithRef._object) {
        objWithRef._object = nullptr;
    }
    PyObjectWithRef &operator=(const PyObjectWithRef &objWithRef) {
        _object = objWithRef._object;
        Py_XINCREF(_object);
    }
    PyObjectWithRef &operator=(PyObjectWithRef &&objWithRef) {
        _object = objWithRef._object;
        objWithRef._object = nullptr;
    }
private:
    PyObject *_object;
}

PyObjectWithRef PyObjectMakeRef(const PyObject *&object) {
    Py_XINCREF(object);
    return PyObjectWithRef(object);
}
