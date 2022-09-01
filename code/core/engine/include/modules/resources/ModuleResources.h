/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"
#include "modules/ModuleInfo.h"
#include "resources/Resource.h"

namespace modulith {
    class ModuleContext;

    /**
     * This class represents a module's code resources that need to be managed as part of a module's lifetime.
     * It is used by the engine to track all the module's components and systems.
     * Furthermore, it is used by the module code itself to register these systems and components.
     */
    class ENGINE_API ModuleResources {

        enum ModuleStatus {
            UNINITIALIZED, // The initial state of a module, before it is initialized
            INITIALIZING, // During this state the module's Initialize() function is called an it can register components / systems
            INITIALIZED, // During this state the module is initialized and can be used in the engine
            SHUTTING_DOWN // During this state the module's Shutdown() function is called
        };

        friend ModuleContext;

    public:

        /**
         * @warning This constructor is only public so unique pointers can be made of this class and should therefore not be used.
         */
        explicit ModuleResources(ModuleInfo info);

        /**
         * Registers a ModuleResource which will receive appropriate callbacks.
         * May only be called during a module's Initialize() function.
         * @tparam TResource The type of ModuleResource to register
         * @tparam TArgs The type arguments needed to construct this resource
         * @param args The arguments used to construct the resource
         */
        template<class TResource, class... TArgs>
        void Register(TArgs... args){
            Assert(_status == ModuleStatus::INITIALIZING,
                "Module {} resources may only be registered while it is Initializing!", _info.Name);
            _resources.push_back(std::make_shared<TResource>(args...));
        }

        /**
         * Registers a ModuleResource which will receive appropriate callbacks.
         * May only be called during a module's Initialize() function.
         * @param resource The resource to register
         */
        void Register(shared<Resource> resource){
            Assert(_status == ModuleStatus::INITIALIZING,
                "Module {} resources may only be registered while it is Initializing!", _info.Name);
            _resources.push_back(std::move(resource));
        }

        /**
         * @return Returns the info of this module
         */
        ModuleInfo GetInfo() { return _info; }

    private:

        void initialize();

        void initializeModuleCode();

        void shutdown();

        void shutdownModuleCode();

        void freeModuleCode();

        template<class FnType, class... Args>
        void findAndExecuteFunctionWith(const std::string& fnName, Args... args);

        ModuleStatus _status = ModuleStatus::UNINITIALIZED;

        ModuleInfo _info;

        std::vector<shared<Resource>> _resources{};

        std::any _libraryHandle{};
    };
}
