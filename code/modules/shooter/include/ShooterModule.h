# pragma once

/**
 * This header should be included by all files of this module,
 * as it automatically includes most files needed.
 * 
 * All classes and free-form functions that can be used by other modules,
 * (e.g. when defined in headers in the include folder)
 * need to use the SHOOTER_API macro.
 * 
 * Example:
 * class SHOOTER_API MyClass { ... };
 * struct SHOOTER_API MyComponent { ... };
 * SHOOTER_API void MyFunc();
 */

#ifdef EXPORT_SHOOTER_MODULE
    #define SHOOTER_API  __declspec(dllexport)
#else
    #define SHOOTER_API  __declspec(dllimport)
#endif

#include "ModulithPreCompiledHeader.h"