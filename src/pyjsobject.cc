#include "pyjsobject.h"
#include "pyjstypeconv.h"
#include <iostream>
#include <cassert>

Nan::Persistent<v8::FunctionTemplate> PyjsObject::constructorTpl;

PyjsObject::PyjsObject(): object(nullptr) {}
PyjsObject::~PyjsObject() { Py_XDECREF(object); }

// steal one ref
void PyjsObject::SetObject(PyObject *object) {
    Py_XDECREF(this->object);
    this->object = object;
}

// new reference
PyObject *PyjsObject::GetObject() {
    Py_XINCREF(object);
    return object;
}

PyjsObject *PyjsObject::UnWrap(v8::Local<v8::Object> object) {
    Nan::HandleScope scope;
    // find real `this`
    while (!IsInstance(object)) {
        v8::Local<v8::Value> prototypeValue = object->GetPrototype();
        if (prototypeValue->IsNull()) {
            // throw
            return nullptr;
        }
        object = prototypeValue->ToObject();
    }
    return ObjectWrap::Unwrap<PyjsObject>(object);
}

void PyjsObject::Init(v8::Local<v8::Object> exports) {
    Nan::HandleScope scope;

    // Prepare constructor template
    v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("PyObject").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);
    // Prototype
    v8::Local<v8::ObjectTemplate> prototpl = tpl->PrototypeTemplate();
    // Nan::SetMethod(prototpl, "print", Print);
    Nan::SetMethod(prototpl, "repr", Repr);
    Nan::SetMethod(prototpl, "value", Value);
    Nan::SetMethod(prototpl, "attr", Attr);

    constructorTpl.Reset(tpl);
    exports->Set(Nan::New("PyObject").ToLocalChecked(), tpl->GetFunction());
}

void PyjsObject::New(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    Nan::HandleScope scope;
    v8::Local<v8::Object> thisObject;
    if (!args.IsConstructCall()) {
        v8::Local<v8::Function> cons = Nan::New(constructorTpl)->GetFunction();
        thisObject = cons->NewInstance();
    } else {
        thisObject = args.This();
    }
    PyjsObject *wrapper = new PyjsObject();
    wrapper->Wrap(thisObject);
    if (args.Length() == 1) { // make a value
        wrapper->SetObject(JsToPy(args[0]));
    }
    args.GetReturnValue().Set(thisObject);
}

void PyjsObject::Value(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    Nan::HandleScope scope;
    PyjsObject *wrapper = UnWrap(args.This());
    PyObject *object = wrapper->object;
    args.GetReturnValue().Set(PyToJs(object));
}

void PyjsObject::Repr(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    PyjsObject *wrapper = UnWrap(args.This());
    Nan::HandleScope scope;
    args.GetReturnValue().Set(NewInstance(PyObject_Repr(wrapper->object)));
}

void PyjsObject::Attr(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    PyObject *object = UnWrap(args.This())->object;
    Nan::HandleScope scope;
    PyObject *attr = JsToPy(args[0]);
    if (args.Length() == 1) {
        PyObject *subObject = PyObject_GetAttr(object, attr);
        if (subObject) {
            args.GetReturnValue().Set(PyToJs(subObject));
            Py_DECREF(subObject);
        } else {
            args.GetReturnValue().Set(Nan::Undefined());
        }
    } else if (args.Length() == 2) {
        PyObject *value = JsToPy(args[1]);
        PyObject_SetAttr(object, attr, value);
        Py_DECREF(value);
    }
    Py_DECREF(attr);
}

// steal one ref
v8::Local<v8::Object> PyjsObject::NewInstance(PyObject *object) {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Function> cons = Nan::New<v8::FunctionTemplate>(constructorTpl)->GetFunction();
    v8::Local<v8::Object> instance = cons->NewInstance(0, {});
    PyjsObject *wrapper = ObjectWrap::Unwrap<PyjsObject>(instance);
    wrapper->SetObject(object);
    return scope.Escape(instance);
}

bool PyjsObject::IsInstance(v8::Local<v8::Object> object) {
    Nan::HandleScope scope;
    return Nan::New(constructorTpl)->HasInstance(object);
}
