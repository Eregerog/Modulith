# pragma once

/**
 * This header should be included by all files of this module,
 * as it automatically includes most files needed.
 *
 * All classes and free-form functions that can be used by other modules,
 * (e.g. when defined in headers in the include folder)
 * need to use the RENDERER_API macro.
 *
 * Example:
 * class RENDERER_API MyClass { ... };
 * struct RENDERER_API MyComponent { ... };
 * RENDERER_API void MyFunc();
 */
#ifdef EXPORT_RENDERER_MODULE
    #define RENDERER_API  __declspec(dllexport)
#else
    #define RENDERER_API  __declspec(dllimport)
#endif

#include "ModulithPreCompiledHeader.h"