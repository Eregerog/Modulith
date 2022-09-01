/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "CoreModule.h"
#include "ECSUtils.h"
#include "ComponentManager.h"

namespace modulith {

    /**
     * A prefab is a blueprint for an entity with components that can be instantiated any number of times
     */
    class CORE_API Prefab {
    public:
        /**
         * Creates a prefab with the given components
         * @tparam TComponents The types of the components. Every type must be trivially copy-constructable
         * @param componentManager A reference to the current component manager
         * @param components The values of the components
         * @return The created prefab
         */
        template<class... TComponents>
        static shared<Prefab> Create(ref<ComponentManager> componentManager, TComponents&... components);

        /**
         * Creates a prefab with all of the entities components
         * @param componentManager A reference to the current component manager
         * @param entityManager A reference to the entity manager the entity is from
         * @param entity The entity to copy. All attached components must be trivially copy-constructable
         * @return The created prefab
         */
        static shared<Prefab> CreateFromEntity(const ref<ComponentManager>& componentManager, const ref<EntityManager>& entityManager, Entity entity);

        /**
         * Creates a prefab with the given signature
         * @param identifier The component signature of the prefab. All contained component types must be trivially copy-construcable
         * @param componentManager A reference to the current component manager
         */
        Prefab(const SignatureIdentifier& identifier, const ref<ComponentManager>& componentManager);

        Prefab(const Prefab&) = delete;

        ~Prefab() {
            for (const auto& component : _identifier) {
                auto* ptr = GetComponentPtr(component);
                auto info = _componentManager->GetInfoOf(component);
                info.Destruct(ptr);
            }

            delete[] _buffer;
        }

        /**
         * Checks if the prefab has a component of the given type attached to it
         * @tparam TComponent The type of the component
         */
        template<class TComponent>
        bool Has(){
            return _identifier.count(typeid(TComponent)) > 0;
        }

        /**
         * @return Returns a pointer to the given component. Nullptr if that component is not present
         */
        template<class TComponent>
        TComponent* Get(){
            return GetComponentPtr<TComponent>();
        }

        /**
         * Instantiates the prefab among the entities of the given entity manager
         * @return The created entity
         */
        Entity InstantiateIn(const ref<EntityManager>& entityManager);


        /**
         * Instantiates and positions the prefab among the entities of the given entity manager.
         * Position and rotation components are attached automatically if not present already
         * @return The created entity
         */
        Entity InstantiateAt(const ref<EntityManager>& entityManager, float3 position, quat rotation = glm::identity<quat>());

        /**
         * @return Returns the size (in bytes) of this prefab's underlying data buffer
         */
        [[nodiscard]] size_t GetSize() const { return _size; }

    private:
        template<class TComponent>
        TComponent* GetComponentPtr();

        void* GetComponentPtr(ComponentIdentifier component);

        template<class TComponent>
        TComponent* MoveComponentIntoPrefab(TComponent& toAdd);


    private:
        SignatureIdentifier _identifier;
        Signature _signature;

        ComponentMap<size_t> _offsets;
        size_t _size;
        std::byte* _buffer;

        ref<ComponentManager> _componentManager;
    };

    template<class TComponent>
    TComponent* Prefab::GetComponentPtr() {
        return (TComponent*) GetComponentPtr(typeid(TComponent));
    }

    template<class TComponent>
    TComponent* Prefab::MoveComponentIntoPrefab(TComponent& toAdd) {
        auto destPtr = GetComponentPtr<TComponent>();
        CoreAssert(destPtr, "The component {} does not exist in this prefab", typeid(TComponent).name())

        memcpy(destPtr, &toAdd, sizeof(TComponent));
        // Invalidate memory of temporary component, so its resources (like smart pointers) are not disposed
        memset(&toAdd, 0, sizeof(TComponent));

        return destPtr;
    }

    template<class... TComponents>
    shared<Prefab> Prefab::Create(ref<ComponentManager> componentManager, TComponents&... components) {
        shared<Prefab> res = std::make_shared<Prefab>(componentManager->ToIdentifier<TComponents...>(), componentManager);
        (res->MoveComponentIntoPrefab<TComponents>(components), ...);
        return res;
    }
}
