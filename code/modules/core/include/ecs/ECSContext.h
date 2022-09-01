/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include <utils/TypeUtils.h>
#include <ecs/systems/SystemsGroup.h>
#include <ecs/systems/System.h>
#include "Subcontext.h"
#include "ComponentManager.h"
#include "EntityManager.h"
#include "utils/DependencyGraph.h"
#include "systems/DefaultSystemsGroups.h"

namespace modulith {

    /**
     * The ecs context contains all necessary managers and systems for the ECS.
     */
    class CORE_API ECSContext : public Subcontext {

    public:

        ECSContext();

        /**
        * @tparam TComponents The types of components attached to the prefab. All of these types need to be trivially copy-constructable.
        * @param components The values of components attached to the prefab
        * @return A prefab with the given components
        */
        template<class... TComponents>
        shared<Prefab> CreatePrefab(TComponents&& ... components);

        /**
         * Creates a prefab by copying all component values on the given entity
         * @param entity An entity with only trivially copy-constructable components attached to it
         * @return A prefab that has all the components of the given entity
         */
        shared<Prefab> CreatePrefab(Entity entity);

        /**
        * @tparam T The type of the system to retrieve
        * @return Returns the system of the given type if it is registered, nullopt otherwise
        */
        template<class T>
        std::optional<shared<T>> TryGetSystem() {
            auto sysHash = typeid(T).hash_code();
            if(_systemLocation.count(sysHash) > 0) {
                return _systemLocation.at(sysHash).lock()->TryGetSystem<T>();
            }

            return std::nullopt;
        }

        /**
         * @return Returns the application's component manager.
         * @see ComponentManager
         */
        ref<ComponentManager> GetComponentManager() { return ref(&_componentManager); }

        /**
         * @return Returns the application's entity manager
         * @see EntityManager
         */
        ref<EntityManager> GetEntityManager() { return ref(&_manager); };


        /**
         * @name System & Systems Group Registration
         */
        ///@{

        /**
         * Registers the given systems group.
         * Once registered, it will receive game loop events
         * and systems can be registered in it
         * @tparam TGroup The type of a systems groups that is not yet registered
         * @param systemGroup The systems group to register
         */
        template<class TGroup, class = std::enable_if_t<std::is_base_of_v<SystemsGroup, TGroup>>>
        void RegisterSystemsGroup(shared<TGroup> systemGroup) {
            auto groupHash = typeid(TGroup).hash_code();
            CoreAssert(_registeredSystemGroups.count(groupHash) == 0,
                "Cannot register system group {} because it is already registered",
                systemGroup->GetName())
            _registeredSystemGroups.emplace(groupHash, systemGroup);
            _systemGroupExecutionOrder.Add(groupHash);
        }

        /**
         * For a given systems group, define dependencies to groups that must be executed before / afterwards
         * @tparam TGroup A registered systems group that gets the dependencies
         * @tparam TExecuteAfter Types of registered groups that the group must execute after
         * @tparam TExecuteBefore Types of registered groups that the group must execute before
         */
        template<class TGroup, class... TExecuteAfter, class... TExecuteBefore, class = std::enable_if_t<
            std::is_base_of_v<SystemsGroup, TGroup>
            && std::conjunction_v<std::is_base_of<SystemsGroup, TExecuteBefore>...>
            && std::conjunction_v<std::is_base_of<SystemsGroup, TExecuteAfter>...>
        >>
        void RegisterSystemsGroupExecutionOrder(shared<TGroup>, ExecuteAfter<TExecuteAfter...>, ExecuteBefore<TExecuteBefore...>){
            (RegisterSystemsGroupDependency<TGroup, TExecuteBefore>(), ...);
            (RegisterSystemsGroupDependency<TExecuteAfter, TGroup>(), ...);
        }

        /**
         * Registers an execution dependency between two registered systems groups
         * @tparam TBefore The systems group to execute before the other
         * @tparam TAfter The systems group to execute after the other
         * @remark This mustn't create dependency circles!
         */
        template<class TBefore, class TAfter, class = std::enable_if_t<std::is_base_of_v<SystemsGroup, TBefore> && std::is_base_of_v<SystemsGroup, TAfter>>>
        void RegisterSystemsGroupDependency(){
            auto beforeHash = typeid(TBefore).hash_code();
            auto afterHash = typeid(TAfter).hash_code();
            CoreAssert(_registeredSystemGroups.count(beforeHash) > 0,
                "Cannot register dependency from systems group {} to systems group {}, because the former is not registered",
                typeid(TBefore).name(),
                typeid(TAfter).name()
            )
            CoreAssert(_registeredSystemGroups.count(afterHash) > 0,
                "Cannot register dependency from systems group {} to systems group {}, because the latter is not registered",
                typeid(TBefore).name(),
                typeid(TAfter).name()
            )
            CoreAssert(_systemGroupExecutionOrder.CanAddDependency(beforeHash, afterHash),
                "Cannot add a systems group execution order dependency between {} and {} because it would cause a circular dependency!",
                typeid(TBefore).name(),
                typeid(TAfter).name()
            )
            _systemGroupExecutionOrder.AddDependency(beforeHash, afterHash);
        }

        /**
         * Deregisters a systems groups
         * @tparam TGroup A registered systems group to remove
         * @remark This automatically removes any dependencies to or from this group
         */
        template<class TGroup, class = std::enable_if_t<std::is_base_of_v<SystemsGroup, TGroup>>>
        void DeregisterSystemsGroup() {
            auto groupHash = typeid(TGroup).hash_code();
            CoreAssert(_registeredSystemGroups.count(groupHash) > 0,
                "Cannot deregister systems group {} it isn't registered",
                typeid(TGroup).name())
            _registeredSystemGroups.erase(groupHash);
            _systemGroupExecutionOrder.Remove(groupHash);
        }

        /**
         * Registers a given system to receive game loop callbacks
         * @tparam TSystem The type of the system to register. Its type mustn't be registered yet
         * @tparam TGroup The type of group the system is contained in
         * @param system The system to register
         * @param inGroup The group the system will be contained in
         */
        template<class TSystem, class TGroup,
            class = std::enable_if_t<
                std::is_base_of_v<System, TSystem>
                && std::is_base_of_v<SystemsGroup, TGroup>
            >>
        void RegisterSystem(shared<TSystem> system, InGroup<TGroup> inGroup = InGroup<LogicSystemsGroup>()){
            auto groupHash = typeid(TGroup).hash_code();
            auto sysHash = typeid(TSystem).hash_code();
            CoreAssert(_systemLocation.count(sysHash) == 0,
                "Cannot register system {}, because it already is registered",
                typeid(TSystem).name()
            )
            CoreAssert(_registeredSystemGroups.count(groupHash) > 0,
                "Cannot register system {} in systems group {}, because the group isn't registered",
                typeid(TSystem).name(),
                typeid(TGroup).name()
            )
            auto group = _registeredSystemGroups.at(groupHash);
            group->RegisterSystem(system);
            _systemLocation.emplace(sysHash, group);
        }

        /**
       * For a given system, define dependencies to systems within its group that must be executed before / afterwards
       * @tparam TSystem A registered system that gets the dependencies
       * @tparam TExecuteAfter Types of registered systems that the system must execute after
       * @tparam TExecuteBefore Types of registered systems that the system must execute before
       */
        template<class TSystem, class... TExecuteAfter, class... TExecuteBefore, class = std::enable_if_t<
            std::is_base_of_v<System, TSystem>
            && std::conjunction_v<std::is_base_of<System, TExecuteBefore>...>
            && std::conjunction_v<std::is_base_of<System, TExecuteAfter>...>
        >>
        void RegisterSystemExecutionOrder(shared<TSystem>, ExecuteAfter<TExecuteAfter...>, ExecuteBefore<TExecuteBefore...>){
            (RegisterSystemDependency<TSystem, TExecuteBefore>(), ...);
            (RegisterSystemDependency<TExecuteAfter, TSystem>(), ...);
        }


        /**
         * Registers an execution dependency between two registered systems
         * @tparam TBefore The system to execute before the other
         * @tparam TAfter The system to execute after the other
         * @remark This mustn't create dependency circles!
         */
        template<class TBefore, class TAfter, class = std::enable_if_t<
            std::is_base_of_v<System, TBefore> && std::is_base_of_v<System, TBefore>>>
        void RegisterSystemDependency(){
            auto beforeHash = typeid(TBefore).hash_code();
            auto afterHash = typeid(TAfter).hash_code();
            CoreAssert(_systemLocation.count(beforeHash) > 0,
                "Cannot register dependency from system {} to system {}, because the former is not registered",
                typeid(TBefore).name(),
                typeid(TAfter).name()
            )
            CoreAssert(_systemLocation.count(afterHash) > 0,
                "Cannot register dependency from system {} to system {}, because the latter is not registered",
                typeid(TBefore).name(),
                typeid(TAfter).name()
            )
            CoreAssert(_systemLocation.at(beforeHash).lock() == _systemLocation.at(afterHash).lock(),
                "Cannot register dependency from system {} to system {}, because they are not in the same systems group.\n The first is inside of {} the second is inside of {}",
                typeid(TBefore).name(),
                typeid(TAfter).name(),
                _systemLocation.at(beforeHash).lock()->GetName(),
                _systemLocation.at(afterHash).lock()->GetName()
            )
            _systemLocation.at(beforeHash).lock()->RegisterSystemDependency<TBefore, TAfter>();
        }

        /**
         * Deregisters a system
         * @tparam TSystem A registered system to remove
         * @remark This automatically removes any dependencies to or from this system
         */
        template<class TSystem, class = std::enable_if_t<std::is_base_of_v<System, TSystem>>>
        void DeregisterSystem(){
            auto sysHash = typeid(TSystem).hash_code();
            CoreAssert(_systemLocation.count(sysHash) > 0,
                "Cannot deregister system {}, because it isn't registered",
                typeid(TSystem).name()
            )
            auto group = _systemLocation.at(sysHash).lock();
            group->DeregisterSystem<TSystem>();
            _systemLocation.erase(sysHash);
        }

        ///@}

        void OnInitialize() override;

        void OnUpdate(float deltaTime) override;

        void OnImGui(float deltaTime, bool renderingToImguiWindow) override;

        void OnPostUpdate() override;

        void OnShutdown() override;

        void OnBeforeUnloadModules(const std::vector<Module>& modules) override;

        void OnAfterUnloadModules(const std::vector<Module>& modules) override;

    private:
        template<class Fn>
        void executeOnSystemsInOrder(Fn fn);

        // The component manager needs to be destructed LAST, therefore it must come first
        owned<ComponentManager> _componentManager;
        owned<EntityManager> _manager;

        DependencyGraph<TypeHash> _systemGroupExecutionOrder{};
        PersistentTypeMap<shared<SystemsGroup>> _registeredSystemGroups{};
        PersistentTypeMap<std::weak_ptr<SystemsGroup>> _systemLocation{};
    };

    template<class... TComponents>
    shared<Prefab> ECSContext::CreatePrefab(TComponents&& ... components) {
        return Prefab::Create(ref(&_componentManager), components...);
    }

}