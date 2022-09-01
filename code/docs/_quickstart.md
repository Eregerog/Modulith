\page quickstart Modulith Quickstart

## Creating a module using the Module Factory

### 1: Creating a module

The module factory (part of the editor module) can be used to automatically generate a module project and its module folder.

1. Ensure that the Editor module is loaded. This can be done by editing the Default.modlist with your text editor of choice found under /modules/ to look like the example given below.
2. Start the ModulithRuntime with the Editor module loaded. 
3. Press F3 to open the debug menu, navigate to the "Browser" window and "Modules" tab and click on the "Create..." button
4. Fill in the necessary information, such as a name and dependencies an click "Create and Start Editing". See below for recommended dependencies
5. You will be asked to select a directory the folder for the module (cmake) project should be created in. This should **NOT** be inside of /modules/, that folder is created automatically
6. Once selected, the module project and module directory will be created and the module will be loaded for editing
7. Open the module in your IDE of choice (see 2a and 2b for guidance) and edit & build the project

#### Example Default.modlist to load the Editor
````yaml
- Core
- Editor
````

#### Module Dependency Recommendations

- The "Core" module provides many foundations for games, such as an application window and OpenGL rendering. For prototypes, this module is recommended.
- The "Renderer" module provides the basic components and systems for rendering objects. It is generally recommended as a dependency for game modules.
- The "Physics" module provides basic components for physics, such as rigidbodies and colliders. It is generally recommended as a dependency for game modules, unless you don't need physics.
- The "Editor" module provides all the windows shown in the debug menu. Game modules should not depend on this module, but you may want to create an additional module that depends on your game module *and* the editor to have editor-only functionality for your game module.

*If you want to change the dependencies of your module project, this can be done by modifying the generated CMakeLists.txt file and changing the entries in the ``set(ModuleDependencies ...)`` line*

*If the installation folder of the ModulithRuntime changes, you will have to adjust the following line in the generated CMakeLists.txt: ``set(ExecutableDirectory ...)``*

### 2a: CLion Setup

*CLion is the author's IDE of choice, as it offers a superior CMake integration when compared to Visual Studio. Using CLion is recommended*

**It is heavily recommended to use CLion with a CMake version of 3.16 or newer. This allows modules to make use of precompiled header, tremendously speeding up compile times.**

1. Open CLion and select the generated module project directory as a project to open
2. If you haven't done so, set up the Visual Studio Toolchain in CLion. 
This can be done under "Settings->Build,Execution & Development->Toolchains", clicking on the "+", selecting Visual Studio and following the instructions on the screen.
3. Ensure that the build output is 64 bit: This is done in the same window by setting the Architecture to "amd64"
4. Add the desired build profiles to the CLion project. This can be done under "Settings->Build,Execution & Development->CMake" by hitting the "+" button.
5. Switch to the correct build profile that matches the Runtime's build configuration
**Now you can edit the module's scripts and build them**

**When compiling the code for the module, beware of the following**
- Modulith only supports profiles named "Debug", "RelWithDebInfo" and "Release"
- Make sure that the build profile of the module matches the profile the Engine is running on, otherwise this will lead to a crash upon loading the module

Whenever new scripts are created externally, CMake inside CLion needs to be refreshed.

### 2b: Visual Studio Setup

**It is heavily recommended to use a CMake version of 3.16 or newer. This allows modules to make use of precompiled header, tremendously speeding up compile times.**

1. Start cmake-gui and select the generated module project folder as the source directory
2. Select a directory to generate to generate the binaries in, preferably ``/cmake-build-visualstudio/``
3. Click ``Configure``, choose the newest installed Visual Studio version and confirm via ``Finish``
4. Click ``Generate``
5. Click ``Open Project`` to open it in Visual Studio
6. Switch to the correct build profile that matches the Runtime's build configuration
**Now you can edit the module's scripts and build them (build by right clicking the project in the solution explorer and hitting "Build")**

**When compiling the code for the module, beware of the following**
- Modulith only supports profiles named "Debug", "RelWithDebInfo" and "Release"
- Make sure that the build profile of the module matches the profile the Engine is running on, otherwise this will lead to a crash upon loading the module

Whenever new scripts are created externally, ``Generate`` need to be pressed inside the cmake-gui and the solution needs to be refreshed in Visual Studio.

### 3 Module "Hello World"

As a first step of writing a custom module, the following "Hello, World!" example should be a guide.
First follow the steps above to create a module and open its project in the IDE. It can have any name and needn't have any dependencies.

Open the autogenerated *ModuleName*Module.cpp. This is the entry point for the module, where we can print "Hello, World"!
This can be done by adding the following line in the ``OnInitialize(...)`` function:

```cpp
LogInfo("Hello, World!");
```

Afterwards, build the module with the correct profile and re-load it in the engine.
There should not be a "*ModuleName*: Hello, World!" logged in the engine's console.

**There is a bug in some CLion versions that will prompt an error when compiling the module while it is loaded in the engine. Just compile it again and that time it should work.**

### 4 Tips

It is recommended to always include the "*ModuleName*Module.h" file in every header and cpp file, since it already includes most of the headers needed.
These include Modulith API headers, STL headers and dependencies such as ImGui.