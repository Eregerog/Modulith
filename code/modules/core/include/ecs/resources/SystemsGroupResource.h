/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include "resources/Resource.h"
#include "Context.h"
#include "ecs/ECSContext.h"
#include "ecs/systems/SystemsGroup.h"

namespace modulith {
    /**
     * A module resource that registers a single systems group to the ECSContext while the module is loaded
     * @tparam TGroup The type of group to register. Only one group per type may be registered
     * @tparam TExecuteAfter A specified template of ExecuteAfter that determines which groups TGroup is execucted after
     * @tparam TExecuteBefore A specified template of ExecuteBefore that determines which groups TGroup is executed before
     */
    template<class TGroup, class TExecuteAfter = ExecuteAfter<>, class TExecuteBefore = ExecuteBefore<>>
    class SystemsGroupResource : public Resource{
    public:

        template<class... TArgs>
        explicit SystemsGroupResource(TArgs... args){
            _group = std::make_shared<TGroup>(args...);
        }

        void OnLoad(std::string description) override {
            Context::GetInstance<ECSContext>()->RegisterSystemsGroup(_group);
        }

        void OnInitializeAfterLoad(std::string description) override {
            Context::GetInstance<ECSContext>()->RegisterSystemsGroupExecutionOrder(_group, TExecuteAfter(), TExecuteBefore());
        }

        void OnShutdownBeforeUnload(std::string description) override {
            // unused
        }

        void OnUnload(std::string description) override {
            Context::GetInstance<ECSContext>()->DeregisterSystemsGroup<TGroup>();
            _group.reset();
        }

        int GetPriority() override { return 2; }

    private:
        shared<TGroup> _group;
    };
}