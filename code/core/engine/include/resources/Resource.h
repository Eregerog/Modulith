/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "Core.h"
#include "modules/Module.h"

namespace modulith{

    class Context;

    /**
     * Resources contain setup and shutdown logic for objects / code with a lifetime tied to its registering system.
     * E.g. when a Resource is Registered by a Module, the object / code it
     * manages will exist as long a s the module is loaded.
     * They are initialized / added when the registering system is started and
     * shutdown / removed when the system is removed.
     *
     * These can be registered during a module's initialization process
     * using { @link ModuleResources.Register } and their
     * callbacks will be automatically called.
     *
     * They are also used by the engine to register Engine Resources
     * that persist for as long as the engine is running.
     */
    class ENGINE_API Resource {
    public:

        /**
         * Sorts the given list of resources by priority (in descending order)
         * And first calls all OnLoad() methods on them, followed by calling OnInitializeAfterLoad() on all of them
         * @param resources The resources to invoke the callbacks on
         * @param description A description for the registering system the resources are tied to
         */
        static void InvokeLoadCallbacksOn(std::vector<shared<Resource>>& resources, const std::string& description);

        /**
         * Sorts the given list of resources by priority (in ascending order)
         * And first calls all OnShutdownBeforeUnload() methods on them, followed by calling OnUnload() on all of them
         * @param resources The resources to invoke the callbacks on
         * @param description A description for the registering system the resources are tied to
         */
        static void InvokeUnloadCallbacksOn(std::vector<shared<Resource>>& resources, const std::string& description);

        virtual ~Resource() = default;

        /**
         * Called when the registering system has been loaded.
         * Should be overwritten for most initialization logic.
         * @param description A description for the registering system this resource is tied to
         */
        virtual void OnLoad(std::string description) = 0;

        /**
         * Called when the registering system has been loaded and all resources received their OnLoad callback.
         * Should be overwritten for late initialization logic that may depend on other resources.
         * @param description A description for the registering system this resource is tied to
         */
        virtual void OnInitializeAfterLoad(std::string description) = 0;

        /**
         * Called when the registering system will be unloaded and before any OnUnload callback is called.
         * Should be overwritten for early shutdown logic that may depend on other resources.
         * @param description A description for the registering system this resource is tied to
         */
        virtual void OnShutdownBeforeUnload(std::string description) = 0;

        /**
         * Called when the registering system will be unloaded and after all OnShutdownBeforeUnload callbacks are called.
         * Should be overwritten for most shutdown logic.
         * @param description A description for the registering system this resource is tied to
         */
        virtual void OnUnload(std::string description) = 0;

        /**
         * Higher priority resources will be loaded first and unloaded last.
         * Resources with the same priority are loaded in order of registration
         * and unloaded in the reverse order.
         */
        virtual int GetPriority() = 0;
    };
}