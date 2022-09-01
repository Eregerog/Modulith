# pragma once

/**
 * This header should be included by all files of this module,
 * as it automatically includes most files needed.
 * 
 * All classes and free-form functions that can be used by other modules,
 * (e.g. when defined in headers in the include folder)
 * need to use the EDITOR_API macro.
 * 
 * Example:
 * class EDITOR_API MyClass { ... };
 * struct EDITOR_API MyComponent { ... };
 * EDITOR_API void MyFunc();
 */

#ifdef EXPORT_EDITOR_MODULE
    #define EDITOR_API  __declspec(dllexport)
#else
    #define EDITOR_API  __declspec(dllimport)
#endif

#include "ModulithPreCompiledHeader.h"