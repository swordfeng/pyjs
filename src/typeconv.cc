#include "typeconv.h"
#include "jsobject.h"
#include "python-util.h"
#include "pyjsfunction.h"
#include "datetime.h"
#include "debug.h"

void TypeConvInit() {
    PyDateTime_IMPORT;
}

v8::Local<v8::Value> PyToJs(PyObjectBorrowed pyObject, bool implicit) {
    Nan::EscapableHandleScope scope;
    if (pyObject == nullptr) {
        return scope.Escape(Nan::Undefined());
    }
    if (implicit) {
        if (pyObject == Py_None) {
            return scope.Escape(Nan::Null());
        } else if (PyUnicode_CheckExact(pyObject)) {
            Py_ssize_t size;
            char *str = PyUnicode_AsUTF8AndSize(pyObject, &size);
            if (!str) return scope.Escape(Nan::Undefined());
            return scope.Escape(Nan::New(str, size).ToLocalChecked());
        } else if (PyBytes_CheckExact(pyObject)) {
            char *buf;
            Py_ssize_t size;
            PyBytes_AsStringAndSize(pyObject, &buf, &size);
            return scope.Escape(Nan::CopyBuffer(buf, size).ToLocalChecked());
        } else if (PyBool_Check(pyObject)) {
            return scope.Escape(Nan::New<v8::Boolean>(pyObject == Py_True));
        } else if (PyFloat_CheckExact(pyObject)) {
            return scope.Escape(Nan::New<v8::Number>(PyFloat_AsDouble(pyObject)));
        } else if (PyList_CheckExact(pyObject)) {
            v8::Local<v8::Array> jsArr = Nan::New<v8::Array>();
            Py_ssize_t size = PyList_Size(pyObject);
            ASSERT(size >= 0);
            for (ssize_t i = 0; i < size; i++) {
                jsArr->Set(i, PyToJs(PyList_GetItem(pyObject, i)));
            }
            return scope.Escape(jsArr);
        } else if (PyDict_CheckExact(pyObject)) {
            v8::Local<v8::Object> jsObject = Nan::New<v8::Object>();
            PyObject *key, *value;
            Py_ssize_t pos = 0;
            while (PyDict_Next(pyObject, &pos, &key, &value)) {
                jsObject->Set(PyToJs(key), PyToJs(value));
            }
            return scope.Escape(jsObject);
        } else if (PyObject_Type(pyObject) == (PyObject *)&JsPyModule::JsFunctionType) {
            return scope.Escape(JsPyModule::JsFunction_GetFunction(pyObject));
        } else if (PyDateTime_CheckExact(pyObject)) {
            PyObjectWithRef pyTsFunc(PyObject_GetAttrString(pyObject, "timestamp"));
            PyObjectWithRef pyTs(PyObject_CallObject(pyTsFunc, PyObjectWithRef(PyTuple_New(0))));
            double timestamp = PyFloat_AsDouble(pyTs);
            v8::Local<v8::Date> jsObject = Nan::New<v8::Date>(timestamp * 1000.0).ToLocalChecked();
            return scope.Escape(jsObject);
        }
    }
    if (PyCallable_Check(pyObject)) {
        v8::Local<v8::Object> jsObject = JsPyWrapper::NewInstance(PyObjectMakeRef(pyObject));
        return scope.Escape(JsPyWrapper::makeFunction(jsObject));
    } else {
        return scope.Escape(JsPyWrapper::NewInstance(PyObjectMakeRef(pyObject)));
    }
}

PyObjectWithRef JsToPy(v8::Local<v8::Value> jsValue) {
    Nan::HandleScope scope;
    if (jsValue->IsObject()) {
        v8::Local<v8::Object> jsObject = jsValue->ToObject();
        JsPyWrapper *object = JsPyWrapper::UnWrap(jsObject);
        if (object) { // found wrapper python object
            return object->GetObject();
        }
    }
    if (jsValue->IsNull()) {
        return PyObjectMakeRef(Py_None);
    } else if (jsValue->IsString()) {
        v8::Local<v8::String> jsString = jsValue->ToString();
        return PyObjectWithRef(PyUnicode_FromStringAndSize(*Nan::Utf8String(jsString), jsString->Utf8Length()));
    } else if (jsValue->IsTrue()) {
        return PyObjectMakeRef(Py_True);
    } else if (jsValue->IsFalse()) {
        return PyObjectMakeRef(Py_False);
    } else if (jsValue->IsNumber()) {
        return PyObjectWithRef(PyFloat_FromDouble(jsValue->NumberValue()));
    } else if (jsValue->IsArray()) {
        v8::Local<v8::Array> jsArr = jsValue.As<v8::Array>();
        PyObjectWithRef pyArr = PyObjectWithRef(PyList_New(jsArr->Length()));
        for (ssize_t i = 0; i < jsArr->Length(); i++) {
            int result = PyList_SetItem(pyArr, i, JsToPy(jsArr->Get(i)).escape());
            ASSERT(result != -1);
        }
        return pyArr;
    } else if (jsValue->IsFunction()) {
        return PyObjectWithRef(JsPyModule::JsFunction_NewFunction(jsValue.As<v8::Function>()));
    } else if (node::Buffer::HasInstance(jsValue)) { // compability?
        return PyObjectWithRef(PyBytes_FromStringAndSize(node::Buffer::Data(jsValue), node::Buffer::Length(jsValue)));
    } else if (jsValue->IsDate()) {
        double timestamp = jsValue.As<v8::Date>()->ValueOf() / 1000.0;
        PyObjectWithRef args(PyTuple_New(1));
        PyTuple_SetItem(args, 0, PyFloat_FromDouble(timestamp));
        PyObjectWithRef pyDateTime(PyDateTime_FromTimestamp(args));
        return pyDateTime;
    } else if (jsValue->IsObject()) { // must be after null, array, function, buffer, date
        v8::Local<v8::Object> jsObject = jsValue->ToObject();
        PyObjectWithRef pyDict = PyObjectWithRef(PyDict_New());
        v8::Local<v8::Array> props = Nan::GetOwnPropertyNames(jsObject).ToLocalChecked();
        for (ssize_t i = 0; i < props->Length(); i++) {
            v8::Local<v8::Value> jsKey = props->Get(i);
            v8::Local<v8::Value> jsValue = jsObject->Get(jsKey);
            int result = PyDict_SetItem(pyDict, JsToPy(jsKey), JsToPy(jsValue));
            ASSERT(result != -1);
        }
        return pyDict;
    } else if (jsValue->IsUndefined()) {
        //return PyObjectWithRef();
        return PyObjectMakeRef(Py_None); // avoid some crashes... but this is not expected however, who knows why?
    }
    ASSERT(0); // should not reach here
}

v8::Local<v8::Value> PyTupleToJsArray(PyObjectBorrowed pyObject) {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Array> arr = Nan::New<v8::Array>();
    ssize_t size = PyTuple_Size(pyObject);
    ASSERT(size > 0);
    for (ssize_t i = 0; i < size; i++) {
        arr->Set(i, PyToJs(PyTuple_GetItem(pyObject, i)));
    }
    return scope.Escape(arr);
}

PyObjectWithRef JsArrayToPyTuple(v8::Local<v8::Value> jsValue) {
    Nan::HandleScope scope;
    v8::Local<v8::Array> arr = jsValue.As<v8::Array>();
    ssize_t size = arr->Length();
    PyObjectWithRef tup(PyTuple_New(size));
    for (ssize_t i = 0; i < size; i++) {
        PyTuple_SetItem(tup, i, JsToPy(arr->Get(i)).escape());
    }
    return tup;
}
