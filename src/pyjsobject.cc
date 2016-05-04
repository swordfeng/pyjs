#include "pyjsobject.h"
#include "pyjstypeconv.h"
#include <iostream>
#include <cassert>

Nan::Persistent<v8::FunctionTemplate> PyjsObject::constructorTpl;
Nan::Persistent<v8::ObjectTemplate> PyjsObject::callableTpl;

void PyjsObject::SetObject(PyObjectWithRef object, v8::Local<v8::Object> instance) {
    this->object = std::move(object);
}

PyObjectWithRef PyjsObject::GetObject() {
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
    Nan::SetMethod(prototpl, "$repr", Repr);
    Nan::SetMethod(prototpl, "$str", Str);
    Nan::SetMethod(prototpl, "$call", Call);
    Nan::SetMethod(prototpl, "$value", Value);
    Nan::SetMethod(prototpl, "$attr", Attr);
    Nan::SetMethod(prototpl, "$", Attr);

    Nan::SetMethod(prototpl, "toString", Str);
    Nan::SetMethod(prototpl, "inspect", Repr);
    Nan::SetMethod(prototpl, "valueOf", ValueOf);

    Nan::SetNamedPropertyHandler(tpl->InstanceTemplate(), AttrGetter, AttrSetter, 0, 0, AttrEnumerator);

    constructorTpl.Reset(tpl);
    exports->Set(Nan::New("PyObject").ToLocalChecked(), tpl->GetFunction());

    v8::Local<v8::ObjectTemplate> ctpl = Nan::New<v8::ObjectTemplate>();
    Nan::SetNamedPropertyHandler(ctpl, AttrGetter, AttrSetter, 0, 0, AttrEnumerator);
    Nan::SetCallAsFunctionHandler(ctpl, CallFunction);
    callableTpl.Reset(ctpl);
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
    if (args.Length() == 1) {
        wrapper->SetObject(JsToPy(args[0]), thisObject);
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
    PyObject *repr = PyObject_Repr(wrapper->object);
    args.GetReturnValue().Set(PyToJs(repr));
    Py_DECREF(repr);
}

void PyjsObject::Str(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    PyjsObject *wrapper = UnWrap(args.This());
    Nan::HandleScope scope;
    PyObject *repr = PyObject_Str(wrapper->object);
    args.GetReturnValue().Set(PyToJs(repr));
    Py_DECREF(repr);
}

void PyjsObject::ValueOf(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    PyObjectBorrowed object = UnWrap(args.This())->object;
    Nan::HandleScope scope;
    if (PyLong_CheckExact(object)) {
        double value = PyLong_AsDouble(object);
        args.GetReturnValue().Set(Nan::New<v8::Number>(value));
    } else {
        v8::Local<v8::Value> instance = PyToJs(object);
        args.GetReturnValue().Set(instance);
    }
}

void PyjsObject::Attr(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    PyObjectBorrowed object = UnWrap(args.This())->object;
    Nan::HandleScope scope;
    PyObjectWithRef attr(JsToPy(args[0]));
    if (args.Length() == 1) {
        PyObjectWithRef subObject(PyObject_GetAttr(object, attr));
        if (subObject) {
            args.GetReturnValue().Set(PyToJs(subObject));
        } else {
            args.GetReturnValue().Set(Nan::Undefined());
        }
    } else if (args.Length() == 2) {
        PyObject_SetAttr(object, attr, JsToPy(args[1]));
    }
}

void PyjsObject::AttrGetter(v8::Local<v8::String> name, const Nan::PropertyCallbackInfo<v8::Value> &info) {
    if (!name->IsString()) return;
    PyjsObject *wrapper = UnWrap(info.This());
    PyObjectBorrowed object = wrapper->object;
    if (!object) return;
    PyObjectWithRef attr = JsToPy(name);
    if (!PyObject_HasAttr(object, attr)) return;
    PyObjectWithRef subObject(PyObject_GetAttr(object, attr));
    info.GetReturnValue().Set(PyToJs(subObject));
}

void PyjsObject::AttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value,
    const Nan::PropertyCallbackInfo<v8::Value> &info) {
    if (!name->IsString()) return;
    if (!IsInstance(info.This())) return;
    PyjsObject *wrapper = UnWrap(info.This());
    if (!wrapper) return;
    PyObject *object = wrapper->object;
    if (!object) return;
    int result = PyObject_SetAttr(object, JsToPy(name), JsToPy(value));
}

void PyjsObject::AttrEnumerator(const Nan::PropertyCallbackInfo<v8::Array> &info) {
    PyjsObject *wrapper = UnWrap(info.This());
    PyObject *object = wrapper->object;
    if (!object) return;
    PyObject *pyAttrs = PyObject_Dir(object);
    info.GetReturnValue().Set(PyToJs(pyAttrs).As<v8::Array>());
    Py_DECREF(pyAttrs);
}

void PyjsObject::Call(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    PyObject *pyFunc = UnWrap(args.This())->object;
    if (!PyCallable_Check(pyFunc)) {
        // throw
        return;
    }
    Nan::HandleScope scope;
    if (args[0]->IsArray()) {
        // arguments
        v8::Local<v8::Array> jsArr = args[0].As<v8::Array>();
        PyObject *pyTuple = PyTuple_New(jsArr->Length());
        for (ssize_t i = 0; i < jsArr->Length(); i++) {
            int result = PyTuple_SetItem(pyTuple, i, JsToPy(jsArr->Get(i)).escape());
            assert(result != -1);
        }

        PyObject *pyResult = PyObject_CallObject(pyFunc, pyTuple);

        args.GetReturnValue().Set(PyToJs(pyResult));
        Py_XDECREF(pyResult);
        Py_DECREF(pyTuple);
    }
}

void PyjsObject::CallFunction(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    PyObjectBorrowed pyFunc = UnWrap(args.This())->object;
    if (!PyCallable_Check(pyFunc)) {
        // throw
        return;
    }
    Nan::HandleScope scope;
    // arguments
    PyObject *pyTuple = PyTuple_New(args.Length());
    for (ssize_t i = 0; i < args.Length(); i++) {
        int result = PyTuple_SetItem(pyTuple, i, JsToPy(args[i]).escape());
        assert(result != -1);
    }

    PyObject *pyResult = PyObject_CallObject(pyFunc, pyTuple);

    args.GetReturnValue().Set(PyToJs(pyResult));
    Py_XDECREF(pyResult);
    Py_DECREF(pyTuple);
}

v8::Local<v8::Object> PyjsObject::NewInstance(PyObjectWithRef object) {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Function> cons = Nan::New<v8::FunctionTemplate>(constructorTpl)->GetFunction();
    v8::Local<v8::Object> instance = cons->NewInstance(0, {});
    PyjsObject *wrapper = UnWrap(instance);
    wrapper->SetObject(object, instance);
    return scope.Escape(instance);
}

bool PyjsObject::IsInstance(v8::Local<v8::Object> object) {
    Nan::HandleScope scope;
    return Nan::New(constructorTpl)->HasInstance(object);
}

v8::Local<v8::Object> PyjsObject::makeFunction(v8::Local<v8::Object> instance) {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::ObjectTemplate> ctpl = Nan::New(callableTpl);
    v8::Local<v8::Object> callable = ctpl->NewInstance();
    callable->SetPrototype(instance);
    return scope.Escape(callable);
}
