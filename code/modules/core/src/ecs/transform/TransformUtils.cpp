/**
 * \brief
 * \author Daniel Götz
 */

#include "ecs/transform/TransformUtils.h"
#include "ecs/transform/TransformComponents.h"
#include "ecs/transform/LocalTransformSystem.h"
#include "ecs/transform/GlobalTransformSystem.h"

namespace modulith{

    void TransformUtils::UpdateTransformOf(ref<EntityManager> ecs, Entity entity) {

        // TODO DG: Get multiple components at once for more performance?
        auto localTransform = LocalTransformSystem::CalculateLocalTransform(
            ecs->GetComponent<PositionData>(entity),
            ecs->GetComponent<RotationData>(entity),
            ecs->GetComponent<ScaleData>(entity)
        );
        auto localTransformComponent = ecs->GetComponent<LocalTransformData>(entity);

        if (localTransformComponent)
            localTransformComponent->Value = localTransform;
        else
            ecs->AddComponent(entity, LocalTransformData(localTransform));

        GlobalTransformSystem::UpdateGlobalTransformsBelow(ecs, entity);
    }


    void TransformUtils::ForAllChildren(ref<EntityManager> ecs, Entity entity, const std::function<void(ref<EntityManager>, Entity)>& fn) {
        if (ecs->IsAlive(entity)) {
            fn(ecs, entity);
            auto children = entity.Get<WithChildrenData>(ecs);
            if (children) {
                for (auto child : children->Values) {
                    ForAllChildren(ecs, child, fn);
                }
            }
        }
    }
}
