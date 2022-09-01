# pragma once

/**
 * This header should be included by all files of this module,
 * as it automatically includes most files needed.
 *
 * All classes and free-form functions that can be used by other modules,
 * (e.g. when defined in headers in the include folder)
 * need to use the PHYSICS_API macro.
 *
 * Example:
 * class PHYSICS_API MyClass { ... };
 * struct PHYSICS_API MyComponent { ... };
 * PHYSICS_API void MyFunc();
 */

#ifdef EXPORT_PHYSICS_MODULE
    #define PHYSICS_API  __declspec(dllexport)
#else
    #define PHYSICS_API  __declspec(dllimport)
#endif

#include "ModulithPreCompiledHeader.h"