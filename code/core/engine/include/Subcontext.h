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
     * Subclasses of a subcontext can be registered at the context to receive game loop callbacks.
     * Only one subcontext per type may be registered
     */
    class ENGINE_API Subcontext {
    public:
        /**
         * Constructs a subcontext from a name used for debugging
         * @param name
         */
        explicit Subcontext(std::string name) : _name(std::move(name)) {}

        /**
         * @return Returns the subcontext's name
         */
        std::string GetName() { return _name; }

        /**
         * @name Game Loop Callbacks
         */
        ///@{

        /**
         * Called when the subcontext is first added.
         * Can be overwritten for initialization logic.
         * The base class method does not need to be called.
         */
        virtual void OnInitialize() {  }

        /**
         * Called at the beginning of each frame.
         * Can be overwritten for specific game logic.
         * The base class method does not need to be called.
         */
        virtual void OnPreUpdate() { }


        virtual void OnBeforeUpdate() { }

        /**
         * Called each frame.
         * Can be overwritten for specific game logic.
         * The base class method does not need to be called.
         * @param deltaTime The time it took to complete the last frame
         */
        virtual void OnUpdate(float deltaTime) { }


        virtual void OnAfterUpdate() { }

        virtual void OnBeforeImGui(bool renderingToImguiWindow) { }

        /**
         * Called each frame.
         * ImGui methods may be called inside of this method to create windows etc..
         * The base class method does not need to be called.
         * @param deltaTime The time it took to complete the last frame
         * @param renderingToImguiWindow
         * True when the application's OnUpdate will be rendering to an imgui subwindow.
         * It is recommended that imgui windows are only shown while this is true,
         * because otherwise docking is disabled and windows may overlap the game.
         */
        virtual void OnImGui(float deltaTime, bool renderingToImguiWindow) {  }

        virtual void OnAfterImGui(bool renderingToImguiWindow) { }

        /**
         * Called at the end of each frame.
         * Can be overwritten for specific game logic.
         * The base class method does not need to be called.
         */
        virtual void OnPostUpdate() { }

        /**
         * Called when the subcontext is removed or when the application is shut down.
         * Can be overwritten for shutdown logic
         * The base class method does not need to be called.
         */
        virtual void OnShutdown() {  }

        ///@}


        /**
         * @name Module Loading Callbacks
         */
        ///@{


        /**
         * Called just before a module is loaded this frame.
         * The base class method does not need to be called.
         * @param module The module that will be loaded
         */
        virtual void OnBeforeLoadModule(const Module& module) { }

        /**
         * Called just after a module is loaded this frame.
         * The base class method does not need to be called.
         * @param module The module that was loaded
         */
        virtual void OnAfterLoadModule(const Module& module) { }

        /**
         * Called just before modules are loaded this frame.
         * The base class method does not need to be called.
         * @param modules The modules that will be loaded
         */
        virtual void OnBeforeLoadModules(const std::vector<Module>& modules) { }

        /**
         * Called just after all modules have been loaded this frame.
         * The base class method does not need to be called.
         * @param modules The modules that were loaded
         */
        virtual void OnAfterLoadModules(const std::vector<Module>& modules) { }



        /**
         * Called just before a module is unloaded this frame.
         * The base class method does not need to be called.
         * @param module The module that will be unloaded
         */
        virtual void OnBeforeUnloadModule(const Module& module) { }

        /**
         * Called just after a module is unloaded this frame.
         * The base class method does not need to be called.
         * @param module The module that was unloaded
         */
        virtual void OnAfterUnloadModule(const Module& module) { }

        /**
         * Called just before modules are unloaded this frame.
         * The base class method does not need to be called.
         * @param modules The modules that will be unloaded
         */
        virtual void OnBeforeUnloadModules(const std::vector<Module>& modules) { }

        /**
         * Called just after all modules have been unloaded this frame.
         * The base class method does not need to be called.
         * @param modules The modules that were unloaded
         */
        virtual void OnAfterUnloadModules(const std::vector<Module>& modules) { }

        ///@}

    private:
        std::string _name;
    };
}