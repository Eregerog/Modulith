\page module_specs Module System Technical Documentation

*This docs file gives an in-depth overview of the technical specifications of Modulith's module system. 
For an API documentation, refer to the \ref module_api "module system API Docs"*

## Standards & Definitions

### Module

A **module** is a directory named after the module that is contained in the ``modules`` folder relative to the executable.

This directory must contain:
- A Module.modconfig file, which contains information about its name, authors, dependencies etc.

This directory can contain:
- A *ModuleName*.dll or *ModuleName*_hotloadable.dll, which will be loaded and whose code will be executed when the module is loaded. This dll is explained in more detail below
- Any number of additional dlls that the main module dll depends on. These will be automatically loaded if the module has been linked against them.
- Any number of subfolders with files that can be loaded as assets.

### Module DLL

When there is a **module-specific dll** (either named *ModuleName*.dll or *ModuleName*_hotloadable.dll) in the module's folder, it is loaded / unloaded alongside its module.
If a *ModuleName*_hotloadable.dll exists, a copy named *ModuleName*.dll is created and loaded, allowing the hotloadable dll to be replaced while the module is loaded. Otherwise *ModuleName*.dll is simply loaded.

The dll(s) must contain the C++ dllexported functions Initialize(modulith::ModuleResource&) and Shutdown(modulith::ModuleResource&).
The former is called when the module is initialized / loaded (refer to module initialization) and the latter is called when the module is shutdown / unloaded (refer to module shutdown).

#### Example

In order to provide the functions mentioned above, a source file with the code similar to the following must be compiled into the dll:

````cpp
#include "modules/resources/ModuleResources.h"

extern "C++" {
    void __declspec(dllexport) __cdecl Initialize(modulith::ModuleResources& module) {
        // Initialization code goes here
    }
    
    void __declspec(dllexport) __cdecl Shutdown(modulith::ModuleResources& module){
        // Shutdown code goes here  
    }
}
````

### Module Project

A **module project** refers to the CMake project that is used to generate the module directory and compile its module dll.
This CMake project must do the following in order to compile a successful module dll:
- Link against all static libraries found in (relative to the executable) modding/ (this includes the engine)
- Add the following include (relative to the executable): modding/include
- Generate a dynamically linked library named *ModuleName.dll* or (preferrably) *ModuleName_hotloadable.dll* that is automatically or manually copied into the module folder
- Add a valid Module.modconfig file to the module's folder (see below for an example)
- Set the following target_compile_definition: SPDLOG_COMPILED_LIB
- Ensure that all structs / classes in its public headers are correctly dllexported / dllimported

If this CMake project also has other modules as dependencies, it must do the following for each of them:
- Link against the *ModuleName*.lib in (relative to the module's folder) modding/
- Add the following include (relative to the module's folder) modding/include

If the module should be a dependency for other modules, it must:
- Copy its *ModuleName*.lib to (relative to its module's folder) modding/
- Copy its public headers to (relative to its module's folder) modding/include

#### Modconfig Example

````yaml
name: MyModule
guid: a642641d-079e-4032-b71d-405c9c093daa
authors: Uncle Bob, Bjarne Stroustrup, Jonathan Blowe
description: This can be anything you want
version: 1.0.0
dependencies:
    - module: FirstDependencyModuleName
      version: 1.0.0
    - module: SecondDependencyModuleName
      version: 1.0.0
````

## Initialization & Shutdown of Modules

When a module is loaded, the dll-exported ``Initialize(modulith::ModuleResource& resource)`` is called. 
After that ``modulith::ModuleResource``(s) can be registered and other setup code executed.

Similarly, when the module is unloaded the ``Shutdown(modulith::ModuleResource& resource)`` is called. 
ModuleResource needn't be deregistered in the Shutdown method, this process is automatic.

### Customizing the dll function Initialization & Shutdown procedures

In order to call the dll-exported ``Initialize`` and ``Shutdown`` functions, the module system needs to have their proper dll-exported name. Since this changes between compiler and compiler version, those names can be customized.
If you get an error along the lines of ``The Initialize/Shutdown function of module '...' could not be found! Error: ###``, you may need to modify the names.

These names can be specified globally, in the "Modulith.config" file found in the same folder as the executable. 
The names after "DefaultInitializeFunctionName:" and "DefaultShutdownFunctionName:" specifies the Initialize and Shutdown dll names, respectively.

Alternatively, should problems only occur with a single module, the names can be changed on a per-module basis as well.
Entries of type "initializeFunctionOverride:" and "shutdownFunctionOverride:" can be added to the module's "Module.modconfig" file to define a module-specific dll function name.

The default (and fallback) function names are as follows:
- Initialize: ?Initialize@@YAXAEAVModuleResources@modulith@@@Z
- Shutdown: ?Shutdown@@YAXAEAVModuleResources@modulith@@@Z

If the compiled dll has different names, these can be found using [dependency walker](http://www.dependencywalker.com/) by doing the following:
1. Open the dll using Dependency Walker
2. Enable C++ function undecoration by pressing F10
3. Selecting the root of the tree in the left subwindow
4. Searching for a function named ``void Initialize(class modulith::ModuleResources &)`` in the middle-right window
5. Once found, decorate the C++ functions again by pressing F10
6. The displayed name is the correct dll function name.