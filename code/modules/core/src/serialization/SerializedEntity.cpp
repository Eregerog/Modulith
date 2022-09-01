/**
 * \brief
 * \author Daniel Götz
 */

#include "serialization/SerializedEntity.h"

modulith::SerializedEntity modulith::SerializedEntity::From(modulith::Entity e) {
    auto res = SerializedEntity();
    res._self = e;

    auto ecsCtx = Context::GetInstance<ECSContext>();
    auto serializationCtx = Context::GetInstance<SerializationContext>();
    auto chunk = ecsCtx->GetEntityManager()->GetChunk(e);

    for (auto& component : chunk->GetIdentifier()) {
        auto info = ecsCtx->GetComponentManager()->GetInfoOf(component);
        if (info.IsSerializable() && serializationCtx->HasSerializable(component)) {
            // TODO DG: Having a function in the chunk that iterates over the components would be even better
            auto asAny = info.CopyFromPointerToAny(chunk->GetComponentPtr(e, component));
            auto serializedComponent = serializationCtx->TryGetSerializableFor(component).value()->Serialize(
                asAny
            );

            // The name will encode the type of component used
            res._components.emplace_back(info.GetFullName(), serializedComponent.GetUnderlyingValue());
        }
    }

    return res;
}
