#include "pyjstypeconv.h"
#include "pyjsobject.h"
#include <cassert>
#include <iostream>
v8::Local<v8::Value> PyToJs(PyObject *pyObject) {
    Nan::EscapableHandleScope scope;
    if (pyObject == nullptr) {
        return scope.Escape(Nan::Undefined());
    } else if (pyObject == Py_None) {
        return scope.Escape(Nan::Null());
    } else if (PyUnicode_CheckExact(pyObject)) {
        Py_ssize_t size;
        char *str = PyUnicode_AsUTF8AndSize(pyObject, &size);
        return scope.Escape(Nan::New(str, size).ToLocalChecked());
    } else if (PyBytes_CheckExact(pyObject)) {
        char *buf;
        Py_ssize_t size;
        int result = PyBytes_AsStringAndSize(pyObject, &buf, &size);
        assert(result != -1);
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
    } else if (PyCallable_Check(pyObject)) {
        /*
        Py_INCREF(pyObject);
        v8::Local<v8::Object> jsObject = PyjsObject::NewInstance(pyObject);

        const int argc = 1;
        v8::Local<v8::Value> argv[] = { jsObject };
        v8::Local<v8::Function> makeFunction = Nan::New(PyjsObject::makeFunction);
        v8::Local<v8::Function> result = makeFunction->Call(jsObject, argc, argv).As<v8::Function>();

        return scope.Escape(result);
        */
        return scope.Escape(Nan::Undefined());
    }
    Py_INCREF(pyObject);
    return scope.Escape(PyjsObject::NewInstance(pyObject));
}

// return new reference
PyObject *JsToPy(v8::Local<v8::Value> jsValue) {
    Nan::HandleScope scope;
    if (jsValue->IsObject()) {
        v8::Local<v8::Object> jsObject = jsValue->ToObject();
        PyjsObject *object = PyjsObject::UnWrap(jsObject);
        if (object) {
            // just unwrap and return
            return object->GetObject();
        }
    }
    if (jsValue->IsNull()) {
        Py_INCREF(Py_None);
        return Py_None;
    } else if (jsValue->IsString()) {
        v8::Local<v8::String> jsString = jsValue->ToString();
        v8::String::Utf8Value utf8String(jsString);
        return PyUnicode_FromStringAndSize(*utf8String, jsString->Utf8Length());
    } else if (jsValue->IsTrue()) {
        Py_INCREF(Py_True);
        return Py_True;
    } else if (jsValue->IsFalse()) {
        Py_INCREF(Py_False);
        return Py_False;
    } else if (jsValue->IsNumber()) {
        return PyFloat_FromDouble(jsValue->NumberValue());
    } else if (jsValue->IsArray()) {
        v8::Local<v8::Array> jsArr = jsValue.As<v8::Array>();
        PyObject *pyArr = PyList_New(jsArr->Length());
        for (ssize_t i = 0; i < jsArr->Length(); i++) {
            int result = PyList_SetItem(pyArr, i, JsToPy(jsArr->Get(i)));
            assert(result != -1);
        }
        return pyArr;
    } else if (jsValue->IsFunction()) {
        assert(0);
    } else if (node::Buffer::HasInstance(jsValue)) { // compability?
        return PyBytes_FromStringAndSize(node::Buffer::Data(jsValue), node::Buffer::Length(jsValue));
    } else if (jsValue->IsObject()) { // must be after null, array, function, buffer
        v8::Local<v8::Object> jsObject = jsValue->ToObject();
        PyObject *pyDict = PyDict_New();
        v8::Local<v8::Array> props = Nan::GetOwnPropertyNames(jsObject).ToLocalChecked();
        for (ssize_t i = 0; i < props->Length(); i++) {
            v8::Local<v8::Value> jsKey = props->Get(i);
            v8::Local<v8::Value> jsValue = jsObject->Get(jsKey);
            PyObject *pyKey = JsToPy(jsKey);
            PyObject *pyValue = JsToPy(jsValue);
            int result = PyDict_SetItem(pyDict, pyKey, pyValue);
            assert(result != -1);
            Py_DECREF(pyKey);
            Py_DECREF(pyValue);
        }
        return pyDict;
    } else if (jsValue->IsUndefined()) {
        return nullptr;
    }
    assert(0); // should not reach here
}
