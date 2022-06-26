#ifndef PYTHON_WRAPPER_H
#define PYTHON_WRAPPER_H

#pragma once

#pragma push_macro("slots")
#undef slots

#define PY_SSIZE_T_CLEAN
#include <python3.7/Python.h>

#pragma pop_macro("slots")
#endif // PYTHON_WRAPPER_H
