/*
 * \brief
 * \author Daniel Götz
 */

#include "modules/ModuleContext.h"
#include "Context.h"
#include "modules/ModuleUtils.h"

namespace modulith {


    ModuleContext::ModuleContext() : Subcontext("Module Context") {
        RefreshAvailableModules();
    }

    void ModuleContext::RefreshAvailableModules() {
        _availableModules.clear();

        std::vector<Module> allModules{};
        for(const auto& info : ModuleUtils::GetAllAvailableModules()){
            auto module = info.AsModule();
            allModules.push_back(module);
            _availableModules.emplace(module, info);
        }

        for(const auto& kvp : _loadedModules){
            if(!ModuleExists(kvp.first)){
                CoreLogWarn("The loaded module {} is no longer a valid module after the refresh. This means it cannot be reloaded!", kvp.first)
            }
        }

        _moduleDependencies.Clear();
        for (const auto& module : allModules) {
            _moduleDependencies.Add(module);
        }

        for (const auto& module : allModules) {
            for (auto& dependency : InfoOf(module).Dependencies) {
                auto moduleMatch = ModuleFromName(dependency.ModuleName);
                CoreAssert(moduleMatch, "No matching loadable module was found for module's {} dependency {}", module.Name(), dependency.ModuleName)
                auto match = InfoOf(moduleMatch.value());
                _moduleDependencies.AddDependency(match.AsModule(), module);
            }
        }
    }

    void ModuleContext::LoadModuleAtBeginOfFrame(const Module& module) {
        CoreAssert(ModuleExists(module), "Module {} cannot be loaded because it is not valid", module)
        CoreAssert(CanLoad(module), "Module {} cannot be loaded because of missing dependencies!", module)

        _modulesToLoad.push_back(module);
    }


    void ModuleContext::UnloadModuleAtEndOfFrame(const Module& module) {
        CoreAssert(ModuleIsStillLoadedNextFrame(module), "Module {} cannot be unloaded because it is not loaded!", module)
        CoreAssert(CanUnload(module), "Module {} cannot be unloaded because of loaded dependants!", module)

        _modulesToUnload.push_back(module);
    }

    std::vector<Module> ModuleContext::LoadModuleWithDependenciesAtBeginOfFrame(const Module& module) {
        auto res = std::vector<Module>();

        auto load = DependencyGraph().AllNodesFromNodeToStartTopological(module);
        std::reverse(load.begin(), load.end());
        for (const auto& toLoad : load) {
            if (!ModuleIsStillLoadedNextFrame(toLoad)) {
                LoadModuleAtBeginOfFrame(toLoad);
                res.push_back(toLoad);
            }
        }

        return std::vector<Module>();
    }

    std::vector<Module> ModuleContext::UnloadModuleWithDependantsAtEndOfFrame(const Module& module) {
        auto res = std::vector<Module>();

        auto unload = DependencyGraph().AllNodesFromNodeToEndTopological(module);
        std::reverse(unload.begin(), unload.end());
        for (const auto& toUnload : unload) {
            if (ModuleIsStillLoadedNextFrame(toUnload)) {
                UnloadModuleAtEndOfFrame(toUnload);
                res.push_back(toUnload);
            }
        }
        return res;
    }



    void ModuleContext::OnPreUpdate() {
        auto& ctx = Context::Instance();
        if (!_modulesToLoad.empty()) {
            ctx.onBeforeLoadModules(_modulesToLoad);
            for (auto& module : _modulesToLoad) {
                ctx.onBeforeLoadModule(module);
                initializeAndAddModule(module);
                ctx.onAfterLoadModule(module);
            }
            ctx.onAfterLoadModules(_modulesToLoad);

            _modulesToLoad.clear();
        }
    }

    void ModuleContext::OnPostUpdate() {
        auto& ctx = Context::Instance();
        if (!_modulesToUnload.empty()) {
            ctx.onBeforeUnloadModules(_modulesToUnload);
            for (auto& module : _modulesToUnload) {
                ctx.onBeforeUnloadModule(module);
                removeAndShutdownModule(module);
                ctx.onAfterUnloadModule(module);
            }
            ctx.onAfterUnloadModules(_modulesToUnload);

            _modulesToUnload.clear();
        }
    }

    shared<ModuleResources> ModuleContext::initializeAndAddModule(const Module& module) {

        auto moduleResources = std::make_shared<ModuleResources>(InfoOf(module));

        moduleResources->initialize();
        _loadedModules.emplace(module, moduleResources);

        return moduleResources;
    }

    void ModuleContext::removeAndShutdownModule(const Module& module) {
        auto moduleResources = _loadedModules.at(module);

        _loadedModules.erase(module);

        moduleResources->shutdown();
    }

    void ModuleContext::OnShutdown() {
        auto& ctx = Context::Instance();
        auto toUnload = _moduleDependencies.AllNodesFromEndToStartTopological();
        ctx.onBeforeUnloadModules(toUnload);
        for(auto& module : toUnload)
            if(ModuleIsCurrentlyLoaded(module))
                removeAndShutdownModule(module);
    }
}
