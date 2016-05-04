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
    void SetObject(PyObject *object, v8::Local<v8::Object> instance); // steal reference
    PyObject *GetObject(); // return new reference

    static v8::Local<v8::Object> makeFunction(v8::Local<v8::Object> instance);
private:
    static void New(const Nan::FunctionCallbackInfo<v8::Value> &args);

    static void Repr(const Nan::FunctionCallbackInfo<v8::Value> &args);
    static void Str(const Nan::FunctionCallbackInfo<v8::Value> &args);

    static void ValueOf(const Nan::FunctionCallbackInfo<v8::Value> &args);

    static void Value(const Nan::FunctionCallbackInfo<v8::Value> &args);
    static void Attr(const Nan::FunctionCallbackInfo<v8::Value> &args);

    static void Call(const Nan::FunctionCallbackInfo<v8::Value> &args);
    static void CallFunction(const Nan::FunctionCallbackInfo<v8::Value> &args);

    static void AttrGetter(v8::Local<v8::String> name, const Nan::PropertyCallbackInfo<v8::Value> &info);
    static void AttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value, const Nan::PropertyCallbackInfo<v8::Value> &info);

    static Nan::Persistent<v8::FunctionTemplate> constructorTpl;
    static Nan::Persistent<v8::ObjectTemplate> callableTpl;

    PyObject *object;
};
