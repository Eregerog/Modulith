/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "CoreModule.h"
#include "ecs/systems/System.h"
#include "ecs/transform/TransformComponents.h"

namespace modulith{

    /**
     * Updates the LocalTransformData based on the PositionData, RotationData and ScaleData of an entity
     */
    class CORE_API LocalTransformSystem : public System {
    public:
        explicit LocalTransformSystem() : System("LocalTransformSystem") {}
        ~LocalTransformSystem() override = default;
        void OnUpdate(float deltaTime) override;

        static float4x4 CalculateLocalTransform(PositionData* position, RotationData* rotation, ScaleData* scale);
    };

}
