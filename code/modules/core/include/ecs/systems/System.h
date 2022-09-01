/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "CoreModule.h"

namespace modulith {

    class Context;

    /**
     * Subclasses of systems are registered in the context and will receive callbacks while the engine is running.
     * Only one instance of a type of System may be registered
     */
    class CORE_API System {
    public:
        /**
         * Creates a system with the given name, which is used for debugging purposes
         * @param name
         */
        explicit System(::std::string name) : _name(::std::move(name)) {}

        virtual ~System() = default;

        /**
         * @return Returns the module's name
         */
        const ::std::string& GetName() { return _name; }

        /**
         * Called when the system is first registered but before its first OnUpdate call.
         * Can be overwritten for initialization logic
         */
        virtual void OnInitialize() {}

        /**
         * Called every frame while the system is registered.
         * Can be overwritten for game update logic
         * @param deltaTime The time of the last frame
         */
        virtual void OnUpdate(float deltaTime) {}

        /**
         * Called every frame while the system is registered and imgui is enabled.
         * Can be overwritten to render imgui windows.
         * @param deltaTime The time of the last frame
         */
        virtual void OnImGui(float deltaTime) {}

        /**
         * Called when the system is deregistered or the application is shutdown.
         * Can be overwritten for shutdown logic
         */
        virtual void OnShutdown() {}

    private:
        ::std::string _name;
    };
};
