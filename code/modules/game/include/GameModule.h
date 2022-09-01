# pragma once

/**
 * This header should be included by all files of this module,
 * as it automatically includes most files needed.
 * 
 * All classes and free-form functions that can be used by other modules,
 * (e.g. when defined in headers in the include folder)
 * need to use the GAME_API macro.
 * 
 * Example:
 * class GAME_API MyClass { ... };
 * struct GAME_API MyComponent { ... };
 * GAME_API void MyFunc();
 */

#ifdef EXPORT_GAME_MODULE
    #define GAME_API  __declspec(dllexport)
#else
    #define GAME_API  __declspec(dllimport)
#endif

#include "ModulithPreCompiledHeader.h"