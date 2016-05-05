#include "pymodule.h"
#include <iostream>

namespace JsPyModule {

static PyModuleDef pyjsmodule = {
    PyModuleDef_HEAD_INIT,
    "pyjs",
    "pyjs python module",
    -1,
    NULL, NULL, NULL, NULL, NULL
};
static PyObject* module = nullptr;

PyObject *GetModule(void) {
    return module;
}

void Init() {
    JsFunction_Init();
    if (PyType_Ready(&JsFunctionType) < 0)
        std::cout << "err!" << std::endl;

    module = PyModule_Create(&pyjsmodule);

    Py_INCREF(&JsFunctionType);
    PyModule_AddObject(module, "JsFunction", (PyObject *)&JsFunctionType);
}

} // namespace JsPyModule
