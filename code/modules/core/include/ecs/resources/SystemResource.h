/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include <ecs/systems/System.h>
#include "resources/Resource.h"
#include "Context.h"
#include "ecs/ECSContext.h"
#include "ecs/systems/DefaultSystemsGroups.h"

namespace modulith {
    /**
     * A resource that registers a single system to the ECSContext while the registering code is loaded
     * @tparam TSystem The type of system to register. Only one system per type may be registered
     * @tparam TInGroup A specified template of InGroup that determines which group the system belongs to
     * @tparam TExecuteAfter A specified template of ExecuteAfter that determines which systems TSystem is execucted after
     * @tparam TExecuteBefore A specified template of ExecuteBefore that determines which systems TSystem is executed before
     */
    template<class TSystem, class TInGroup = InGroup<LogicSystemsGroup>, class TExecuteAfter = ExecuteAfter<>, class TExecuteBefore = ExecuteBefore<>>
    class SystemResource : public Resource{
    public:

        /**
         * Creates a systems resource by constructing the system of type TSystem
         * @tparam TArgs The constructor parameter types (inferred)
         * @param args The constructor parameter values
         */
        template<class... TArgs>
        explicit SystemResource(TArgs... args){
            _system = std::make_shared<TSystem>(args...);
        }

        void OnLoad(std::string description) override {
            Context::GetInstance<ECSContext>()->RegisterSystem(_system, TInGroup());
        }

        void OnInitializeAfterLoad(std::string description) override {
            Context::GetInstance<ECSContext>()->RegisterSystemExecutionOrder(_system, TExecuteAfter(), TExecuteBefore());
            _system->OnInitialize();
        }

        void OnShutdownBeforeUnload(std::string description) override {
            // When unloading a module all systems are shut down anyways, so this is not needed for the time being...
            //_system->OnShutdown(ctx);
        }

        void OnUnload(std::string description) override {
            Context::GetInstance<ECSContext>()->DeregisterSystem<TSystem>();
            _system.reset();
        }

        int GetPriority() override { return 1; }

    private:
        shared<TSystem> _system;
    };
}