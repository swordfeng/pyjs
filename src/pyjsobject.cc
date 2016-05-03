#include "pyjsobject.h"
#include "pyjstypeconv.h"
#include <iostream>
#include <cassert>

Nan::Persistent<v8::FunctionTemplate> PyjsObject::constructorTpl;
Nan::Persistent<v8::Function> PyjsObject::makeFunction;

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
    Nan::SetMethod(prototpl, "__repr__", Repr);
    Nan::SetMethod(prototpl, "__str__", Str);
    Nan::SetMethod(prototpl, "__call__", Call);
    Nan::SetMethod(prototpl, "value", Value);
    Nan::SetMethod(prototpl, "attr", Attr);
    // $ is aliased to Attr
    Nan::SetMethod(prototpl, "$", Attr);
    // make toString an alias to str
    Nan::SetMethod(prototpl, "toString", Str);
    // make inspect an alias to repr
    Nan::SetMethod(prototpl, "inspect", Repr);

    constructorTpl.Reset(tpl);
    exports->Set(Nan::New("PyObject").ToLocalChecked(), tpl->GetFunction());

    // make a function from a callable PyObject
    static const char scriptString[] = "                                            \
        function makeFunction(object) {                                             \
            var resultFunction = function () {                                      \
                var args = [];                                                      \
                for (var i = 0; i < arguments.length; i++) args.push(arguments[i]); \
                return object.__call__(args);                                       \
            };                                                                      \
        resultFunction.__proto__ = object;                                          \
            return resultFunction;                                                  \
        };                                                                          \
        makeFunction                                                                \
    ";
    v8::Local<Nan::BoundScript> script = Nan::CompileScript(Nan::New(scriptString).ToLocalChecked()).ToLocalChecked();
    v8::Local<v8::Function> resultFunction = Nan::RunScript(script).ToLocalChecked().As<v8::Function>();
    makeFunction.Reset(resultFunction);
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

void PyjsObject::AttrGetter(v8::Local<v8::String> name, const Nan::PropertyCallbackInfo<v8::Value> &info) {
    PyObject *object = UnWrap(info.This())->object;
    Nan::HandleScope scope;
    PyObject *attr = JsToPy(name);
    PyObject *subObject = PyObject_GetAttr(object, attr);
    if (subObject) {
        info.GetReturnValue().Set(PyToJs(subObject));
        Py_DECREF(subObject);
    } else {
        info.GetReturnValue().Set(Nan::Undefined());
    }
    Py_DECREF(attr);
}

void PyjsObject::AttrSetter(v8::Local<v8::String> name, v8::Local<v8::Value> value,
    const Nan::PropertyCallbackInfo<void> &info) {
    PyObject *object = UnWrap(info.This())->object;
    Nan::HandleScope scope;
    PyObject *attr = JsToPy(name);
    PyObject *pyValue = JsToPy(value);
    PyObject_SetAttr(object, attr, pyValue);
    Py_DECREF(pyValue);
    Py_DECREF(attr);
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
            int result = PyTuple_SetItem(pyTuple, i, JsToPy(jsArr->Get(i)));
            assert(result != -1);
        }

        PyObject *pyResult = PyObject_CallObject(pyFunc, pyTuple);

        args.GetReturnValue().Set(PyToJs(pyResult));
        Py_XDECREF(pyResult);
        Py_DECREF(pyTuple);
    }
}

// steal one ref
v8::Local<v8::Object> PyjsObject::NewInstance(PyObject *object) {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Function> cons = Nan::New<v8::FunctionTemplate>(constructorTpl)->GetFunction();
    v8::Local<v8::Object> instance = cons->NewInstance(0, {});
    PyjsObject *wrapper = ObjectWrap::Unwrap<PyjsObject>(instance);
    wrapper->SetObject(object);
    // add getter && setters to object ?
    PyObject *attrNames = PyObject_Dir(object);
    assert(attrNames);

    Py_ssize_t size = PyList_Size(attrNames);
    for (ssize_t i = 0; i < size; i++) {
        PyObject *item = PyList_GetItem(attrNames, i);
        Py_ssize_t size;
        const char *attrName = PyUnicode_AsUTF8AndSize(item, &size);
        if (size >= 4 && attrName[0] == '_' && attrName[1] == '_' &&
            attrName[size - 1] == '_' && attrName[size - 2] == '_') {
            continue;
        }
        v8::Local<v8::String> jsName = Nan::New(attrName, size).ToLocalChecked();
        Nan::SetAccessor(instance, jsName, AttrGetter, AttrSetter);
    }

    Py_DECREF(attrNames);
    return scope.Escape(instance);
}

bool PyjsObject::IsInstance(v8::Local<v8::Object> object) {
    Nan::HandleScope scope;
    return Nan::New(constructorTpl)->HasInstance(object);
}
