/**
 * \brief
 * \author Daniel Götz
 */

#include <ecs/StandardComponents.h>
#include <ecs/transform/TransformComponents.h>
#include <ecs/ComponentManager.h>

namespace modulith{

    void ComponentManager::RegisterComponents(const std::vector<shared<ComponentInfo>>& componentInfos){
        for(auto& component : componentInfos)
            RegisterComponent(component);
    }

    void ComponentManager::RegisterComponent(const shared<ComponentInfo>& componentInfo) {
        CoreAssert(componentInfo->_identifier, "Cannot register a component info that is invalid!")
        auto& identifier = componentInfo->_identifier.value();
        if (_registeredComponents.count(identifier) > 0)
            return;

        // the first component gets index 1, therefore ++ before the variable
        auto info = RegisteredComponent(
            componentInfo,
            ++_componentCount
        );
        _registeredComponents[identifier] = std::move(info);
    }

    void ComponentManager::DeregisterComponent(const shared<ComponentInfo>& componentInfo) {
        CoreAssert(componentInfo->_identifier, "Cannot deregister a component info that is invalid!")
        _registeredComponents.erase(componentInfo->_identifier.value());
    }


    Signature ComponentManager::ToSignature(const SignatureIdentifier& identifier) {
        auto result = Signature();
        for (auto& component : identifier) {
            result.set(_registeredComponents[component].GetIndex());
        }
        return result;
    }

    RegisteredComponent ComponentManager::GetInfoOf(const ComponentIdentifier identifier) {
        return _registeredComponents[identifier];
    }

    size_t ComponentManager::RegisteredComponentCount() const {
        return _registeredComponents.size();
    }

}
