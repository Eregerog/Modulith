/*
 * \brief
 * \author Daniel Götz
 */


#include <ecs/transform/TransformComponents.h>
#include "ecs/Prefab.h"
#include "ecs/EntityManager.h"

namespace modulith{

    shared<Prefab> Prefab::CreateFromEntity(const ref<ComponentManager>& componentManager, const ref<EntityManager>& entityManager, Entity entity) {
        CoreAssert(entityManager->IsAlive(entity), "Cannot create a prefab from entity {} because it is not alive", entity.GetId())
        auto chunk = entityManager->GetChunk(entity);
        auto res = std::make_shared<Prefab>(chunk->GetIdentifier(), componentManager);
        for(auto& component : res->_identifier){
            auto info = componentManager->GetInfoOf(component);
            auto srcPtr = chunk->GetComponentPtr(entity, component);
            auto destPtr = res->GetComponentPtr(component);
            info.CreateCopyIn(destPtr, srcPtr);
        }
        return res;
    }


    Prefab::Prefab(const SignatureIdentifier& identifier, const ref<ComponentManager>& componentManager) : _componentManager(componentManager){
        _identifier = identifier;
        _size = 0;
        for (auto& component : _identifier) {
            RegisteredComponent componentInfo = componentManager->GetInfoOf(component);
            CoreAssert(componentInfo.IsCopyable(), "Cannot make a prefab from the non-copyable component {}", componentInfo.GetFullName())
            _offsets[component] = _size;
            _size += componentInfo.GetSize();
            _signature.set(componentInfo.GetIndex());
        }

        _buffer = new std::byte[_size];
    }

    void* Prefab::GetComponentPtr(ComponentIdentifier component) {
        // This method can be called with non-contained components and null will be returned
        if (_offsets.count(component) == 0)
            return nullptr;
        return _buffer + _offsets[component];
    }

    Entity Prefab::InstantiateIn(const ref<EntityManager>& entityManager) {
        auto entity = Entity(++entityManager->_runningEntityId);
        auto chunk = entityManager->GetOrCreateChunkFor(_identifier);
        chunk->AllocateEntity(entity);
        for(const auto& component : _identifier){
            auto info = _componentManager->GetInfoOf(component);
            auto* destPtr = chunk->GetComponentPtr(entity, component);
            auto* srcPtr = GetComponentPtr(component);
            info.CreateCopyIn(destPtr, srcPtr);
        }
        entityManager->_entityLocations[entity] = chunk;
        return entity;
    }

    Entity Prefab::InstantiateAt(const ref<EntityManager>& entityManager, float3 position, quat rotation) {
        auto res = InstantiateIn(entityManager);
        res.Add(entityManager, PositionData(position));
        res.Add(entityManager, RotationData(rotation));
        return res;
    }

}
