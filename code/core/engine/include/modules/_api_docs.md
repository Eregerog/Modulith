\page module_api Module System API Documentation

*This docs file gives a short overview of Modulith's module system's API. 
For a more detailed and technical introduction, refer to the \ref module_specs "technical documentation".*

## Module

A module is a collection of code and assets that can be loaded and unloaded at runtime, adding additional sub-engines or gameplay logic to the engine.
Modules can have dependencies between each other, which allows them to use the data structures and systems of another module.

This system can be used in three ways:
- Creating a low-level sub-component of the engine, such as a renderer
- Creating a game module (from one or multiple modules)
- Creating an extension / add-on module to a game, adding additional content 

A module is defined by a folder named after the module found under ``PATH_TO_EXECUTABLE/modules/...``. Inside that folder the module's dll, assets and module config file should be placed.

## Module Context

In order to query what modules are loaded or to manually load modules, the ``modulith::ModuleContext`` can be used.
It is retrieved from the ``modulith::Context`` singleton using: 
```cpp
shared<ModuleContext> moduleCtx = Context::GetInstance<ModuleContext>();
```

Generally speaking, the ``LoadModuleAtBeginOfFrame`` and ``LoadModuleAtBeginOfFrame`` methods can be used to load / unload modules via code.
However, modules can only be loaded when all modules they depend on are also loaded and only be unloaded if no other module depends on them.
The ``LoadModuleWithDependenciesAtBeginOfFrame`` and ``UnloadModuleWithDependantsAtEndOfFrame`` methods automate that process.

New modules are only loaded at the beginning of a frame, while modules are only unloaded at the end of a frame to prevent a critical system being unloaded during the frame, breaking the application.

## Creating a module

To create a module, either refer to the Technical Documentation to create it manually or refer to the Quickstart to use the Editor module for a more automated process.
The latter (creating a Module through the Editor) is recommended.

## Module Resources

A module can register various ``modulith::ModuleResource``. Each of these will receive callbacks when the module is loaded / unloaded, providing an API for implementing symmetric registration and deregistration operations.
When using a the ``modulith::ComponentResource`` that inherits from ``modulith::ModuleResource``, a component will be registered when the module is loaded and automatically deregistered when it is unloaded, allowing that component to be used as long as the module is loaded.

Other examples of *ModuleResoure(s)* include: ``modulith::ComponentResource``, ``modulith::SystemResource`` and ``modulith::SystemsGroupResource``.
 
If you want to create your own kind of *Module Resource*, you simply need to create a class that derives from ``modulith::ModuleResource`` and overrides the necessary callback methods.