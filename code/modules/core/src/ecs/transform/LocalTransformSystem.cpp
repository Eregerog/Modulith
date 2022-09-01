/*
 * \brief
 * \author Daniel Götz
 */

#include <ecs/transform/TransformComponents.h>
#include "ecs/transform/LocalTransformSystem.h"
#include <ecs/ECSContext.h>
#include "Context.h"

namespace modulith{

    void LocalTransformSystem::OnUpdate(float deltaTime) {

        auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();

        ecs->QueryAll(
            Any<LocalTransformData, PositionData, RotationData, ScaleData>(),
            [ecs](auto entity, auto* localTransform, auto* position, auto* rotation, auto* scale) {

                auto localTransformMatrix = CalculateLocalTransform(position, rotation, scale);

                if (localTransform == nullptr) {
                    entity.AddDeferred(ecs, LocalTransformData(localTransformMatrix));
                } else {
                    localTransform->Value = localTransformMatrix;
                }
            }
        );
    }

    float4x4 LocalTransformSystem::CalculateLocalTransform(PositionData* position, RotationData* rotation, ScaleData* scale) {
        auto translationMatrix =
            position != nullptr ? glm::translate(float4x4(1.0f), position->Value) : float4x4(1.0f);
        auto rotationMatrix = rotation != nullptr ? glm::mat4_cast(rotation->Value) : float4x4(1.0f);
        auto scaleMatrix = scale != nullptr ? glm::scale(float4x4(1.0f), scale->Value) : float4x4(1.0f);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

}
