//
// Created by Daniel on 19/10/2019.
//

#pragma once

#include "Core.h"
#include "Context.h"

namespace modulith {

    class Engine;

    /**
     * The entry point of the engine's executable.
     * An implementation must be provided from outside
     * and return the a subclass of Engine.
     * @return An instance of a subclass of Engine,
     * which will receive engine lifetime events.
     */
    Engine* CreateEngineInstance();

    /**
     * This abstract class is the entry point for the engine's executable.
     * When linking against the ModulithEngine's library,
     * the executable must provide a class that inherits from Engine.
     *
     * Since this library contains the main() function,
     * the subclass then needs to implement the CreateInstance() method,
     * which will be the executable's entry point.
     */
    class ENGINE_API Engine {
    public:
        friend class Context;

        /**
         * Creates an instance of the engine
         * @param applicationName The name of the application, shown in the application's window
         */
        explicit Engine(std::string  applicationName);

        virtual ~Engine();

        /**
         * Call this method to start the lifecycle of the engine.
         * Once this method returns, the engine has been shut down.
         */
        void Run();

    protected:

        /**
         * Constructs and registers the Resource for this Engine.
         * These resources must be registered before Run() is called
         * and persist for as long as the Engine runs.
         *
         * Resources needn't be deregistered.
         * @tparam TResource The type of resource to register. It must derive from
         * @tparam TArgs The argument types needed to construct the resource
         * @param args The argument values needed to construct the resource
         */
        template<class TResource, class... TArgs, class = std::enable_if_t<std::is_base_of_v<Resource, TResource>, int>>
        void Register(TArgs... args){
            CoreAssert(!_isInitialized, "Engine Resources may only be registered before Run() is called")
            _engineResources.push_back(std::make_shared<TResource>(args...));
        }

        /**
         * Called when the engine is first started.
         * Can be overwritten for initialization logic.
         */
        virtual void OnInitialize() {}

        /**
         * Called during every frame.
         * Can be overwritten for update logic.
         */
        virtual void OnUpdate(float deltaTime) {}


        /**
         * Called when the engine is shut down.
         * Can be overwritten for shutdown logic.
         */
        virtual void OnShutdown() {}

    private:
        std::vector<shared<Resource>> _engineResources{};

        std::chrono::high_resolution_clock::time_point _previousFrameTime;
        bool _isInitialized = false;
        bool _isRunning = true;
        std::string _applicationName;
    };
}
