/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"
#include <utils/boolinq/boolinq.h>
#include <Subcontext.h>
#include "modules/resources/ModuleResources.h"
#include "utils/DependencyGraph.h"

namespace modulith {

    class Context;

    /**
     * The module context is responsible for loading / unloading modules and the dependencies between them.
     */
    class ENGINE_API ModuleContext : public Subcontext {

        friend Context;

    public:

        /**
         * @warning This is only public for creating a unique pointer.
         * The user should always use the module context provided my the context!
         */
        ModuleContext();

        /**
         * @return Handles for every module in the "module" folder of the currently executed binary that are valid
         */
        std::vector<Module> GetAvailableModules() {
            return boolinq::from(_availableModules)
                .select(
                    [](auto& pair) { return pair.first; }
                )
                .orderBy([this](auto module) { return _moduleDependencies.MaxDistanceFromStart(module); })
                .toStdVector();
        }

        /**
         * @return Handles for every currently loaded module
         * @remark The returned handles may not be included in {@link GetAvailableModules()}
         *         if they have become invalid since first loaded
         */
        std::vector<Module> GetLoadedModules() {
            return boolinq::from(_loadedModules).select(
                [](auto& pair) { return pair.first; }
            ).toStdVector();
        }

        /**
         * @return Whether the provided module handle is a valid module that can be loaded
         */
        [[nodiscard]] bool ModuleExists(const Module& module) const {
            return _availableModules.count(module) > 0;
        }

        /**
         * @return Whether the provided module handle is loaded for this frame
         * @remark This method does not check if the module will be unloaded at
         * the end of the frame or loaded at the begin of the next frame.
         * It is therefore recommended to use {@link ModuleIsStillLoadedNextFrame(const Module&)}
         */
        [[nodiscard]] bool ModuleIsCurrentlyLoaded(const Module& module) const { return _loadedModules.count(module); }

        /**
         * @return Whether the provided module handle is currently loaded or will (still) be loaded next frame
         */
        [[nodiscard]] bool ModuleIsStillLoadedNextFrame(const Module& module) const {
            return (ModuleIsCurrentlyLoaded(module) && !WillBeUnloaded(module)) || WillBeLoaded(module);
        }

        /**
         * Tries to find a valid module handle with the given name.
         * @return A module handle with a matching name, or std::nullopt if none was found.
         */
        [[nodiscard]] std::optional<Module> ModuleFromName(const std::string& name) {
            for (auto module : GetAvailableModules()) {
                if (module.Name() == name)
                    return module;
            }
            return std::nullopt;
        }

        /**
         * @param module A module that must be valid.
         * @return The Info of that module
         */
        [[nodiscard]] ModuleInfo InfoOf(const Module& module) const {
            CoreAssert(ModuleExists(module), "Cannot get the info of a module ({}) that does not exist.", module.Name())
            return _availableModules.at(module);
        }

        /**
         * @name Module Loading
         * Contains methods for querying if a module is / can be loaded or unloaded
         * and methods for loading or unloading them.
         * Module loading / unloading is deferred between the current and next frame,
         * so current systems can run to their end.
         * It is not possible to unload / load a module immediately!
         */
        ///@{

        /**
         * @return Will the provided module be loaded at the begin of the next frame?
         * @remark This is true for any modules that {@link LoadModuleAtBeginOfFrame(Module)} was called on this frame
         */
        [[nodiscard]] bool WillBeLoaded(const Module& module) const { return boolinq::from(_modulesToLoad).contains(module); }

        /**
         * @return Will the provided module be unloaded at the end of the current frame?
         * @remark This is true for any modules that {@link UnloadModuleAtEndOfFrame(Module)} was called on this frame
         */
        [[nodiscard]] bool WillBeUnloaded(const Module& module) const { return boolinq::from(_modulesToUnload).contains(module); }

        /**
         * @return Whether the provided module can currently be loaded by calling {@link LoadModuleAtBeginOfFrame(Module)}
         * @remark This is true if the module will not be loaded at the begin of
         *         the next frame, isn't yet scheduled to be loaded
         *         and if all its dependencies will then be loaded.
         */
        [[nodiscard]] bool CanLoad(const Module& module) const {
            return !ModuleIsStillLoadedNextFrame(module) && boolinq::from(_moduleDependencies.AllPrevsOf(module)).all(
                [this](auto& dependency) { return ModuleIsStillLoadedNextFrame(dependency); }
            );
        }

        /**
         * @return Whether the provided module can currently be unloaded by calling {@link UnloadModuleAtEndOfFrame(Module)}
         * @remark This is true if the module is currently loaded, isn't yet scheduled to be unloaded
         *         and no modules depending on it would be present at the begin of next frame.
         */
        [[nodiscard]] bool CanUnload(const Module& module) const {
            return ModuleIsStillLoadedNextFrame(module) && boolinq::from(_moduleDependencies.AllNextsOf(module)).all(
                [this](auto& dependant) { return !ModuleIsStillLoadedNextFrame(dependant); }
            );
        }

        /**
         * The provided module will be loaded at the begin of the next frame,
         * calling its Initialize function and registering all its systems and components.
         * @param module The loaded module for which {@link CanLoad(Module)} must return true
         */
        void LoadModuleAtBeginOfFrame(const Module& module);

        /**
         * The provided module will be unloaded at the end of the current frame,
         * calling its Shutdown function and deregistering all its systems and components.
         * @param module The unloaded module for which {@link CanUnload(Module)} must return true
         */
        void UnloadModuleAtEndOfFrame(const Module& module);

        /**
         * Calling this method loads the given module at the start of the next frame.
         * If any of its direct or indirect dependencies are not loaded,
         * they will first be loaded in the correct order.
         * @param module A valid module which will not be loaded at the begin of the next frame.
         * @return A list that contains the modules that were loaded in order, including the parameter.
         */
        std::vector<Module> LoadModuleWithDependenciesAtBeginOfFrame(const Module& module);

        /**
         * Calling this method unloads the given module at the end of the current frame.
         * If any of its direct or indirect dependants are still loaded,
         * they will first be unloaded in the correct order.
         * @param module A valid module which will still be loaded at the end of the frame.
         * @return A list that contains the modules that were unloaded in order, including the parameter.
         */
        std::vector<Module> UnloadModuleWithDependantsAtEndOfFrame(const Module& module);

        ///@}

        /**
         * Calling this method will refresh the list of all available modules and the dependencies between them.
         *
         * @remark This could have the effect of a module that was previously valid (and part of that list)
         *         becoming invalid and being omitted from that list.
         */
        void RefreshAvailableModules();

        /**
         * @return A read-only dependency graph that describes the dependencies between all available modules.
         */
        [[nodiscard]] const DependencyGraph<Module>& DependencyGraph() const { return _moduleDependencies; }

        void OnPreUpdate() override;

        void OnPostUpdate() override;

        void OnShutdown() override;

    private:

        shared<ModuleResources> initializeAndAddModule(const Module& module);

        void removeAndShutdownModule(const Module& module);

        std::unordered_map<Module, ModuleInfo> _availableModules{};
        std::unordered_map<Module, shared<ModuleResources>> _loadedModules{};

        std::vector<Module> _modulesToUnload{};
        std::vector<Module> _modulesToLoad{};

        ::modulith::DependencyGraph<Module> _moduleDependencies{};
    };
}
