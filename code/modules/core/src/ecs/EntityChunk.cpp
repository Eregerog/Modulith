/**
 * \brief
 * \author Daniel Götz
 */

#include "ecs/EntityChunk.h"

namespace modulith{

    EntityChunk::EntityChunk(
        const SignatureIdentifier& signature, const ref<ComponentManager>& componentManager
    ) : _componentManager(componentManager) {
        _entitySize = sizeof(Entity);
        _identifier = signature;
        for (auto& component : signature) {
            RegisteredComponent componentInfo = componentManager->GetInfoOf(component);
            _offsets[component] = _entitySize;
            _entitySize += componentInfo.GetSize();
            _signature.set(componentInfo.GetIndex());
        }

        _aliveCount = 0;
        _deadCount = 0;

        _capacity = (MODU_CHUNK_SIZE_BYTES / _entitySize) - 1; // The capacity is one less than the possible capacity: The 'last' slot is used for swapping
        CoreAssert(
            _capacity >= 2,
            "The signature of this chunks exceeded the limit of {} bytes per entity with {} bytes per entity. "
            "A chunk can only hold {} bytes of data, and there must be room for at least 2 entities",
            2 * MODU_CHUNK_SIZE_BYTES, _entitySize, MODU_CHUNK_SIZE_BYTES
        )

        if (_capacity < 5)
        CoreLogWarn("A chunk with only a capacity for {} entities was created, with a size of {} bytes per entity. This is very close to the limit!", _capacity, _entitySize)
    }

    EntityChunk::~EntityChunk() {
        destructEntityComponents(0, _aliveCount + _deadCount);
    }

    bool EntityChunk::ContainsEntity(Entity entity, bool mustBeAlive) const {
        return _entityIndices.count(entity) > 0 && (!mustBeAlive || _entityIndices.at(entity) < _aliveCount);
    }

    bool EntityChunk::ContainsComponent(const ComponentIdentifier& componentType) const {
        return _identifier.count(componentType) > 0;
    }

    void* EntityChunk::GetComponentPtr(Entity entity, ComponentIdentifier component) {
        CoreAssert(ContainsEntity(entity),
            "The entities' pointer cannot be gotten because it does not exist in this chunk!");
        // This method can be called with non-contained component so For(Any<...>) can return null for components not present
        if (_offsets.count(component) == 0)
            return nullptr;
        auto index = _entityIndices[entity];
        auto offset = (index * _entitySize) + _offsets[component];
        return _buffer + offset;
    }


    void EntityChunk::AllocateEntity(Entity entity) {
        CoreAssert(_entityIndices.count(entity) == 0,
            "Cannot allocate entity {} because it is already present in the chunk", entity)
        CoreAssert(_aliveCount < _capacity, "No more entities can be allocated in this chunk - it is full!")
        auto allocatedIndex = _aliveCount;
        _aliveCount++;

        _entityIndices[entity] = allocatedIndex;
        auto destination = _buffer + (_entitySize * allocatedIndex);
        
        // Zero-initialize the buffer when an entity is allocated, so any "zero-initialized" component can be safely destructed
        memset(destination, 0, _entitySize);
        memcpy(destination, &entity, sizeof(Entity));
    }

    void EntityChunk::MoveEntity(
        Entity entity, EntityChunk& from, EntityChunk& to, const SignatureIdentifier& identifier,
        ref<ComponentManager>& manager
    ) {
        to.AllocateEntity(entity);
        for (auto& componentType : identifier) {
            memmove(
                to.GetComponentPtr(entity, componentType), from.GetComponentPtr(entity, componentType),
                manager->GetInfoOf(componentType).GetSize());
        }
        from.FreeEntityImmediately(entity);
    }

    void EntityChunk::FreeEntityDeferred(Entity entity) {
        CoreAssert(_aliveCount > 0, "Cannot free an entity when there are none in the chunk")
        CoreAssert(ContainsEntity(entity, true), "The entity cannot be freed because it is not alive in this chunk!");

        makeLastAliveEntity(entity);

        _aliveCount--;
        _deadCount++;
    }

    void EntityChunk::FreeEntityImmediately(Entity entity) {
        CoreAssert(_aliveCount > 0, "Cannot free an entity when there are none in the chunk")
        CoreAssert(ContainsEntity(entity), "The entity cannot be freed because it does not exist in this chunk!");

        makeLastAliveEntity(entity);

        _aliveCount--;
        _entityIndices.erase(entity);
    }

    void EntityChunk::makeLastAliveEntity(Entity entity) {
        auto lastAliveIndex = _aliveCount - 1;
        auto lastEntity = entityAt(lastAliveIndex);

        if (entity == lastEntity)
            return;

        auto entityIndex = _entityIndices.at(entity);

        auto* entityPtr = _buffer + (entityIndex * _entitySize);
        auto* lastPtr = _buffer + (lastAliveIndex * _entitySize);
        auto* tempPtr = _buffer + (_capacity * _entitySize);

        memmove(tempPtr, entityPtr, _entitySize);
        memmove(entityPtr, lastPtr, _entitySize);
        memmove(lastPtr, tempPtr, _entitySize);

        _entityIndices[entity] = lastAliveIndex;
        _entityIndices[lastEntity] = entityIndex;

        CoreAssert(entityAt(entityIndex) == lastEntity, "The makeLastAliveEntity function is not implemented correctly")
        CoreAssert(entityAt(lastAliveIndex) == entity, "The makeLastAliveEntity function is not implemented correctly")
    }

    Entity EntityChunk::entityAt(uint32_t index) const {
        CoreAssert(index < GetOccupied(),
            "There is no entity at index {} since there are only {} entities total in this chunk", index, GetOccupied())
        return *reinterpret_cast<const Entity*>(_buffer + (_entitySize * index));
    }

    std::vector<Entity> EntityChunk::CleanupDeadEntitiesAtEndOfFrame() {
        auto res = std::vector<Entity>(_deadCount);
        destructEntityComponents(_aliveCount, _aliveCount + _deadCount);
        for (auto index = _aliveCount; index < _aliveCount + _deadCount; ++index) {
            auto entity = entityAt(index);
            _entityIndices.erase(entity);
            res.push_back(entity);
        }
        _deadCount = 0;
        return res;
    }

    void EntityChunk::destructEntityComponents(uint32_t fromIndex, uint32_t toIndex) {
        for (const auto& component : GetIdentifier()) {
            auto info = _componentManager->GetInfoOf(component);
            for(uint32_t current = fromIndex; current < toIndex; ++current){
                info.Destruct(GetComponentPtr(entityAt(current), component));
            }
        }
    }
}
