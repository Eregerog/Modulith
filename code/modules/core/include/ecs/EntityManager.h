/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "CoreModule.h"
#include "ComponentManager.h"
#include "EntityChunk.h"
#include "Entity.h"
#include "StandardComponents.h"

namespace modulith{

    class Prefab;
    class Context;

    /**
     * An entity manager contains the data of all its entities and provides methods
     * for adding / removing / changing and querying these entities.
     *
     * Please refer to the ECS documentation for more information on the EntityManager and how to use it
     * (found under includes/ecs/_docs.md or in the doxygen docs).
     */
    class CORE_API EntityManager {
        friend Prefab;
        friend Context;
    public:
        /**
         * Creates an entity manager using the specified component manager
         * @param componentManager The application's current component manager
         */
        explicit EntityManager(ref<ComponentManager> componentManager) : _componentManager(
            std::move(componentManager)) {}

        // Needed when passing EntityManager to a lambda that receives the parameter through auto.
        // This will prevent auto being the value type, and instead becomes the reference type
        EntityManager(EntityManager&) = delete;

        /**
         * @name Entity Lifetime
         * Methods for manipulating and querying an entitie's lifetime
         */
        ///@{

        /**
         * Creates a new entity and returns it
         */
        Entity CreateEntity();

        /**
         * Creates an entity with the given components and returns it
         * @tparam TComponents The types of all attached components
         * @param components The values of all the attached components
         * @return Returns the created entity
         */
        template<class... TComponents>
        Entity CreateEntityWith(TComponents&& ... components);

        std::pair<Entity, shared<EntityChunk>> CreateEntityBy(SignatureIdentifier& identifier);

        /**
         * Marks the given entity for destruction.
         * It will still exist until the end of the current frame,
         * but will be excluded from all queries.
         * At the end of the frame, the entity will be destroyed.
         * @param entity An entity that is currently alive
         */
        void DestroyEntity(Entity entity);

        /**
         * @return Whether the given entity is currently alive / exists in the entity manager
         */
        [[nodiscard]] bool IsAlive(Entity entity) const { return _entityLocations.count(entity) > 0; }

        ///@}

        /**
         * @name Component manipulation
         * Methods for adding / removing  components to and from entities and simple queries
         *
         * Adding a component to an entity that already has it replaces the existing component
         * Removing a component from an entity that does not have it has no effect and the method returns false.
         */
        ///@{

        /**
         * Moves the entity into an associated chunk where there is space for the given component.
         * If not already present, the component will be zero-initialized
         * and should be manually initialized by the calling code.
         * @param entity The entity to add the component to
         * @param identifier The type of the component
         * @return Returns a pointer to the component
         */
        void* AddComponent(Entity entity, ComponentIdentifier identifier);

        /**
         * Constructs a component and adds it to the entity
         * @tparam TComponent The type of the component
         * @tparam TArgs The types of the component's constructor arguments
         * @param entity The entity to add the component to
         * @param args The values of the component's constructor arguments
         * @return A pointer to the added component
         */
        template<class TComponent, class... TArgs>
        TComponent* AddComponent(Entity entity, TArgs... args);

        /**
         * Adds the given component to the entity
         * @tparam TComponent The type of the component
         * @param entity The entity to add the component to
         * @param toAdd The value of the added component
         * @return A poitner to the added component on the entity
         */
        template<class TComponent>
        TComponent* AddComponent(Entity entity, TComponent&& toAdd);

        /**
         * Adds the given components to the entity by automatically constructing them
         * @tparam TComponents The types of the components. All must be trivially constructable
         * @param entity The entity to add the components to
         */
        template<class... TComponents>
        void AddComponents(Entity entity);

        /**
         * Adds the given components to the entity
         * @tparam TComponents The types of the components.
         * @param entity The entity to add the components to
         * @param toAdd The values of the added components
         */
        template<class... TComponents>
        void AddComponents(Entity entity, TComponents&& ... toAdd);

        /**
         * Removes and destruct the component of the given type from the entity
         * @param entity The entity to remove the component from
         * @param identifier The type of the component to remove
         * @return True if the component was present and removed, false otherwise
         */
        bool RemoveComponent(Entity entity, ComponentIdentifier identifier);

        /**
         * Removes the given component from the entity
         * @tparam TComponent The type of the component to remove
         * @param entity The entity to remove the component from
         * @return Returns true if the component was present and removed, false if the component was not present
         */
        template<class TComponent>
        bool RemoveComponent(Entity entity);

        /**
         * Removes the given components from the entity
         * @tparam TComponents The types of the components to remove
         * @param entity The entity to remove the components from
         * @return Returns false if none of the components was present to be removed, true otherwise
         */
        template<class... TComponents>
        bool RemoveComponents(Entity entity);

        /**
         * @return Returns true if the entity has all the components of the given types
         */
        template<class... TComponents>
        bool HasComponents(Entity entity);

        /**
         * Tries to get the component attached to an entity
         * @tparam TComponent The type of the component
         * @param entity The entity
         * @return Nullptr if the component was not present, a pointer to the component otherwise
         */
        template<class TComponent>
        TComponent* GetComponent(Entity entity);

        ///@}

        /**
         * @name Advanced Queries
         *
         * These queries allow the caller to execute code for every entity that matches certain restrictions
         * in the form of a function parameter which will be called for every entity match.
         *
         * For general information on the ECS system and a comprehensive introduction to Queries,
         * please refer to the ECS documentation (found under includes/ecs/_docs.md or in the doxygen docs).
         *
         * Within this function, all entity manipulation operations (AddComponent, RemoveComponent, Create, Destroy) must be deferred using Defer.
         * These will then be executed at the end of the query.
         *
         * The user can set the following restrictions:
         * @see Each All types given to this class must be present on a given entity to be part of the query.
         * @see Any At least one of the given types must be present on an entity to be part fo the query.
         * @see None None of the types given to this class must be present on the entity to be part of the query.
         * @see Has For each type given to this class the function will receive a boolean that is set to true if the given component type is present on the entity.
         *
         * The provided function must have an appropriate signature depending on the restrictions:
         * First, a parameter for the entity of type Entity
         * Then, reference parameters for all of the Each types of type Each&
         * After that, pointer parameters for all the Any types of type Any*
         * Lastly, booleans for each of the Has types of type bool
         *
         * This results in a function signature similar to:
         * void(Entity, EachComponents&..., AnyComponents*..., bool...)
         *
         * Example: Calling QueryAll(Each<Foo>(), Any<Bar, Baz>, None<Quaz>, Has<Jazz>, ...) requires a lambda function with the parameters
         * void(Entity, Foo&, Bar*, Baz*, bool) in that order. The auto keyword can be used for these types.
         *
         * QueryActive will automatically exclude entities with the {@link DisabledTag} or {@link IndirectlyDisabledTag}
         * QueryAll does not exclude such entities
         */
        ///@{


        /**
         * @return Returns whether a query is currently being executed
         */
        [[nodiscard]] bool IsInsideQuery() const { return _iterationDepth > 0; }

        /**
         * @remark Refer to the general doxygen documentation on Queries on how to use this method and its overloads
         */
        template<class... EachComponents, class... AnyComponents, class... NoneComponents, class... THasComponents, class Fn, class = typename std::enable_if<
            // If Fn is a Callable with signature void(Entity, EachComponents&..., AnyComponents*..., bool...) this function can be used
            std::is_same<
                decltype(std::declval<Fn>().operator()(
                    std::declval<Entity>(),
                    std::declval<EachComponents&>()...,
                    std::declval<AnyComponents*>()...,
                    HasComponentUtility<THasComponents>()...
                )),
                void
            >::value>::type>
        void QueryAll(
            Each<EachComponents...> each, Any<AnyComponents...> any, None<NoneComponents...> none,
            Has<THasComponents...> has, Fn function
        );


        /**
         * @remark Refer to the general doxygen documentation on Queries on how to use this method and its overloads
         */
        template<class... EachComponents, class... AnyComponents, class... NoneComponents, class Fn, class = typename std::enable_if<
            // If Fn is a Callable with signature void(Entity, EachComponents&..., AnyComponents*...) this function can be used
            std::is_same<
                decltype(std::declval<Fn>().operator()(
                    std::declval<Entity>(),
                    std::declval<EachComponents&>()...,
                    std::declval<AnyComponents*>()...
                )),
                void
            >::value>::type>
        void QueryAll(Each<EachComponents...> each, Any<AnyComponents...> any, None<NoneComponents...> none, Fn function);

        /**
         * @remark Refer to the general doxygen documentation on Queries on how to use this method and its overloads
         */
        template<class... EachComponents, class Fn, class = typename std::enable_if<
            // If Fn is a Callable with signature void(Entity, EachComponents&...) this function can be used
            std::is_same<
                decltype(std::declval<Fn>().operator()(
                    std::declval<Entity>(),
                    std::declval<EachComponents&>()...
                )),
                void
            >::value>::type>
        void QueryAll(Each<EachComponents...> each, Fn function);

        /**
         * @remark Refer to the general doxygen documentation on Queries on how to use this method and its overloads
         */
        template<class... AnyComponents, class Fn, class = typename std::enable_if<
            // If Fn is a Callable with signature void(Entity, AnyComponents*...) this function can be used
            std::is_same<
                decltype(std::declval<Fn>().operator()(
                    std::declval<Entity>(),
                    std::declval<AnyComponents*>()...
                )),
                void
            >::value>::type>
        void QueryAll(Any<AnyComponents...> any, Fn function);

        /**
         * @remark Refer to the general doxygen documentation on Queries on how to use this method and its overloads
         */
        template<class... EachComponents, class... NoneComponents, class Fn, class = typename std::enable_if<
            // If Fn is a Callable with signature void(Entity, EachComponents&...) this function can be used
            std::is_same<
                decltype(std::declval<Fn>().operator()(
                    std::declval<Entity>(),
                    std::declval<EachComponents&>()...
                )),
                void
            >::value>::type>
        void QueryAll(Each<EachComponents...> each, None<NoneComponents...> none, Fn function);


        /**
         * @remark Refer to the general doxygen documentation on Queries on how to use this method and its overloads
         */
        template<class... EachComponents, class... AnyComponents, class... NoneComponents, class... THasComponents, class Fn, class = typename std::enable_if<
            // If Fn is a Callable with signature void(Entity, EachComponents&..., AnyComponents*..., bool...) this function can be used
            std::is_same<
                decltype(std::declval<Fn>().operator()(
                    std::declval<Entity>(),
                    std::declval<EachComponents&>()...,
                    std::declval<AnyComponents*>()...,
                    HasComponentUtility<THasComponents>()...
                )),
                void
            >::value>::type>
        void QueryActive(
            Each<EachComponents...> each, Any<AnyComponents...> any, None<NoneComponents...> none,
            Has<THasComponents...> has, Fn function
        );

        /**
         * @remark Refer to the general doxygen documentation on Queries on how to use this method and its overloads
         */
        template<class... EachComponents, class... AnyComponents, class... NoneComponents, class Fn, class = typename std::enable_if<
            // If Fn is a Callable with signature void(Entity, EachComponents&..., AnyComponents*...) this function can be used
            std::is_same<
                decltype(std::declval<Fn>().operator()(
                    std::declval<Entity>(),
                    std::declval<EachComponents&>()...,
                    std::declval<AnyComponents*>()...
                )),
                void
            >::value>::type>
        void
        QueryActive(Each<EachComponents...> each, Any<AnyComponents...> any, None<NoneComponents...> none, Fn function);

        /**
         * @remark Refer to the general doxygen documentation on Queries on how to use this method and its overloads
         */
        template<class... EachComponents, class Fn, class = typename std::enable_if<
            // If Fn is a Callable with signature void(Entity, EachComponents&...) this function can be used
            std::is_same<
                decltype(std::declval<Fn>().operator()(
                    std::declval<Entity>(),
                    std::declval<EachComponents&>()...
                )),
                void
            >::value>::type>
        void QueryActive(Each<EachComponents...> each, Fn function);

        /**
         * @remark Refer to the general doxygen documentation on Queries on how to use this method and its overloads
         */
        template<class... AnyComponents, class Fn, class = typename std::enable_if<
            // If Fn is a Callable with signature void(Entity, AnyComponents*...) this function can be used
            std::is_same<
                decltype(std::declval<Fn>().operator()(
                    std::declval<Entity>(),
                    std::declval<AnyComponents*>()...
                )),
                void
            >::value>::type>
        void QueryActive(Any<AnyComponents...> any, Fn function);

        /**
         * @remark Refer to the general doxygen documentation on Queries on how to use this method and its overloads
         */
        template<class... EachComponents, class... NoneComponents, class Fn, class = typename std::enable_if<
            // If Fn is a Callable with signature void(Entity, EachComponents&...) this function can be used
            std::is_same<
                decltype(std::declval<Fn>().operator()(
                    std::declval<Entity>(),
                    std::declval<EachComponents&>()...
                )),
                void
            >::value>::type>
        void QueryActive(Each<EachComponents...> each, None<NoneComponents...> none, Fn function);

        ///@}

        /**
         * @name Misc Methods
         */
        ///@{

        /**
         * @return Returns the chunk of the given entity. The entity must be alive
         */
        shared<EntityChunk> GetChunk(Entity entity);


        shared<EntityChunk> GetOrCreateChunkFor(const SignatureIdentifier& identifier);

        /**
         * @return Returns all the entity manager's current chunks
         */
        std::vector<shared<EntityChunk>> AllChunks() { return _chunks; }

        /**
         * @return Returns the amount of currently active chunks
         */
        [[nodiscard]] size_t ChunkCount() const { return _chunks.size(); }

        /**
         * @return Returns the amount of entities that are currently alive
         */
        [[nodiscard]] size_t EntityCount() const { return _entityLocations.size(); }

        /**
         * @return Returns the amount of currently registered components
         */
        [[nodiscard]] size_t RegisteredComponentCount() const { return _componentManager->RegisteredComponentCount(); }

        ///@}

        /**
         * Should be used during iteration methods (such as @refitem QueryActive) to call
         * methods that modify entities and their components.
         * Cannot be used outside of iteration methods.
         * @param deferredOperation
         * An operation that is executed once the iteration operation completes.
         * If this is a lambda, follow these capture rules:
         * - Never capture the entity manager this was called on, use the parameter instead!
         * - Never capture variables that are declared in the scope of the iteration by reference:
         * Once the operation is executed, that memory will have been freed!
         */
        void Defer(const std::function<void(ref<EntityManager>)>& deferredOperation);

        /**
         * Should be called only at the end of frame.
         * This method cleans up all destroyed entities and empty chunks.
         */
        void OnEndOfFrame();
    private:

        template<class... TComponents>
        void ensureComponentsAreRegistered() const;

        void ensureComponentIsRegistered(ComponentIdentifier identifier) const;

        template<class TComponent>
        void ensureComponentIsRegistered() const;

        /**
         * The depth of nested iteration functions currently being executed
         */
        int _iterationDepth = 0;
        std::vector<std::function<void(ref<EntityManager>)>> _deferredOperations;

        shared<EntityChunk> getChunkReadWrite(Entity entity);


        void executeDeferredOperations();

        unsigned int _runningEntityId = 0; // TODO temporary: replace with guid system / id pool?
        std::vector<shared<EntityChunk>> _chunks;
        EntityMappedTo<shared<EntityChunk>> _entityLocations;

        ref<ComponentManager> _componentManager;
    };

/// --------------------------------------------------------------------------------------------------------
///                     UTILS
/// --------------------------------------------------------------------------------------------------------

    template<class TComponent>
    void EntityManager::ensureComponentIsRegistered() const {
        CoreAssert(_componentManager->IsRegistered<TComponent>(),
            "The component {} is not registered in the component manager", typeid(TComponent).name())
    }

    inline void EntityManager::ensureComponentIsRegistered(ComponentIdentifier identifier) const {
        CoreAssert(_componentManager->IsRegistered(identifier),
            "The component {} is not registered in the component manager", identifier.get().name())
    }

    template<class... TComponents>
    void EntityManager::ensureComponentsAreRegistered() const {
        (ensureComponentIsRegistered<TComponents>(), ...);
    }

    template<class... TComponents>
    Entity EntityManager::CreateEntityWith(TComponents&& ... components) {
        ensureComponentsAreRegistered<TComponents...>();

        auto identifier = _componentManager->ToIdentifier<TComponents...>();
        auto [result, chunk] = CreateEntityBy(identifier);

        (chunk->MoveComponentIntoChunk(result, components), ...);

        return result;
    }

/// --------------------------------------------------------------------------------------------------------
///                     ADD COMPONENT
/// --------------------------------------------------------------------------------------------------------

    template<class TComponent, class... TArgs>
    TComponent* EntityManager::AddComponent(Entity entity, TArgs... args) {
        return AddComponent(entity, TComponent(args...));
    }

    template<class TComponent>
    TComponent* EntityManager::AddComponent(Entity entity, TComponent&& toAdd) {
        auto destPtr = AddComponent(entity, typeid(TComponent));

        memcpy(destPtr, &toAdd, sizeof(TComponent));
        // Free memory of temporary component, so its resources (like smart pointers) are not disposed
        memset(&toAdd, 0, sizeof(TComponent));
        return (TComponent*) destPtr;
    }

    template<class... TComponents>
    void EntityManager::AddComponents(Entity entity) {
        AddComponents(entity, TComponents()...);
    }

    template<class... TComponents>
    void EntityManager::AddComponents(Entity entity, TComponents&& ... toAdd) {
        CoreAssert(_iterationDepth == 0,
            "Entities cannot be modified while iterating over them! Use EntityManager->Defer instead!")
        ensureComponentsAreRegistered<TComponents...>();

        auto currentChunk = _entityLocations[entity];
        CoreAssert(currentChunk->ContainsEntity(entity),
            "The entity location data is no longer synced with the chunk data!")

        auto currentIdentifier = currentChunk->GetIdentifier();
        auto destinationIdentifier = SignatureIdentifier{typeid(TComponents)...};
        destinationIdentifier.insert(currentIdentifier.begin(), currentIdentifier.end());

        CoreAssert(destinationIdentifier.size() >= currentIdentifier.size(),
            "The new identifier cannot contain less components than the original one when adding components!")

        shared<EntityChunk> destinationChunk = currentChunk;

        if (destinationIdentifier.size() != currentIdentifier.size()) {

            destinationChunk = GetOrCreateChunkFor(destinationIdentifier);

            CoreAssert(currentChunk != destinationChunk,
                "When adding a component to an entity that doesn't have it, its chunk must change!")

            // The current identifier is used, since both chunks contain all components from it
            EntityChunk::MoveEntity(entity, *currentChunk, *destinationChunk, currentIdentifier, _componentManager);

            _entityLocations[entity] = destinationChunk;
        }

        (destinationChunk->MoveComponentIntoChunk<TComponents>(entity, toAdd), ...);
    }

/// --------------------------------------------------------------------------------------------------------
///                     REMOVE COMPONENT
/// --------------------------------------------------------------------------------------------------------

    template<class TComponent>
    bool EntityManager::RemoveComponent(Entity entity) {
        return RemoveComponent(entity, typeid(TComponent));
    }

    template<class T>
    void destruct(T* t) { t->~T(); }

    template<class... TComponents>
    bool EntityManager::RemoveComponents(Entity entity) {
        CoreAssert(_iterationDepth == 0,
            "Entities cannot be modified while iterating over them! Use EntityManager->Defer instead!")
        ensureComponentsAreRegistered<TComponents...>();

        auto currentChunk = _entityLocations[entity];
        CoreAssert(currentChunk->ContainsEntity(entity),
            "The entity location data is no longer synced with the chunk data!")

        auto currentIdentifier = currentChunk->GetIdentifier();
        auto destinationIdentifier = ComponentSet(currentIdentifier);
        auto removedIdentifier = SignatureIdentifier({typeid(TComponents)...});
        for (const auto& toRemove : removedIdentifier)
            destinationIdentifier.erase(toRemove);

        CoreAssert(destinationIdentifier.size() <= currentIdentifier.size(),
            "There cannot be more components on an entity after calling RemoveComponent!")

        if (destinationIdentifier.size() == currentIdentifier.size())
            return false;


        auto destinationChunk = GetOrCreateChunkFor(destinationIdentifier);

        (destruct<TComponents>(currentChunk->GetComponentPtr<TComponents>(entity)), ...);

        // The destination identifier is used, since both chunks contain all components from it
        EntityChunk::MoveEntity(entity, *currentChunk, *destinationChunk, destinationIdentifier, _componentManager);

        _entityLocations[entity] = destinationChunk;

        return true;
    }

/// --------------------------------------------------------------------------------------------------------
///                     COMPONENT QUERIES
/// --------------------------------------------------------------------------------------------------------

    template<class TComponent>
    TComponent* EntityManager::GetComponent(Entity entity) {
        ensureComponentsAreRegistered<TComponent>();
        auto chunk = getChunkReadWrite(entity);
        return chunk->GetComponentPtr<TComponent>(entity);
    }

    template<class... TComponents>
    bool EntityManager::HasComponents(Entity entity) {
        ensureComponentsAreRegistered<TComponents...>();
        if (!IsAlive(entity))
            return false;

        auto identifier = _componentManager->ToIdentifier<TComponents...>();
        auto signature = _componentManager->ToSignature(identifier);

        auto chunk = GetChunk(entity);
        return (chunk->GetSignature() & signature) == signature;
    }

    template<class... EachComponents, class... AnyComponents, class... NoneComponents, class... THasComponents, class Fn, class>
    void EntityManager::QueryAll(
        Each<EachComponents...> each, Any<AnyComponents...> any, None<NoneComponents...>, Has<THasComponents...>,
        Fn function
    ) {
        ensureComponentsAreRegistered<EachComponents..., AnyComponents..., NoneComponents..., THasComponents...>();
        ++_iterationDepth;

        auto eachIdentifier = _componentManager->ToIdentifier<EachComponents...>();
        auto eachSignature = _componentManager->ToSignature(eachIdentifier);

        auto anyIdentifier = _componentManager->ToIdentifier<AnyComponents...>();
        auto anySignature = _componentManager->ToSignature(anyIdentifier);

        auto noneIdentifier = _componentManager->ToIdentifier<NoneComponents...>();
        auto noneSignature = _componentManager->ToSignature(noneIdentifier);

        for (const shared<EntityChunk>& chunk : _chunks) {
            auto chunkIdentifier = chunk->GetIdentifier();
            auto chunkSignature = chunk->GetSignature();
            if ((chunkSignature & eachSignature) == eachSignature
                && (anySignature.none() || (chunkSignature & anySignature).any())
                && (chunkSignature & noneSignature).none()
                ) {
                chunk->Query(each, any, function, (chunkIdentifier.count(typeid(THasComponents)) > 0)...);
            }
        }

        --_iterationDepth;
        if (_iterationDepth == 0)
            executeDeferredOperations();
    }

    template<class... EachComponents, class... AnyComponents, class... NoneComponents, class Fn, class>
    void
    EntityManager::QueryAll(Each<EachComponents...> each, Any<AnyComponents...> any, None<NoneComponents...>, Fn function) {
        ensureComponentsAreRegistered<EachComponents..., AnyComponents..., NoneComponents...>();
        ++_iterationDepth;

        auto eachIdentifier = _componentManager->ToIdentifier<EachComponents...>();
        auto eachSignature = _componentManager->ToSignature(eachIdentifier);

        auto anyIdentifier = _componentManager->ToIdentifier<AnyComponents...>();
        auto anySignature = _componentManager->ToSignature(anyIdentifier);

        auto noneIdentifier = _componentManager->ToIdentifier<NoneComponents...>();
        auto noneSignature = _componentManager->ToSignature(noneIdentifier);

        for (const shared<EntityChunk>& chunk : _chunks) {
            auto chunkSignature = chunk->GetSignature();
            if ((chunkSignature & eachSignature) == eachSignature
                && (anySignature.none() || (chunkSignature & anySignature).any())
                && (chunkSignature & noneSignature).none()
                ) {
                chunk->Query(each, any, function);
            }
        }

        --_iterationDepth;
        if (_iterationDepth == 0)
            executeDeferredOperations();
    }

    template<class... EachComponents, class Fn, class>
    void EntityManager::QueryAll(Each<EachComponents...> each, Fn function) {
        ensureComponentsAreRegistered<EachComponents...>();
        ++_iterationDepth;

        auto eachIdentifier = _componentManager->ToIdentifier<EachComponents...>();
        auto eachSignature = _componentManager->ToSignature(eachIdentifier);

        for (const shared<EntityChunk>& chunk : _chunks) {
            auto chunkSignature = chunk->GetSignature();
            if ((chunkSignature & eachSignature) == eachSignature) {
                chunk->Query(each, Any(), function);
            }
        }

        --_iterationDepth;
        if (_iterationDepth == 0)
            executeDeferredOperations();
    }

    template<class... AnyComponents, class Fn, class>
    void EntityManager::QueryAll(Any<AnyComponents...> any, Fn function) {
        ensureComponentsAreRegistered<AnyComponents...>();
        ++_iterationDepth;

        auto anyIdentifier = _componentManager->ToIdentifier<AnyComponents...>();
        auto anySignature = _componentManager->ToSignature(anyIdentifier);

        for (const shared<EntityChunk>& chunk : _chunks) {
            auto chunkSignature = chunk->GetSignature();
            if (anySignature.none() || (chunkSignature & anySignature).any()) {
                chunk->Query(Each(), any, function);
            }
        }

        --_iterationDepth;
        if (_iterationDepth == 0)
            executeDeferredOperations();
    }


    template<class... EachComponents, class... NoneComponents, class Fn, class>
    void EntityManager::QueryAll(Each<EachComponents...> each, None<NoneComponents...>, Fn function) {
        ensureComponentsAreRegistered<EachComponents..., NoneComponents...>();
        ++_iterationDepth;

        auto eachIdentifier = _componentManager->ToIdentifier<EachComponents...>();
        auto eachSignature = _componentManager->ToSignature(eachIdentifier);

        auto noneIdentifier = _componentManager->ToIdentifier<NoneComponents...>();
        auto noneSignature = _componentManager->ToSignature(noneIdentifier);

        for (const shared<EntityChunk>& chunk : _chunks) {
            auto chunkSignature = chunk->GetSignature();
            if ((chunkSignature & eachSignature) == eachSignature
                && (chunkSignature & noneSignature).none()
                ) {
                chunk->Query(each, Any(), function);
            }
        }

        --_iterationDepth;
        if (_iterationDepth == 0)
            executeDeferredOperations();
    }

    template<class... EachComponents, class... AnyComponents, class... NoneComponents, class... THasComponents, class Fn, class>
    void EntityManager::QueryActive(
        Each<EachComponents...> each, Any<AnyComponents...> any, None<NoneComponents...>, Has<THasComponents...> has,
        Fn function
    ) {
        QueryAll(each, any, None<IndirectlyDisabledTag, NoneComponents...>(), has, function);
    }

    template<class... EachComponents, class... AnyComponents, class... NoneComponents, class Fn, class>
    void EntityManager::QueryActive(
        Each<EachComponents...> each, Any<AnyComponents...> any, None<NoneComponents...>, Fn function
    ) {
        QueryAll(each, any, None<IndirectlyDisabledTag, NoneComponents...>(), function);
    }

    template<class... EachComponents, class Fn, class>
    void EntityManager::QueryActive(Each<EachComponents...> each, Fn function) {
        QueryAll(each, None<IndirectlyDisabledTag>(), function);
    }

    template<class... AnyComponents, class Fn, class>
    void EntityManager::QueryActive(Any<AnyComponents...> any, Fn function) {
        QueryAll(any, None<IndirectlyDisabledTag>(), function);
    }

    template<class... EachComponents, class... NoneComponents, class Fn, class>
    void EntityManager::QueryActive(Each<EachComponents...> each, None<NoneComponents...>, Fn function) {
        QueryAll(each, None<IndirectlyDisabledTag, NoneComponents...>(), function);
    }

/// --------------------------------------------------------------------------------------------------------
///                     ENTITY ALIASES
/// --------------------------------------------------------------------------------------------------------

    inline bool Entity::IsAlive(ref<EntityManager> manager){
        return manager->IsAlive(*this);
    }

    inline void Entity::Destroy(ref<EntityManager> manager) {
        manager->DestroyEntity(*this);
    }

    inline void Entity::DestroyDeferred(ref<EntityManager> manager){
        manager->Defer([id = Id](auto manager){manager->DestroyEntity(Entity(id));});
    }

    template<class TComponent>
    bool Entity::Has(ref<EntityManager> manager) {
        return manager->HasComponents<TComponent>(*this);
    }

    template<class TComponent>
    TComponent* Entity::Get(ref<EntityManager> manager) {
        return manager->GetComponent<TComponent>(*this);
    }

    template<class TComponent>
    TComponent* Entity::Add(ref<EntityManager> manager){
        return this->Add(manager, TComponent());
    }

    template<class TComponent>
    TComponent* Entity::Add(ref<EntityManager> manager, TComponent&& toAdd) {
        return manager->AddComponent(*this, std::move(toAdd));
    }

    template<class TComponent>
    bool Entity::Remove(ref<EntityManager> manager) {
        return manager->RemoveComponent<TComponent>(*this);
    }

    template<class TComponent>
    void Entity::AddDeferred(ref<EntityManager> manager){
        this->AddDeferred(manager, TComponent());
    }

    template<class TComponent>
    void Entity::AddDeferred(ref<EntityManager> manager, TComponent&& toAdd) {
        manager->Defer(
            [id = Id, comp = std::move(toAdd)](auto manager) { manager->AddComponent<TComponent>(Entity(id), comp); }
        );
    }

    template<class TComponent>
    void Entity::RemoveDeferred(ref<EntityManager> manager) {
        manager->Defer([id = Id](auto manager) { manager->RemoveComponent<TComponent>(Entity(id)); });
    }

    template<class TComponent>
    void Entity::Toggle(ref<EntityManager> manager) {
        if (Has<TComponent>(manager)) Remove<TComponent>(manager); else Add(manager, TComponent());
    }

    template<class TComponent>
    void Entity::SetIf(ref<EntityManager> manager, bool condition){
        auto hasIt = Has<TComponent>(manager);
        if(hasIt && !condition) Remove<TComponent>(manager);
        else if(!hasIt && condition) Add<TComponent>(manager);
    }
}
