#include "jsobject.h"
#include "typeconv.h"
#include <iostream>
#include "error.h"
#include "debug.h"

Nan::Persistent<v8::FunctionTemplate> JsPyWrapper::constructorTpl;
Nan::Persistent<v8::ObjectTemplate> JsPyWrapper::callableTpl;

bool JsPyWrapper::implicitConversionEnabled = true;

void JsPyWrapper::SetObject(PyObjectWithRef object, v8::Local<v8::Object> instance) {
    this->object = std::move(object);
}

PyObjectWithRef JsPyWrapper::GetObject() {
    return object;
}

JsPyWrapper *JsPyWrapper::UnWrap(v8::Local<v8::Object> object) {
    Nan::HandleScope scope;
    // find real `this`
    while (!IsInstance(object)) {
        v8::Local<v8::Value> prototypeValue = object->GetPrototype();
        if (prototypeValue->IsNull()) {
            return nullptr; // error
        }
        object = prototypeValue->ToObject();
    }
    return ObjectWrap::Unwrap<JsPyWrapper>(object);
}

v8::Local<v8::Object> JsPyWrapper::NewInstance(PyObjectWithRef object) {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Function> cons = Nan::New<v8::FunctionTemplate>(constructorTpl)->GetFunction();
    v8::Local<v8::Object> instance = Nan::NewInstance(cons, 0, {}).ToLocalChecked();
    JsPyWrapper *wrapper = UnWrap(instance);
    wrapper->SetObject(object, instance);
    return scope.Escape(instance);
}

bool JsPyWrapper::IsInstance(v8::Local<v8::Object> object) {
    Nan::HandleScope scope;
    return Nan::New(constructorTpl)->HasInstance(object);
}

v8::Local<v8::Object> JsPyWrapper::makeFunction(v8::Local<v8::Object> instance) {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::ObjectTemplate> ctpl = Nan::New(callableTpl);
    v8::Local<v8::Object> callable = ctpl->NewInstance();
    Nan::SetPrototype(callable, instance).ToChecked();
    return scope.Escape(callable);
}

void JsPyWrapper::Init(v8::Local<v8::Object> exports) {
    GILStateHolder gilholder;
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
    Nan::SetMethod(prototpl, "$type", Type);

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

    Nan::SetAccessor(exports, Nan::New("implicitConversion").ToLocalChecked(),
        [] (v8::Local<v8::String>, const Nan::PropertyCallbackInfo<v8::Value> &args) {
            args.GetReturnValue().Set(Nan::New(implicitConversionEnabled));
        }, [] (v8::Local<v8::String>, v8::Local<v8::Value> value, const Nan::PropertyCallbackInfo<void> &args) {
            implicitConversionEnabled = value->BooleanValue();
        }
    );
}

void JsPyWrapper::New(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    GILStateHolder gilholder;
    Nan::HandleScope scope;
    v8::Local<v8::Object> thisObject;
    if (!args.IsConstructCall()) {
        v8::Local<v8::Function> cons = Nan::New(constructorTpl)->GetFunction();
        thisObject = Nan::NewInstance(cons).ToLocalChecked();
    } else {
        thisObject = args.This();
    }
    JsPyWrapper *wrapper = new JsPyWrapper();
    wrapper->Wrap(thisObject);
    if (args.Length() >= 1) {
        wrapper->SetObject(JsToPy(args[0]), thisObject);
    } else {
        wrapper->SetObject(JsToPy(Nan::Undefined()), thisObject);
    }
    args.GetReturnValue().Set(thisObject);
    CHECK_PYTHON_ERROR;
}

void JsPyWrapper::Value(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    GILStateHolder gilholder;
    JsPyWrapper *wrapper = UnWrap(args.This());
    if (!wrapper || !wrapper->object) return Nan::ThrowTypeError("Unexpected object");

    args.GetReturnValue().Set(PyToJs(wrapper->object));
    CHECK_PYTHON_ERROR;
}

void JsPyWrapper::Repr(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    GILStateHolder gilholder;
    JsPyWrapper *wrapper = UnWrap(args.This());
    if (!wrapper || !wrapper->object) return Nan::ThrowTypeError("Unexpected object");

    args.GetReturnValue().Set(PyToJs(PyObjectWithRef(PyObject_Repr(wrapper->object))));
    CHECK_PYTHON_ERROR;
}

void JsPyWrapper::Str(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    GILStateHolder gilholder;
    JsPyWrapper *wrapper = UnWrap(args.This());
    if (!wrapper || !wrapper->object) return Nan::ThrowTypeError("Unexpected object");

    PyObjectWithRef object = PyObjectMakeRef(wrapper->object);
    if (!PyUnicode_Check(object)) object = PyObjectWithRef(PyObject_Str(wrapper->object));
    args.GetReturnValue().Set(PyToJs(object));
    CHECK_PYTHON_ERROR;
}

void JsPyWrapper::ValueOf(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    GILStateHolder gilholder;
    JsPyWrapper *wrapper = UnWrap(args.This());
    if (!wrapper || !wrapper->object) return Nan::ThrowTypeError("Unexpected object");

    PyObjectBorrowed object = wrapper->object;
    if (PyLong_CheckExact(object)) {
        args.GetReturnValue().Set(Nan::New<v8::Number>(PyLong_AsDouble(object)));
    } else {
        args.GetReturnValue().Set(PyToJs(object));
    }
    CHECK_PYTHON_ERROR;
}

void JsPyWrapper::Type(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    GILStateHolder gilholder;
    JsPyWrapper *wrapper = UnWrap(args.This());
    if (!wrapper || !wrapper->object) return Nan::ThrowTypeError("Unexpected object");

    PyObjectBorrowed object = wrapper->object;
    args.GetReturnValue().Set(PyToJs(PyObject_Type(object)));
    CHECK_PYTHON_ERROR;
}

void JsPyWrapper::Attr(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    GILStateHolder gilholder;
    JsPyWrapper *wrapper = UnWrap(args.This());
    if (!wrapper || !wrapper->object) return Nan::ThrowTypeError("Unexpected object");

    PyObjectBorrowed object = wrapper->object;
    Nan::HandleScope scope;

    PyObjectWithRef attr(JsToPy(args[0]));
    if (args.Length() == 1) {
        PyObjectWithRef subObject(PyObject_GetAttr(object, attr));
        if (subObject) {
            args.GetReturnValue().Set(PyToJs(subObject, implicitConversionEnabled));
        } else {
            args.GetReturnValue().Set(Nan::Undefined());
        }
    } else if (args.Length() == 2) {
        PyObject_SetAttr(object, attr, JsToPy(args[1]));
    }
    CHECK_PYTHON_ERROR;
}

void JsPyWrapper::AttrGetter(v8::Local<v8::String> name, const Nan::PropertyCallbackInfo<v8::Value> &info) {
    GILStateHolder gilholder;
    JsPyWrapper *wrapper = UnWrap(info.This());
    if (!wrapper || !wrapper->object) return Nan::ThrowTypeError("Unexpected object");

    if (!name->IsString()) return;
    PyObjectBorrowed object = wrapper->object;
    PyObjectWithRef attr = JsToPy(name);
    if (!PyObject_HasAttr(object, attr)) return;
    PyObjectWithRef subObject(PyObject_GetAttr(object, attr));
    info.GetReturnValue().Set(PyToJs(subObject, implicitConversionEnabled));
    CHECK_PYTHON_ERROR;
}

void JsPyWrapper::AttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value,
    const Nan::PropertyCallbackInfo<v8::Value> &info) {
    GILStateHolder gilholder;
    JsPyWrapper *wrapper = UnWrap(info.This());
    if (!wrapper || !wrapper->object) return Nan::ThrowTypeError("Unexpected object");

    if (!name->IsString()) return;
    if (!IsInstance(info.This())) return;

    PyObject_SetAttr(wrapper->object, JsToPy(name), JsToPy(value));
    CHECK_PYTHON_ERROR;
}

void JsPyWrapper::AttrEnumerator(const Nan::PropertyCallbackInfo<v8::Array> &info) {
    GILStateHolder gilholder;
    JsPyWrapper *wrapper = UnWrap(info.This());
    if (!wrapper || !wrapper->object) return Nan::ThrowTypeError("Unexpected object");

    if (!IsInstance(info.This())) return;

    PyObjectWithRef pyAttrs(PyObject_Dir(wrapper->object));
    info.GetReturnValue().Set(PyToJs(pyAttrs).As<v8::Array>());
    CHECK_PYTHON_ERROR;
}

void JsPyWrapper::Call(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    GILStateHolder gilholder;
    JsPyWrapper *wrapper = UnWrap(args.This());
    if (!wrapper || !wrapper->object) return Nan::ThrowTypeError("Unexpected object");

    PyObjectBorrowed pyFunc = wrapper->object;
    if (!PyCallable_Check(pyFunc)) {
        return Nan::ThrowTypeError("not a function");
    }
    Nan::HandleScope scope;
    PyObjectWithRef pyArgs(PyTuple_New(0));
    PyObjectWithRef pyKw(nullptr);

    if (args[0]->IsArray()) {
        v8::Local<v8::Array> jsArgs = args[0].As<v8::Array>();
        pyArgs = PyObjectWithRef(PyTuple_New(jsArgs->Length()));
        for (ssize_t i = 0; i < jsArgs->Length(); i++) {
            int result = PyTuple_SetItem(pyArgs, i, JsToPy(jsArgs->Get(i)).escape());
            ASSERT(result != -1);
        }
        if (args[1]->IsObject()) {
            v8::Local<v8::Object> jsKw = args[1]->ToObject();
            pyKw = PyObjectWithRef(PyDict_New());
            v8::Local<v8::Array> keys = Nan::GetOwnPropertyNames(jsKw).ToLocalChecked();
            for (ssize_t i = 0; i < keys->Length(); i++) {
                v8::Local<v8::Value> jsKey = keys->Get(i);
                v8::Local<v8::Value> jsValue = jsKw->Get(jsKey);
                int result = PyDict_SetItem(pyKw, JsToPy(jsKey), JsToPy(jsValue));
                ASSERT(result != -1);
            }
        }
    }
    args.GetReturnValue().Set(PyToJs(PyObjectWithRef(PyObject_Call(pyFunc, pyArgs, pyKw)), implicitConversionEnabled));
    CHECK_PYTHON_ERROR;
}

void JsPyWrapper::CallFunction(const Nan::FunctionCallbackInfo<v8::Value> &args) {
    GILStateHolder gilholder;
    JsPyWrapper *wrapper = UnWrap(args.This());
    if (!wrapper || !wrapper->object) return Nan::ThrowTypeError("Unexpected object");

    PyObjectBorrowed pyFunc = wrapper->object;
    if (!PyCallable_Check(pyFunc)) {
        return Nan::ThrowTypeError("not a function");
    }
    Nan::HandleScope scope;
    // arguments
    PyObjectWithRef pyTuple(PyTuple_New(args.Length()));
    for (ssize_t i = 0; i < args.Length(); i++) {
        int result = PyTuple_SetItem(pyTuple, i, JsToPy(args[i]).escape());
        ASSERT(result != -1);
    }
    args.GetReturnValue().Set(PyToJs(PyObjectWithRef(PyObject_CallObject(pyFunc, pyTuple)), implicitConversionEnabled));
    CHECK_PYTHON_ERROR;
}
