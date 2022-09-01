/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"

namespace modulith{

    class EntityManager;

    /**
     * Entities are identifiers with no behaviour to which any amount of components can be attached to.
     * These components contain data and are mutated by systems.
     */
    struct CORE_API Entity {
        /**
         * Creates an invalid entity
         */
        Entity() : Id(Invalid().Id) {}

        /**
         * Creates an entity with the given id
         * @param id The id of the entity
         */
        explicit Entity(uint32_t id) : Id(id) {}

        /**
         * @return Returns the entities Id
         */
        uint32_t GetId() const { return Id; }

        /**
         * @name Entity Manager Aliases
         * This section contains shorthand aliases for all methods in the EntityManager,
         * as well as a few utility methods combining multiple alias methods.
         */
        ///@{

        /**
         * @see EntityManager.IsAlive
         * @return Returns whether this entity is alive under the given entity manager
         */
        bool IsAlive(ref<EntityManager> manager);

        /**
         * Marks the given entity for destruction.
         * It will still exist until the end of the current frame,
         * but will be excluded from all queries.
         * At the end of the frame, the entity will be destroyed.
         * @see EntityManager.DestroyEntity
         * @param manager An entity manager in which this entity is currently alive
         */
        void Destroy(ref<EntityManager> manager);

        /**
         * Marks the given entity for destruction after the current query completes.
         * It will still exist until the end of the current frame,
         * but will be excluded from all queries.
         * At the end of the frame, the entity will be destroyed.
         * May only be called inside the function of a query.
         * @see EntityManager.Defer
         * @see EntityManager.DestroyEntity
         * @param manager An entity manager in which this entity is currently alive
         */
        void DestroyDeferred(ref<EntityManager> manager);

        /**
         * @see EntityManager.HasComponents
         * @return Returns whether the component of the given type is present on this entity
         */
        template<class TComponent>
        bool Has(ref<EntityManager> manager);

        /**
         * @see EntityManager.GetComponent
         * @return Returns a pointer to the component if it is present on this entity, or nullptr otherwise
         */
        template<class TComponent>
        TComponent* Get(ref<EntityManager> manager);

        /**
         * Constructs and adds a component of the given type to this entity.
         * @see EntityManager.AddComponent
         * @tparam TComponent The component type that must be trivially constructable
         * @return Returns a pointer to the added component
         */
        template<class TComponent>
        TComponent* Add(ref<EntityManager>);

        /**
         * Adds the given component to this entity
         * @see EntityManager.AddComponent
         * @tparam TComponent The component type
         * @param toAdd The value of the component
         * @return Returns a pointer to the added component
         */
        template<class TComponent>
        TComponent* Add(ref<EntityManager> manager, TComponent&& toAdd);

        /**
         * Constructs and adds a component of the given type after the current query has been completed
         * May only be called inside the function of a query.
         * @see EntityManager.Defer
         * @see EntityManager.AddComponent
         * @tparam TComponent The type of the added component. It must be trivially construcable
         */
        template<class TComponent>
        void AddDeferred(ref<EntityManager> manager);

        /**
         * Adds a component of the given type after the current query has been completed
         * May only be called inside the function of a query.
         * @see EntityManager.Defer
         * @see EntityManager.AddComponent
         * @tparam TComponent The type of the added component.
         * @param toAdd The component to add
         */
        template<class TComponent>
        void AddDeferred(ref<EntityManager> manager, TComponent&& toAdd);

        /**
         * Removes the component from this entity
         * @see EntityManager.RemoveComponent
         * @tparam TComponent The type of component to remove
         * @return Returns true if the component was present and removed and false if it was not present
         */
        template<class TComponent>
        bool Remove(ref<EntityManager> manager);

        /**
         * Removes the component from this entity after the current query has been completed.
         * May only be called inside the function of a query.
         * @see EntityManager.Defer
         * @see EntityManager.RemoveComponent
         * @tparam TComponent The type of the component to remove
         */
        template<class TComponent>
        void RemoveDeferred(ref<EntityManager> manager);

        /**
         * If the given component is present on this entity, it will be removed.
         * If it is not present, the component will be constructed and added instead.
         * This is intended to be used with tag components.
         * @see EntityManager.AddComponent
         * @see EntityManager.RemoveComponent
         * @tparam TComponent The type of component to toggle. Must be trivially constructable.
         */
        template<class TComponent>
        void Toggle(ref<EntityManager> manager);

        /**
         * If the given condition is true, the component will be added (overwriting the component if already present).
         * If it is false, the component will be removed (no effect if the component is not present).
         * @see EntityManager.AddComponent
         * @see EntityManager.RemoveComponent
         * @tparam TComponent The type of component to toggle. Must be trivially constructable.
         */
        template<class TComponent>
        void SetIf(ref<EntityManager> manager, bool condition);

        /**
         * Compares two entities by their Ids
         */
        bool operator==(Entity other) const {
            return Id == other.Id;
        }

        /**
         * Compares two entities by their Ids
         */
        bool operator!=(Entity other) const {
            return Id != other.Id;
        }

        static Entity Invalid() { return Entity(0); }

    private:
        uint32_t Id = 0;
    };

    struct EntityHasher;
    struct EntityEqualTo;

    /**
     * Alias for an unordered map mapping an entity to TValue
     */
    template<class TValue>
    using EntityMappedTo = std::unordered_map<Entity, TValue, EntityHasher, EntityEqualTo>;

    /**
     * Alias for an unordered set of entities
     */
    using EntitySet = std::unordered_set<Entity, EntityHasher>;

    /**
     * Provides a function for creating a hash code for an entity
     */
    struct EntityHasher {
        std::size_t operator()(modulith::Entity e) const {
            return std::hash<uint32_t>{}(e.GetId());
        }
    };

    /**
     * Provides a function for creating comparing two entities for equality
     */
    struct EntityEqualTo {
        bool operator()(modulith::Entity lhs, modulith::Entity rhs) const {
            return lhs == rhs;
        }
    };
}

/**
 * Allows printing entities to an output stream, such as when logging
 */
inline std::ostream& operator<<(std::ostream& os, const modulith::Entity e) {
    return os << "Entity (id: " << e.GetId() << ")";
}

namespace YAML {

    // Allows the (de-)serialization of Entities

    template<>
    struct convert<Entity> {
        static Node encode(const Entity& val) {
            Node node;
            node["Id"] = val.GetId();
            return node;
        }

        static bool decode(const Node& node, Entity& res) {

            try {
                res = Entity(node["Id"].as<uint32_t>());
                return true;
            } catch (YAML::InvalidNode&) {
                return false;
            }
        }
    };
}