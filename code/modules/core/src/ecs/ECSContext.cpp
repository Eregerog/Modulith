/**
 * \brief
 * \author Daniel Götz
 */

#include "ecs/ECSContext.h"
#include "ecs/Prefab.h"
#include "Context.h"

namespace modulith{


    ECSContext::ECSContext() : Subcontext("ECS Context"), _componentManager(std::make_unique<ComponentManager>()), _manager(std::make_unique<EntityManager>(ref(&_componentManager))) {
    }


    template<class Fn>
    void ECSContext::executeOnSystemsInOrder(Fn fn) {
        for(auto& groupHash : _systemGroupExecutionOrder.AllNodesFromStartToEndTopological()){
            _registeredSystemGroups.at(groupHash)->ExecuteInOrder(fn);
        }
    }

    shared<Prefab> ECSContext::CreatePrefab(Entity entity) {
        return Prefab::Create(ref(&_componentManager), *_manager, entity);
    }

    void ECSContext::OnInitialize() {
        executeOnSystemsInOrder([](auto& system) { system->OnInitialize(); });
    }

    void ECSContext::OnUpdate(float deltaTime) {
        auto& ctx = Context::Instance();
        executeOnSystemsInOrder(
            [deltaTime, &ctx](auto& system) {
                ctx.GetProfiler().BeginMeasurement(system->GetName() + ".OnUpdate()");
                system->OnUpdate(deltaTime);
                ctx.GetProfiler().EndMeasurement();
            }
        );
    }

    void ECSContext::OnImGui(float deltaTime, bool renderingToImguiWindow) {
        if(renderingToImguiWindow) {
            auto& ctx = Context::Instance();
            executeOnSystemsInOrder(
                [deltaTime, &ctx](auto& system) {
                    ctx.GetProfiler().BeginMeasurement(system->GetName() + ".OnImGui()");
                    system->OnImGui(deltaTime);
                    ctx.GetProfiler().EndMeasurement();
                }
            );
        }
    }

    void ECSContext::OnPostUpdate() {
        _manager->OnEndOfFrame();
    }

    void ECSContext::OnShutdown() {
        executeOnSystemsInOrder([](auto& system) { system->OnShutdown(); });
        // When shutting down, ensure that all entities and components are destroyed.
        // This must be done manually, because modules may not be loaded anymore after shutdown!
        _manager.reset();
    }

    void ECSContext::OnBeforeUnloadModules(const std::vector<Module>& modules) {
        // TODO: Investigate if we need to really shut down all systems and restart them
        executeOnSystemsInOrder([](auto& system){ system->OnShutdown(); });

        // TODO: Replace this with a serilization -> deserialization once it exists
        _manager.reset();
    }

    void ECSContext::OnAfterUnloadModules(const std::vector<Module>& modules) {
        _manager = std::make_unique<EntityManager>(ref(&_componentManager));
        executeOnSystemsInOrder([](auto& system){ system->OnInitialize(); });
    }
}
