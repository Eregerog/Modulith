/**
 * \brief
 * \author Daniel Götz
 */

#include <ecs/transform/TransformComponents.h>
#include "ecs/EntityManager.h"

namespace modulith{

    void EntityManager::OnEndOfFrame() {
        std::vector<Entity> allDestroyedEntities;

        for(shared<EntityChunk>& chunk : _chunks){
           auto chunkDestroyedEntities = chunk->CleanupDeadEntitiesAtEndOfFrame();
           allDestroyedEntities.insert(
                allDestroyedEntities.end(), chunkDestroyedEntities.begin(), chunkDestroyedEntities.end());
        }

        std::for_each(
            allDestroyedEntities.begin(), allDestroyedEntities.end(), [this](Entity toDestroy) {
                _entityLocations.erase(toDestroy);
            }
        );

        _chunks.erase(
            std::remove_if(
                _chunks.begin(), _chunks.end(), [](shared<EntityChunk>& chunk) { return chunk->GetOccupied() == 0; }
            ),
            _chunks.end()
        );
    }

    shared<EntityChunk>
    EntityManager::GetOrCreateChunkFor(const SignatureIdentifier& identifier) {
        auto signature = _componentManager->ToSignature(identifier);
        for (shared<EntityChunk>& chunk : _chunks) {
            if (chunk->GetSignature() == signature && chunk->GetFree() > 0) {
                return chunk;
            }
        }
        auto newChunk = std::make_shared<EntityChunk>(identifier, _componentManager);
        _chunks.push_back(newChunk);
        return newChunk;
    }

    Entity EntityManager::CreateEntity() {
        auto identifier = SignatureIdentifier();
        auto [res, _] = CreateEntityBy(identifier);
        return res;
    }


    std::pair<Entity, shared <EntityChunk>> EntityManager::CreateEntityBy(SignatureIdentifier& identifier) {
        CoreAssert(
            _iterationDepth == 0,
            "Entities cannot be created while iterating over them! Use EntityManager->Defer instead!"
        );

        auto result = Entity(++_runningEntityId);
        CoreAssert(result != Entity::Invalid(), "A created entity cannot have the invalid id");

        auto chunk = GetOrCreateChunkFor(identifier);
        chunk->AllocateEntity(result);
        _entityLocations[result] = chunk;

        return std::make_pair(result, chunk);
    }

    void EntityManager::DestroyEntity(Entity entity) {
        CoreAssert(_iterationDepth == 0,
            "Entities cannot be destroyed while iterating over them! Use EntityManager->Defer instead!")
        CoreAssert(_entityLocations.count(entity) > 0, "You cannot destroy entity {0} since it does not exist!", entity)

        auto* children = GetComponent<WithChildrenData>(entity);
        if (children) {
            std::for_each(children->Values.begin(), children->Values.end(), [this](Entity child) { DestroyEntity(child); });
        }

        auto chunk = _entityLocations[entity];
        chunk->FreeEntityDeferred(entity);
    }


    void* EntityManager::AddComponent(Entity entity, ComponentIdentifier identifier) {

        CoreAssert(_iterationDepth == 0,
            "Entities cannot be modified while iterating over them! Use EntityManager->Defer instead!")
        ensureComponentIsRegistered(identifier);
        auto info = _componentManager->GetInfoOf(identifier);

        // Intentional override of the identifier:
        // The identifier used may be part of static memory of a module that does not "own" the type
        // which may lead to access violation bugs when the module is removed.
        // Here we ensure that the identifier is stored in the static memory of the module that owns it.
        identifier = info.GetIdentifier();

        auto currentChunk = _entityLocations[entity];
        CoreAssert(currentChunk->ContainsEntity(entity),
            "The entity location data is no longer synced with the chunk data!")

        auto destPtr = currentChunk->GetComponentPtr(entity, identifier);
        if (destPtr == nullptr) {
            auto currentIdentifier = currentChunk->GetIdentifier();
            auto destinationIdentifier = SignatureIdentifier(currentIdentifier);
            destinationIdentifier.insert(identifier);

            auto destinationChunk = GetOrCreateChunkFor(destinationIdentifier);

            CoreAssert(currentChunk != destinationChunk,
                "When adding a component to an entity that doesn't have it, its chunk must change!")

            // The current identifier is used, since both chunks contain all components from it
            EntityChunk::MoveEntity(entity, *currentChunk, *destinationChunk, currentIdentifier, _componentManager);

            _entityLocations[entity] = destinationChunk;
            destPtr = destinationChunk->GetComponentPtr(entity, identifier);
        }

        CoreAssert(destPtr != nullptr, "The destPtr must be assigned before the method returns!")

        return destPtr;

    }

    bool EntityManager::RemoveComponent(Entity entity, ComponentIdentifier identifier) {
        CoreAssert(_iterationDepth == 0,
        "Entities cannot be modified while iterating over them! Use EntityManager->Defer instead!")
        ensureComponentIsRegistered(identifier);

        auto info = _componentManager->GetInfoOf(identifier);

        // Intentional override of the identifier:
        // The identifier used may be part of static memory of a module that does not "own" the type
        // which may lead to access violation bugs when the module is removed.
        // Here we ensure that the identifier is stored in the static memory of the module that owns it.
        identifier = info.GetIdentifier();

        auto currentChunk = _entityLocations[entity];
        CoreAssert(currentChunk->ContainsEntity(entity),
        "The entity location data is no longer synced with the chunk data!")

        auto currentIdentifier = currentChunk->GetIdentifier();
        auto destinationIdentifier = ComponentSet(currentIdentifier);
        auto componentExisted = destinationIdentifier.erase(identifier);

        if (!componentExisted)
            return false;

        auto destinationChunk = GetOrCreateChunkFor(destinationIdentifier);

        info.Destruct(currentChunk->GetComponentPtr(entity, identifier));

        // The destination identifier is used, since both chunks contain all components from it
        EntityChunk::MoveEntity(entity, *currentChunk, *destinationChunk, destinationIdentifier, _componentManager);

        _entityLocations[entity] = destinationChunk;

        return true;
    }


    shared<EntityChunk> EntityManager::GetChunk(Entity entity) {
        return _entityLocations.at(entity);
    }

    shared<EntityChunk> EntityManager::getChunkReadWrite(Entity entity) {
        CoreAssert(IsAlive(entity), "Cannot get the chunk of the entity {} that is not alive", entity)
        CoreAssert(_entityLocations.count(entity) > 0, "The alive entity {} has no chunk! This should not happen", entity)
        return _entityLocations.at(entity);
    }

    void EntityManager::Defer(const std::function<void(ref<EntityManager>)>& deferredOperation) {
        CoreAssert(_iterationDepth > 0, "Defer should only be used while iterating. Otherwise it has no effect!")
        _deferredOperations.push_back(deferredOperation);
    }

    void EntityManager::executeDeferredOperations() {
        CoreAssert(_iterationDepth == 0,
            "Deferred operations should only be executed once iteration has ended. This indicates a bug in the entity manager")
        for (auto& operation : _deferredOperations)
            operation(ref(this));
        _deferredOperations.clear();
    }
}
