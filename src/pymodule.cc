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

PyObject *GetModule(void) {
    static PyObject* m = nullptr;

    if (!m) {
        if (PyType_Ready(&JsFunctionType) < 0)
            std::cout << "err!" << std::endl;

        m = PyModule_Create(&pyjsmodule);

        Py_INCREF(&JsFunctionType);
        PyModule_AddObject(m, "JsFunction", (PyObject *)&JsFunctionType);
    }

    return m;
}

} // namespace JsPyModule
