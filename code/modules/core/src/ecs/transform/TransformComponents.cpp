/**
 * \brief
 * \author Daniel Götz
 */

#include "ecs/transform/TransformComponents.h"
#include <glm/gtx/matrix_decompose.hpp>

namespace modulith{

    float3 LocalTransformData::Right() const {
        return glm::normalize(Value[0]);
    }

    float3 LocalTransformData::Up() const {
        return glm::normalize(Value[1]);
    }

    float3 LocalTransformData::Forward() const {
        return glm::normalize(Value[2]);
    }

    float3 LocalTransformData::Position() const {
        return Value[3];
    }

    quat LocalTransformData::Rotation() const {
        return glm::quat_cast(UnscaledTransform());
    }

    float3 LocalTransformData::Scale() const {
        return float3(
            glm::length(Value[0]), glm::length(Value[1]), glm::length(Value[2]));
    }

    float4x4 LocalTransformData::UnscaledTransform() const {
        return float4x4(
            glm::normalize(Value[0]),
            glm::normalize(Value[1]),
            glm::normalize(Value[2]),
            Value[3]
        );
    }

    float3 GlobalTransformData::Right() const {
        return glm::normalize(Value[0]);
    }

    float3 GlobalTransformData::Up() const {
        return glm::normalize(Value[1]);
    }

    float3 GlobalTransformData::Forward() const {
        return glm::normalize(Value[2]);
    }

    float3 GlobalTransformData::Position() const {
        return Value[3];
    }

    quat GlobalTransformData::Rotation() const {
        return glm::quat_cast(UnscaledTransform());
    }


    float3 GlobalTransformData::Scale() const {
        return float3(
            glm::length(Value[0]), glm::length(Value[1]), glm::length(Value[2]));
    }

    float4x4 GlobalTransformData::UnscaledTransform() const {
        return float4x4(
            glm::normalize(Value[0]),
            glm::normalize(Value[1]),
            glm::normalize(Value[2]),
            Value[3]
        );
    }
}
