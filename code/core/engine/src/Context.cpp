/*
 * \brief
 * \author Daniel Götz
 */

#include <Context.h>

namespace modulith{

    Context::Context(owned<modulith::Profiler> profiler) : _profiler(std::move(profiler)) {   }

    template<class Fn>
    void Context::forEachSubcontext(Fn fn){
        for(auto& pair : _subcontexts)
            fn(pair.second);
    }

    void Context::onInitialize() {
        forEachSubcontext([](auto subcontext){ subcontext->OnInitialize(); });
    }

    void Context::onPreUpdate() {
        forEachSubcontext([](auto subcontext){ subcontext->OnPreUpdate(); });
    }


    void Context::onUpdate(float deltaTime) {
        forEachSubcontext([](auto subcontext){ subcontext->OnBeforeUpdate(); });
        forEachSubcontext([deltaTime](auto subcontext){ subcontext->OnUpdate(deltaTime); });
        forEachSubcontext([](auto subcontext){ subcontext->OnAfterUpdate(); });
    }

    void Context::onImGui(float deltaTime) {
        forEachSubcontext([this](auto subcontext){ subcontext->OnBeforeImGui(_imGuiEnabled); });
        forEachSubcontext([this, deltaTime](auto subcontext){ subcontext->OnImGui(deltaTime, _imGuiEnabled); });
        forEachSubcontext([this](auto subcontext){ subcontext->OnAfterImGui(_imGuiEnabled); });
    }

    void Context::onPostUpdate() {
        forEachSubcontext([](auto subcontext){ subcontext->OnPostUpdate(); });
        GetProfiler().ClearMeasurements();
    }

    void Context::onShutdown() {
        forEachSubcontext([](auto subcontext){ subcontext->OnShutdown(); });
        _subcontexts.clear();
    }

    void Context::onBeforeLoadModules(const std::vector<Module> modules) {
        forEachSubcontext([modules](auto subcontext){ subcontext->OnBeforeLoadModules(modules); });
    }

    void Context::onAfterLoadModules(const std::vector<Module> modules) {
        forEachSubcontext([modules](auto subcontext){ subcontext->OnAfterLoadModules(modules); });
    }

    void Context::onBeforeUnloadModules(const std::vector<Module> modules) {
        forEachSubcontext([modules](auto subcontext){ subcontext->OnBeforeUnloadModules(modules); });
    }

    void Context::onAfterUnloadModules(const std::vector<Module> modules) {
        forEachSubcontext([modules](auto subcontext){ subcontext->OnAfterUnloadModules(modules); });
    }

    void Context::onBeforeLoadModule(const Module& module) {
        forEachSubcontext([&module](auto subcontext){ subcontext->OnBeforeLoadModule(module); });
    }

    void Context::onAfterLoadModule(const Module& module) {
        forEachSubcontext([&module](auto subcontext){ subcontext->OnAfterLoadModule(module); });
    }

    void Context::onBeforeUnloadModule(const Module& module) {
        forEachSubcontext([&module](auto subcontext){ subcontext->OnBeforeUnloadModule(module); });
    }

    void Context::onAfterUnloadModule(const Module& module) {
        forEachSubcontext([&module](auto subcontext){ subcontext->OnAfterUnloadModule(module); });
    }
}
