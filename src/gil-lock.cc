#include "gil-lock.h"
#include <uv.h>
#include <iostream>
#include "debug.h"

namespace GILLock {
void Init() {
    PyEval_InitThreads();
    static PyThreadState *_save = nullptr;

    static uv_prepare_t gilrelease;
    static uv_check_t gilensure;

    uv_prepare_init(uv_default_loop(), &gilrelease);
    uv_check_init(uv_default_loop(), &gilensure);
    uv_prepare_start(&gilrelease, [] (uv_prepare_t *) {
        _save = PyEval_SaveThread();
    });
    uv_check_start(&gilensure, [] (uv_check_t *) {
        if (PyGILState_Check()) return;
        ASSERT(_save);
        PyEval_RestoreThread(_save);
    });
    uv_unref((uv_handle_t *)&gilrelease);
    uv_unref((uv_handle_t *)&gilensure);
}
} // namespace GILLock
