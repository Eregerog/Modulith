# pragma once

/**
 * This header should be included by all files of this module,
 * as it automatically includes most files needed.
 * 
 * All classes and free-form functions that can be used by other modules,
 * (e.g. when defined in headers in the include folder)
 * need to use the MARIO_API macro.
 * 
 * Example:
 * class MARIO_API MyClass { ... };
 * struct MARIO_API MyComponent { ... };
 * MARIO_API void MyFunc();
 */
#ifdef EXPORT_MARIO_MODULE
    #define MARIO_API  __declspec(dllexport)
#else
    #define MARIO_API  __declspec(dllimport)
#endif

#include "ModulithPreCompiledHeader.h"