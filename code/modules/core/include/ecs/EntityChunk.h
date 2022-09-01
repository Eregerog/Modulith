/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include "ECSUtils.h"
#include "ComponentManager.h"

namespace modulith{

    /// Each entity chunk's entity buffer size is 16 kb
    #define MODU_CHUNK_SIZE_BYTES 16 * 1024

    /**
     * An entity chunk contains a number of entities that all have the same signature (e.g. they have the same components).
     * It is a custom allocator for entities with the same signature and ensures a cache-friendly memory layout.
     * @remark Its buffer is organized as follows: First, all alive entities, then all dead entities followed by all unoccupied entity slots.
     * "Dead" entities are excluded from queries and will be removed at the end of the frame.
     */
    class CORE_API EntityChunk {

    public:
        /**
         * Creates an entity chunk
         * @param signature The signature of all entities. It contains the information of all their component types.
         * @param componentManager The application's current component manager
         */
        EntityChunk(const SignatureIdentifier& signature, const ref<ComponentManager>& componentManager);

        EntityChunk(EntityChunk& chunk) = delete;

        ~EntityChunk();

        /**
         * @name Chunk Properties
         */
        ///@{

        /**
         * @return Returns the amount of entity slots currently occupied
         */
        [[nodiscard]] size_t GetOccupied() const { return _aliveCount + _deadCount; }

        /**
         * @return Returns the total capacity of this chunk
         */
        [[nodiscard]] size_t GetCapacity() const { return _capacity; }

        /**
         * @return Returns the amount of entity slots currently free
         */
        [[nodiscard]] size_t GetFree() const { return GetCapacity() - GetOccupied(); }

        /**
         * @return Returns the component signature of this chunk
         */
        [[nodiscard]] Signature GetSignature() const { return _signature; }

        /**
         * @return Returns the component signature identifier of this chunk
         */
        [[nodiscard]] SignatureIdentifier GetIdentifier() const { return _identifier; }

        /**
         * @return Returns the size (in bytes) of a single entity and its components allocated in this chunk
         */
        [[nodiscard]] size_t GetEntitySize() const { return _entitySize; }

        /**
         * @return Returns the names of all the names of this chunk's component
         */
        [[nodiscard]] std::vector<std::string> GetIdentifierNames() const {
            std::vector<std::string> res{};
            for (auto& component : _identifier)
                res.emplace_back(component.get().name());
            return res;
        }

        /**
         * @return Returns all entities allocated in this chunk
         */
        [[nodiscard]] std::vector<Entity> AllEntities() const {
            std::vector<Entity> res{};
            for (auto& kvp : _entityIndices)
                res.emplace_back(kvp.first);
            return res;
        }

        ///@}

        /**
         * @name Entity Queries
         */
        ///@{

        [[nodiscard]] size_t OffsetOf(Entity entity) const {
            CoreAssert(_entityIndices.count(entity) == 1, "The entity is not contained in this chunk!")
            return _entityIndices.at(entity);
        }

        /**
         * Returns a typed pointer to the entity's component
         * @tparam TComponent The type of the component.
         * @param entity The entity to get the component of. The entity must be contained in this chunk.
         * @return A pointer to the component, or nullptr if this chunk does not contain the component.
         */
        template<class TComponent>
        TComponent* GetComponentPtr(Entity entity);

        /**
         * Returns an untyped pointer to the entity's component
         * @param entity The entity to get the component of. The entity mist be contain in this chunk.
         * @param component The identifier of the component to get.
         * @return A pointer to the component, or nullptr if this chunk does not contain the component.
         */
        void* GetComponentPtr(Entity entity, ComponentIdentifier component);

        /**
         * Returns whether the given entity is contained in this chunk
         * @param mustBeAlive If false, entities that are marked as "dead" will also be included.
         */
        [[nodiscard]] bool ContainsEntity(Entity entity, bool mustBeAlive = false) const;

        /**
         * @return Returns whether the component of the given type is contained in this chunk
         */
        [[nodiscard]] bool ContainsComponent(const ComponentIdentifier& componentType) const;

        /**
         * Internal Implementation of the EntityManager.Query.
         * The passed function is called for every entity in this chunk with the appropriate parameters.
         * @see EntityManager.QueryActive
         * @see EntityManager.QueryAll
         * @remark Refer to the general doxygen documentation on Queries on how this method is used
         */
        template<class... EachComponents, class... AnyComponents, class... HasComponents, class Fn>
        void Query(Each<EachComponents...> each, Any<AnyComponents...> any, Fn function, HasComponents... hasComponents);

        ///@}

        /**
         * @name Chunk Modification
         */
        ///@{

        /**
         * Allocates a new entity in this chunk.
         * There must be room for another entity.
         * @param entity The entity to be allocated. It mustn't already be contained in this chunk.
         */
        void AllocateEntity(Entity entity);

        /**
         * Frees the given entity immediately
         * @param entity An entity that is contained in the chunk.
         */
        void FreeEntityImmediately(Entity entity);


        /**
         * Marks the entity as "dead" to be freed at the end of the frame.
         * @param entity An entity that is contained in the chunk, but not yet marked dead.
         */
        void FreeEntityDeferred(Entity entity);

        /**
         * Moves an entity and its component values from one chunk to another.
         * Components present in the "from" chunk but not the "to" chunk are not moved and need to
         * be manually destructed by the calling code.
         * Component present in the "to" chunk but not the "from" chunk are not initialized and need to
         * be manually assigned by the calling code.
         * @param entity The entity to move. It must be contained in the from chunk and not contained in the to chunk.
         * @param from The chunk to remove the entity from
         * @param to The chunk to move the entity to
         * @param identifier The identifier of the moved components. It must be a subset of both chunk's contained component identifiers.
         * @param manager A reference to the application's current component manager.
         */
        static void MoveEntity(
            Entity entity, EntityChunk& from, EntityChunk& to, const SignatureIdentifier& identifier,
            ref<ComponentManager>& manager
        );

        /**
         * Moves the given component into the chunk.
         * The original value of the allocated component is reset and cannot be used after calling this method.
         * @tparam TComponent The type of component to move. It must be contained in this chunk
         * @param entity the entity to add the component to
         * @param toAdd The component to move. After calling this method, the original value may no longer be used.
         * @return A pointer to the moved component
         */
        template<class TComponent>
        TComponent* MoveComponentIntoChunk(Entity entity, TComponent& toAdd);

        ///@}

        /**
         * When called, all entities that were marked as "dead" will be freed from this chunk.
         * @return All entities that got freed.
         */
        std::vector<Entity> CleanupDeadEntitiesAtEndOfFrame();

    private:

        /**
         * @param fromIndex Inclusive
         * @param toIndex Exclusive
         */
        void destructEntityComponents(uint32_t fromIndex, uint32_t toIndex);
        void makeLastAliveEntity(Entity entity);
        [[nodiscard]] Entity entityAt(uint32_t index) const;

        ref<ComponentManager> _componentManager;

        size_t _capacity;
        uint32_t _aliveCount;
        uint32_t _deadCount;

        SignatureIdentifier _identifier;
        Signature _signature;

        // Size of an entry for an entity
        size_t _entitySize;

        EntityMappedTo<uint32_t> _entityIndices;
        ComponentMap<size_t> _offsets;

        std::byte _buffer[MODU_CHUNK_SIZE_BYTES];
    };


    template<class TComponent>
    TComponent* EntityChunk::GetComponentPtr(Entity entity) {
        return (TComponent*) GetComponentPtr(entity, typeid(TComponent));
    }

    template<class... EachComponents, class... AnyComponents, class... HasComponents, class Fn>
    void EntityChunk::Query(Each<EachComponents...>, Any<AnyComponents...>, Fn function, HasComponents... hasComponents) {

        for(uint32_t index = 0; index < _aliveCount; ++index){
            auto entity = entityAt(index);
            function(
                entity, *(GetComponentPtr<EachComponents>(entity))..., (GetComponentPtr<AnyComponents>(entity))...,
                hasComponents...
            );
        }
    }

    template<class TComponent>
    TComponent* EntityChunk::MoveComponentIntoChunk(Entity entity, TComponent& toAdd) {
        auto destPtr = GetComponentPtr<TComponent>(entity);

        memcpy(destPtr, &toAdd, sizeof(TComponent));
        // Free memory of temporary component, so its resources (like smart pointers) are not disposed
        memset(&toAdd, 0, sizeof(TComponent));

        return destPtr;
    }
}
