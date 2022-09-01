/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include "utils/DependencyGraph.h"
#include "utils/TypeUtils.h"
#include "ecs/systems/System.h"

namespace modulith {

    /**
     * Used to specify which Systems or SystemsGroups this system must execute before
     * @tparam ... The types of Systems or SystemsGroups that are executed after the given system
     */
    template<class...>
    struct ExecuteBefore {
    };


    /**
     * Used to specify which Systems or SystemsGroups this system must execute after
     * @tparam ... The types of Systems or SystemsGroups that are executed before the given system
     */
    template<class...>
    struct ExecuteAfter {
    };

    class CORE_API SystemsGroup {
    public:
        /**
         * @return Returns a debug-only name of this systems group
         */
        virtual std::string GetName() = 0;

        /**
         * Registers a given system in this group to receive game loop callbacks
         * @tparam TSystem The type of the system to register. Its type mustn't be registered yet
         * @param system The system to register
         */
        template<class TSystem,class = std::enable_if_t<std::is_base_of_v<System, TSystem>>>
        void RegisterSystem(shared <TSystem> system) {
            auto systemHash = typeid(TSystem).hash_code();
            CoreAssert(_registeredSystems.count(systemHash) == 0,
                "Cannot register system {} in systems group {} because the system is already contained in the group",
                typeid(TSystem).name(),
                GetName())
            _registeredSystems.emplace(systemHash, system);
            _systemExecutionOrder.Add(systemHash);
        }

        /**
         * Registers an execution dependency between two registered systems of this group
         * @tparam TBefore The system to execute before the other
         * @tparam TAfter The system to execute after the other
         * @remark This mustn't create dependency circles!
         */
        template<class TBefore, class TAfter, class = std::enable_if_t<
            std::is_base_of_v<System, TBefore> && std::is_base_of_v<System, TBefore>>>
        void RegisterSystemDependency(){
            auto beforeHash = typeid(TBefore).hash_code();
            auto afterHash = typeid(TAfter).hash_code();
            CoreAssert(_registeredSystems.count(beforeHash) > 0,
                "Cannot register dependency from system {} to system {}, because the former is not registered",
                typeid(TBefore).name(),
                typeid(TAfter).name()
            )
            CoreAssert(_registeredSystems.count(afterHash) > 0,
                "Cannot register dependency from system {} to system {}, because the latter is not registered",
                typeid(TBefore).name(),
                typeid(TAfter).name()
            )
            CoreAssert(_systemExecutionOrder.CanAddDependency(beforeHash, afterHash),
                "Cannot add a system execution order dependency between {} and {} because it would cause a circular dependency!",
                typeid(TBefore).name(),
                typeid(TAfter).name()
            )
            _systemExecutionOrder.AddDependency(beforeHash, afterHash);
        }

        /**
         * Deregisters a system
         * @tparam TSystem A registered system of this group to remove
         * @remark This automatically removes any dependencies to or from this system
         */
        template<class TSystem, class = std::enable_if_t<std::is_base_of_v<System, TSystem>>>
        void DeregisterSystem() {
            auto systemHash = typeid(TSystem).hash_code();
            CoreAssert(_registeredSystems.count(systemHash) > 0,
                "Cannot deregister system {} in systems group {} because the system isn't  contained in the group",
                typeid(TSystem).name(),
                GetName())
            _registeredSystems.erase(systemHash);
            _systemExecutionOrder.Remove(systemHash);
        }

        /**
         * Calls the given function on each system in this group,
         * in order of system execution order.
         * @tparam Fn Type of a callable object with operator()(shared<System>&)
         */
        template<class Fn, class = std::enable_if_t<
            std::is_same_v<decltype(std::declval<Fn>().operator()(std::declval<shared<System>&>())), void>
        >>
        void ExecuteInOrder(Fn fn) {
            for (auto systemHash : _systemExecutionOrder.AllNodesFromStartToEndTopological()) {
                fn(_registeredSystems.at(systemHash));
            }
        }

        /**
         * @tparam T The type of the system to retrieve
         * @return Returns the system of the given type if it is registered, nullopt otherwise
         */
        template<class T>
        std::optional<shared<T>> TryGetSystem() {
            auto systemHash = typeid(T).hash_code();
            if (_registeredSystems.count(systemHash) > 0) {
                auto res = std::dynamic_pointer_cast<T>(_registeredSystems.at(systemHash));
                CoreAssert(res != nullptr,
                    "The system registered for type {0} could not be dynamically cast to type shared_ptr<{0}>. This should not happen!",
                    typeid(T).name())
                return res;
            }

            return std::nullopt;
        }

    private:
        DependencyGraph<TypeHash> _systemExecutionOrder;
        PersistentTypeMap<shared<System>> _registeredSystems;
    };

    /**
     * This type is used to denote that a System belongs in a group
     * @tparam TGroup The type of a SystemsGroup the systems belongs to
     */
    template<class TGroup, class = std::enable_if_t<std::is_base_of_v<SystemsGroup, TGroup>>>
    struct InGroup {
    };
}
