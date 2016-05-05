#include <Python.h>


namespace JsPyModule {


static PyModuleDef pyjsmodule = {
    PyModuleDef_HEAD_INIT,
    "pyjs",
    "pyjs python module",
    -1,
    NULL, NULL, NULL, NULL, NULL
};

PyMODINIT_FUNC PyInit_JsFunction(void) {
    PyObject* m;

    JsFunctionType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&JsFunctionType) < 0)
        return NULL;

    m = PyModule_Create(&pyjsmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&JsFunctionType);
    PyModule_AddObject(m, "JsFunction", (PyObject *)&JsFunctionType);
    return m;
}

} // namespace JsPyModule
