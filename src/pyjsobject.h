#pragma once
#include <node.h>
#include <nan.h>
#include <Python.h>

class PyjsObject : public Nan::ObjectWrap {
public:
    static void Init(v8::Local<v8::Object> exports);
    static v8::Local<v8::Object> NewInstance(PyObject *object); // steal reference
    static bool IsInstance(v8::Local<v8::Object> object);
    static PyjsObject *UnWrap(v8::Local<v8::Object> object);
    PyjsObject();
    ~PyjsObject();
    void SetObject(PyObject *object); // steal reference
    PyObject *GetObject(); // return new reference
private:
    static void New(const Nan::FunctionCallbackInfo<v8::Value> &args);

    static void Repr(const Nan::FunctionCallbackInfo<v8::Value> &args);
    static void Value(const Nan::FunctionCallbackInfo<v8::Value> &args);
    static void Attr(const Nan::FunctionCallbackInfo<v8::Value> &args);

    static Nan::Persistent<v8::FunctionTemplate> constructorTpl;

    PyObject *object;
};
