#include "py-gil-lock.h"
#include <uv.h>
#include <iostream>

namespace GILLock {
void Init() {
    static PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();

    static uv_prepare_t gilrelease;
    static uv_check_t gilensure;

    uv_prepare_init(uv_default_loop(), &gilrelease);
    uv_check_init(uv_default_loop(), &gilensure);
    uv_prepare_start(&gilrelease, [] (uv_prepare_t *) {
        PyGILState_Release(gstate);
    });
    uv_check_start(&gilensure, [] (uv_check_t *) {
        if (uv_loop_alive(uv_default_loop())) {
            gstate = PyGILState_Ensure();
        }
    });
    uv_unref((uv_handle_t *)&gilrelease);
    uv_unref((uv_handle_t *)&gilensure);
}
} // namespace GILLock
