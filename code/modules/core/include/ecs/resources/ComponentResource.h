/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include "resources/Resource.h"
#include "Context.h"
#include "ecs/ECSContext.h"

namespace modulith {

    /**
     * A resource that registers a single component type to the ComponentManager while the registering code is loaded
     * @tparam TComponent The type of component to register. Every type may only be registered once
     */
    template<class TComponent>
    class ComponentResource : public Resource {
    public:
        /**
         * Creates a component resource of type TComponent with a debug name for the component
         * @param componentName The debug name of the component
         */
        explicit ComponentResource(const std::string& componentName) : _componentName(componentName) {}

        void OnLoad(std::string description) override {
            _registeredComponents = ComponentInfo::Create<TComponent>(description, _componentName);
            Context::GetInstance<ECSContext>()->GetComponentManager()->RegisterComponents(_registeredComponents);
        }

        void OnInitializeAfterLoad(std::string description) override {
            // unused
        }

        void OnShutdownBeforeUnload(std::string description) override {
            // unused
        }

        void OnUnload(std::string description) override {
            auto ecsCtx = Context::GetInstance<ECSContext>();
            for (const auto& component : _registeredComponents)
                ecsCtx->GetComponentManager()->DeregisterComponent(component);
            _registeredComponents.clear();
        }

        int GetPriority() override { return 3; }

    private:
        std::string _componentName;
        std::vector<shared<ComponentInfo>> _registeredComponents;
    };
}