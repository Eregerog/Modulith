\page editor_api Editor Module API Documentation

*This docs file gives a short overview over the Editor Module*

When loaded the editor module provides an in-engine editor with UI. Its featuers are listed below:
The editor can be opened by pressing ``F3``.

## Module Browser

Under ``Browser->Modules`` the Module Browser is found. This lists all currently valid modules of the application.
There they can be loaded, unloaded and reloaded. Additional information is shown in the Properties window by clicking on a module.

Furthermore, new modules may also be created. This automatically creates a CMake Project at a user defined path and a module entry for the application.

Lastly, any module may also be edited from there, if the user points to the module's CMake Project root folder. While edited, the Script Browser may be used.

## Script Browser

Under ``Browser->Scripts`` the Script Browser is found. It shows the filetree of a module project with the C++ source and header files.
New scripts from commonly-used templates may be created with a few clicks.

## Entity Info

The ``Entity Info`` window displays information about the entities currently alive. There are 2 different views:
- Hierarchy: Shows the entities as they are layed out in the scene graph. This does not represent the layout in memory.
- Chunks: Shows the chunks with their entities as they are layed out in memory.

In the hierarchy view, entities may be clicked to show their properties in the Property Window.
The values of serializable components attached to entities may temporarily be changed there.

## Profiler

The module also provides a simple ``Profiler`` that simply shows the timing of various modulith::System function calls.