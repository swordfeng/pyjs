#pragma once

#include "pyjsfunction.h"
#include "pymodule.h"


namespace JsPyModule {
void Init();
PyObject *GetModule(void);
} // namespace JsPyModule
