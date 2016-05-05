#include "pyjstypeconv.h"
#include "jsobject.h"
#include "python-util.h"
#include <cassert>
#include <iostream>

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
        if (object) {
            // just unwrap and return
            return object->GetObject();
        }
    }
    if (jsValue->IsNull()) {
        return PyObjectMakeRef(Py_None);
    } else if (jsValue->IsString()) {
        v8::Local<v8::String> jsString = jsValue->ToString();
        v8::String::Utf8Value utf8String(jsString);
        return PyObjectWithRef(PyUnicode_FromStringAndSize(*utf8String, jsString->Utf8Length()));
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
            assert(result != -1);
        }
        return pyArr;
    } else if (jsValue->IsFunction()) {
        assert(0);
    } else if (node::Buffer::HasInstance(jsValue)) { // compability?
        return PyObjectWithRef(PyBytes_FromStringAndSize(node::Buffer::Data(jsValue), node::Buffer::Length(jsValue)));
    } else if (jsValue->IsObject()) { // must be after null, array, function, buffer
        v8::Local<v8::Object> jsObject = jsValue->ToObject();
        PyObjectWithRef pyDict = PyObjectWithRef(PyDict_New());
        v8::Local<v8::Array> props = Nan::GetOwnPropertyNames(jsObject).ToLocalChecked();
        for (ssize_t i = 0; i < props->Length(); i++) {
            v8::Local<v8::Value> jsKey = props->Get(i);
            v8::Local<v8::Value> jsValue = jsObject->Get(jsKey);
            int result = PyDict_SetItem(pyDict, JsToPy(jsKey), JsToPy(jsValue));
            assert(result != -1);
        }
        return pyDict;
    } else if (jsValue->IsUndefined()) {
        //return PyObjectWithRef();
        return PyObjectMakeRef(Py_None); // avoid some crashes... but this is not expected however, who knows why?
    }
    assert(0); // should not reach here
}
