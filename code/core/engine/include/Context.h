/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"
#include <profiling/Profiler.h>
#include "modules/ModuleContext.h"
#include "utils/TypeUtils.h"
#include "Subcontext.h"

namespace modulith{
    class Engine;
    class System;
    class Renderer;
    class ModuleResources;

    /**
     * The context class manages the lifetime of and contains reference to all the subcomponents of the engine.
     * There should only ever be once instance of the Context, which is managed by the engine.
     * It can be accessed using the Instance() method.
     * All Subcontext can be accessed using the GetInstance() method
     */
    class ENGINE_API Context {

        friend Engine;
        friend ModuleContext;
        friend Renderer;
        friend ModuleResources;

    public:

        /**
         * @return Returns the (singleton) instance of the context
         */
        static Context& Instance();

        /**
         * Alias for Instance().Get<Subcontext>()
         * Can be used to get a registered subcontext
         */
        template<class T>
        static shared<T> GetInstance() { return Instance().Get<T>(); }

        /**
         * Creates a context object
         * @param profiler The profiler of the context
         * @remark Since there should only ever be one Context,
         *         creating one's own context is discouraged
         */
        explicit Context(owned<modulith::Profiler> profiler);

        template<class T>
        shared<T> Get() { return std::dynamic_pointer_cast<T>(_subcontexts.at(typeid(T).hash_code())); }

        /**
         * @return Returns the current application's profiler
         * @see Profiler
         */
        modulith::Profiler& GetProfiler() { return *_profiler; }

        /**
         * Registers the given subcontext to the context to receive game event callbacks
         * @tparam TSubcontext The type of subcontext to register, which has to derive from Subcontext. Only one subcontext per type may be registered
         * @param subcontext The value of subcontext to register
         * @remark OnInitialize must be called on the subcontext manually
         */
        template<class TSubcontext, class = std::enable_if_t<std::is_base_of_v<Subcontext, TSubcontext>>>
        void RegisterSubcontext(shared<TSubcontext> subcontext){
            auto sctxHash = typeid(TSubcontext).hash_code();
            CoreAssert(_subcontexts.count(sctxHash) == 0,
                "Cannot register a subcontext of type {} because one of that type is already registered",
                typeid(TSubcontext).name()
            )
            _subcontexts.emplace(sctxHash, subcontext);
        }

        /**
         * Deregisters an already registered subcontext
         * @tparam TSubcontext The type of subcontext to deregister. It must derive from Subcontext.
         * @remark OnShutdown must be called on the subcontext manually
         */
        template<class TSubcontext, class = std::enable_if_t<std::is_base_of_v<Subcontext, TSubcontext>>>
        void DeregisterSubcontext(){
            auto sctxHash = typeid(TSubcontext).hash_code();
            CoreAssert(_subcontexts.count(sctxHash) > 0,
                "Cannot deregister a subcontext of type {} because one of that type is not registered",
                typeid(TSubcontext).name()
            )
            _subcontexts.erase(sctxHash);
        }


        /**
         * @return Returns whether ImGui rendering is currently enabled
         */
        [[nodiscard]] bool IsImGuiEnabled() const { return _imGuiEnabled; }

        void SetImGuiEnabled(bool enabled) { _imGuiEnabled = enabled; }

        void RequestShutdown() { _isRunning = false; }

    private:
        void onInitialize();

        void onPreUpdate();

        void onUpdate(float deltaTime);

        void onImGui(float deltaTime);

        void onPostUpdate();

        void onShutdown();

        void onBeforeLoadModule(const Module& module);
        void onAfterLoadModule(const Module& module);
        void onBeforeLoadModules(std::vector<Module> modules);
        void onAfterLoadModules(std::vector<Module> modules);

        void onBeforeUnloadModule(const Module& module);
        void onAfterUnloadModule(const Module& module);
        void onBeforeUnloadModules(std::vector<Module> modules);
        void onAfterUnloadModules(std::vector<Module> modules);

        template<class Fn>
        void forEachSubcontext(Fn fn);

        // Subcontexts

        owned<modulith::Profiler> _profiler;

        bool _imGuiEnabled = false;
        bool _isRunning = true;


        PersistentTypeMap<shared<Subcontext>> _subcontexts{};
    };
}
