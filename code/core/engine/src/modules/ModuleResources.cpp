/*
 * \brief
 * \author Daniel Götz
 */

#include "modules/resources/ModuleResources.h"

#include <utility>
#include <modules/ModulePathUtils.h>
#include <utils/PreferencesContext.h>
#include "Context.h"

namespace modulith {

    typedef void (__cdecl* InitializeFn)(ModuleResources&);

    typedef void (__cdecl* ShutdownFn)(ModuleResources&);

    std::string fallbackInitializeFnName = "?Initialize@@YAXAEAVModuleResources@modulith@@@Z"; // if compiled by MSVC
    std::string fallbackShutdownFnName = "?Shutdown@@YAXAEAVModuleResources@modulith@@@Z"; // if compiled by MSVC

    // TODO DG: Make this platform independent
    struct WindowsModuleHandle {
        explicit WindowsModuleHandle(HMODULE handle) : Handle(handle) {}

        HMODULE Handle = nullptr;
    };

    ModuleResources::ModuleResources(ModuleInfo info) : _info(std::move(info)) { }

    void ModuleResources::initialize() {
        auto name = _info.Name;

        initializeModuleCode();

        _status = ModuleStatus::INITIALIZED;

        Resource::InvokeLoadCallbacksOn(_resources, _info.Name);

        CoreLogInfo("Module '{}' has been initialized", name)
    }

    void ModuleResources::shutdown() {
        auto name = _info.Name;

        Resource::InvokeUnloadCallbacksOn(_resources, _info.Name);
        _resources.clear();

        _status = ModuleStatus::SHUTTING_DOWN;
        shutdownModuleCode();

        freeModuleCode();

        CoreLogInfo("Module '{}' has been shut down", name)
    }

    template<class FnType, class... Args>
    void ModuleResources::findAndExecuteFunctionWith(const std::string& fnName, Args... args) {
        FnType fnPtr = nullptr;
        if (_libraryHandle.has_value()) {
            auto libraryHandle = std::any_cast<WindowsModuleHandle>(_libraryHandle).Handle;

            fnPtr = (FnType) GetProcAddress(libraryHandle, fnName.c_str());

            CoreAssert(fnPtr, "The {} function of module '{}' could not be found! Error: {}", fnName, _info.Name,
                GetLastError())

            fnPtr(args...);
        }
    }

    void ModuleResources::initializeModuleCode() {
        std::error_code msvcErr;

        auto name = _info.Name;

        auto moduleFolderPath = ModulePathUtils::GetModuleFolder(name);

        auto hotloadableModulePath = ModulePathUtils::GetHotloadableModuleDllPath(moduleFolderPath, name);
        auto loadedModulePath = moduleFolderPath / (name + ".dll");

        // Copy a hotloadable module, if it exists
        if (fs::exists(hotloadableModulePath)) {
            CoreLogInfo("Found a hotloadable library for module '{}', creating a copy...", name)
            std::error_code copyErr;
            fs::copy(
                hotloadableModulePath, loadedModulePath, fs::copy_options::update_existing, copyErr
            );
            CoreAssert(!copyErr, "Could not copy the hotloadable module library '{}' to be loaded. Error: {}", name,
                copyErr.message())
        }

        if (!fs::exists(loadedModulePath))
            // This module has no code to load
            return;

        auto moduleToLoad = fs::canonical(loadedModulePath, msvcErr);

        CoreAssert(!msvcErr, "The path to the module '{}' at '{}' could not be found. Error: {}", name,
            moduleToLoad.generic_string(), msvcErr.message())

        auto dllDirChangeSuccess = SetDllDirectory(moduleFolderPath.generic_string().c_str());

        CoreAssert(dllDirChangeSuccess,
            "The temporary change in dll directory to the folder of module '{}' was not successful!", name)

        HMODULE libraryHandle = LoadLibrary(moduleToLoad.generic_string().c_str());
        if (!libraryHandle) {
            auto error = GetLastError();
            CoreLogError("The '{}' module at '{}' could not be loaded! Error code: {}", name, moduleToLoad, error)

            switch (error) {
                case 193: CoreAssert(false,
                    "This is likely due to a mismatch of the modules architecture or outdated build.")
                    break;
                case 126: CoreAssert(false,
                    "This is likely due to the library not being able to load/find the DLLs it depends on")
                    break;
                case 127: CoreAssert(false,
                    "This is likely because it could not find a symbol in one of its dependant libraries. Did the symbol's name, signature or calling convention change? It the symbol no longer being exported?")
                    break;
                default: CoreAssert(false, "This is due to an unknown case!")
                    break;
            }
        }
        _libraryHandle = WindowsModuleHandle(libraryHandle);

        auto preferencesCtx = Context::Instance().Get<PreferencesContext>();

        std::string initializeFnName;
        if (!_info.InitializeFunctionOverride.empty()) {
            initializeFnName = _info.InitializeFunctionOverride;
            CoreLogInfo("When loading the module {} the module-specific initialize function name override '{}' will be used", _info.Name, initializeFnName)
        } else if (auto defaultInitializeFunction = preferencesCtx->TryGet("DefaultInitializeFunctionName")) {
            initializeFnName = defaultInitializeFunction.value();
            CoreLogInfo("When loading the module {} the config-default initialize function name '{}' will be used", _info.Name, initializeFnName)
        } else {
            initializeFnName = fallbackInitializeFnName;
            CoreLogInfo("When loading the module {} the fallback initialize function name '{}' will be used", _info.Name, initializeFnName)
        }

        _status = ModuleStatus::INITIALIZING;
        // explicit args type is required, otherwise *this is passed by value
        findAndExecuteFunctionWith<InitializeFn, ModuleResources&>(initializeFnName, *this);
    }

    void ModuleResources::shutdownModuleCode() {

        _status = ModuleStatus::SHUTTING_DOWN;

        if (_libraryHandle.has_value()) {

            auto preferencesCtx = Context::Instance().Get<PreferencesContext>();

            std::string shutdownFnName;
            if (!_info.ShutdownFunctionOverride.empty()) {
                shutdownFnName = _info.ShutdownFunctionOverride;
                CoreLogInfo("When unloading the module {} the module-specific shutdown function name override '{}' will be used", _info.Name, shutdownFnName)
            } else if (auto defaultShutdownFunction = preferencesCtx->TryGet("DefaultShutdownFunctionName")) {
                shutdownFnName = defaultShutdownFunction.value();
                CoreLogInfo("When unloading the module {} the config-default shutdown function name '{}' will be used", _info.Name, shutdownFnName)
            } else {
                shutdownFnName = fallbackShutdownFnName;
                CoreLogInfo("When unloading the module {} the fallback shutdown function name '{}' will be used", _info.Name, shutdownFnName)
            }

            // explicit args type is required, otherwise *this is passed by value
            findAndExecuteFunctionWith<ShutdownFn, ModuleResources&>(shutdownFnName, *this);

            _status = ModuleStatus::UNINITIALIZED;
        }
    }

    void ModuleResources::freeModuleCode() {
        if (_libraryHandle.has_value()) {
            auto name = _info.Name;

            auto libraryHandle = std::any_cast<WindowsModuleHandle>(_libraryHandle).Handle;
            auto freeLibrarySuccess = FreeLibrary(libraryHandle);
            CoreAssert(freeLibrarySuccess, "The module '{}' could not be unloaded", name)
            _libraryHandle = nullptr;

            auto moduleFolderPath = fs::absolute(fs::path("modules") / name);
            auto hotloadableModulePath = ModulePathUtils::GetHotloadableModuleDllPath(moduleFolderPath, name);
            auto loadedModulePath = moduleFolderPath / (name + ".dll");
            if (fs::exists(hotloadableModulePath)) {
                CoreLogInfo("Found a hotloadable library for module '{}', deleting the copy...", name)
                std::error_code removeErr;
                fs::remove(loadedModulePath, removeErr);
                if (removeErr) CoreLogWarn("Could not deleted the copy of module library '{}'. Error: {}", name,
                    removeErr.message())
            }
        }
    }

}
