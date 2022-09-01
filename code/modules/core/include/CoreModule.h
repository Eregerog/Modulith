# pragma once

/**
 * This header should be included by all files of this module,
 * as it automatically includes most files needed.
 *
 * All classes and free-form functions that can be used by other modules,
 * (e.g. when defined in headers in the include folder)
 * need to use the CORE_API macro.
 *
 * Example:
 * class CORE_API MyClass { ... };
 * struct CORE_API MyComponent { ... };
 * CORE_API void MyFunc();
 */

#ifdef EXPORT_CORE_MODULE
    #define CORE_API  __declspec(dllexport)
#else
    #define CORE_API  __declspec(dllimport)
#endif

#include "ModulithPreCompiledHeader.h"
#include "input/KeyCodes.h"