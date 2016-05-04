#include "py-gil-lock.h"
#include <uv.h>

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
        gstate = PyGILState_Ensure();
    });
}
} // namespace GILLock
