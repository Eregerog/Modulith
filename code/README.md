# Modulith

## Requirements

- Required: A C++ 17 compatible compiler or newer
- Required: CMake 3.13 or newer, CMake 3.16 or newer *heavily* recommended
- Supported compilers: 
  - Any MSVC Compiler shipped with Visual Studio 15.x **but 16.x is recommended**
  - To be exact **MSVC Version 14.15.x and above** have been tested and work
- Supported IDEs:
    - CLion 2020.1.x or newer
    - **Visual Studio is not supported for building the Engine, it is only supported for building Modules**
- Supported Platforms:
  - Windows 64-bit

### Building with CLion (recommended)

**Step 1: Setting up the project**

1. Open the ``code/`` folder with CLion
2. If the CMake reports warnings / errors after opening for the first time **these can be ignored until CLion is set up correctly**
3. If not done already, configure a Toolchain for MSVC under Visual Studio. This can be done under Settings->Build,Execution and Deployment->Toolchains
4. Set the Toolchain's Architecture to amd64
5. Under Settings->Build,Execution and Deployment->CMake add two more Profiles. These should be auto-configured to Release and RelWithDebInfo
6. If the CMake project is not reloaded automatically, do so manually (Shift+F5 default keybind)

**Step 2: (optional): Enabling Debugging for MSVC**

*CLion versions before 2020.1.x currently does not support MSVC debugging by default. Versions 2019.2.x and 2019.3.x allow this feature to be enabled. Not applicable for older versions!*

1. Double tap shift to open the search and search for "Experimental features"
2. Enable ccir.debugger.lldb.windows

**Step 3: (optional): Configure CMake to use precompiled headers**

*Only necessary when using a version of CLion using CMake 3.15 or older. The version chan be checked in the toolchain settings.*

1. Install CMake 3.16 or newer manually
2. Select the installed cmake.exe under Settings->Toolchains->CMake
3. Compilation will now generate precompiled headers, speeding up compilation


**Step 4: Building all dependencies & the project**

*First, build the engine itself. This may take a long time when done for the fist time!*

1. Select the profile of choice (Debug, RelWithDebInfo, Release). For most cases RelWithDebInfo is recommended
2. Select the ``ModulithRuntime`` configuration and build it (if this repository was cloned, the profile is found under "Engine/ModulithRuntime")

*Now, build all the modules that you want to use. They need to be build in order of dependency, or by following the sequence defined below*

3. Select the ``Core`` configuration and  build it  (if this repository was cloned, the profile is found under "Modules/Core")
4. Select the ``Renderer`` configuration and build it (if this repository was cloned, the profile is found under "Modules/Renderer")
5. Building the Physics module:
    1. Copy all the **.dll** files from either ``prebuilt/Debug``, ``prebuilt/RelWithDebInfo`` or ``prebuilt/Release`` into ``build/modules/Physics``. The source directory should match the build profile used.
    2. Select the ``Physics`` configuration and build it (if this repository was cloned, the profile is found under "Modules/Physics")
    3. There may be a lot of warnings when doing so, they can be safely ignored
6. Select the ``Game`` configuration and build it (if this repository was cloned, the profile is found under "Modules/Game")
7. (optional) Select the ``Shooter`` configuration and build it (if this repository was cloned, the profile is found under "Modules/Shooter")
8. (optional) Select the ``Mario`` configuration and build it (if this repository was cloned, the profile is found under "Modules/Mario")
9. Select the ``Editor`` configuration and build it (if this repository was cloned, the profile is found under "Modules/Editor")

*Lastly, you need to set the modules that should be loaded and start the runtime*

Open the ``build/modules/Default.modlist`` in your text editor of choice and specify the modules loaded as follows:
```yaml
- Core
- Renderer
- Physics
- Game
- Editor
```

- If you want to start the engine through CLion:
    - Set the "Working Directory" of the ModulithRuntime configuration to the "build" folder
    - Then select the ``Engine/ModulithRuntime`` configuration and *run* it
- Otherwise, you can start the generated "ModulithRuntime.exe" executable in the "build" folder directly

**Step 5: (optional): Debugging with Visual Studio**

*Since debugging MSVC is currently not well supported under CLion, Visual Studio can be used for debugging*

1. Run the Engine in CLion (do not debug, only run)
2. Open Visual Studio without opening any project (Select "Continue without code" on the start screen)
3. Press the "Attach.." button at the top and select the process of Modulith (usually called ModulithRuntime.exe)
4. You can now open the header files and set breakpoints

**Troubleshooting**

If you are using a older/newer version of the compiler than supported, you may have to change the module function names.
Refer to the documentation's Module System Technical Documentation for more.

When building a module fails because it can't find a header file of the engine, ensure that the engine was built before the module.
Also make sure that the engine and the module were built **on the same profile**.

If the Physics module cannot be loaded, ensure that the proper dlls were copied from the prebuilt folder into the physics module folder. These must always match the profile used when compiling the engine.

When the loading of any module fails, ensure that the profile the module built in is the same as the profile used for the engine (e.g. Debug, RelWithDebInfo, Release)