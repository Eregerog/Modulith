/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "PhysicsModule.h"
#include "PxPhysicsAPI.h"

namespace modulith::physics{

    physx::PxTransform ToPxTransform(modulith::GlobalTransformData& transform) {
        using namespace physx;

        auto position = transform.Position();
        auto pxPosition = PxVec3(position.x, position.y, position.z);

        auto rotation = transform.Rotation();
        auto pxRotation = PxQuat(rotation.x, rotation.y, rotation.z, rotation.w);

        Assert(pxRotation.isSane(), "Could not convert transform to a PxTransform - the rotation was not valid!")

        return PxTransform(pxPosition, pxRotation);
    }

    physx::PxRigidActor*
    ActorFromEither(PxRigidDynamicData* dynamicBody, PxRigidStaticData* staticBody, PxControllerData* characterController) {
        if (dynamicBody != nullptr)
            return static_cast<physx::PxRigidActor*>(dynamicBody->GetRigidBody());
        else if (staticBody != nullptr)
            return static_cast<physx::PxRigidActor*>(staticBody->GetRigidBody());
        else if (characterController != nullptr)
            return static_cast<physx::PxRigidActor*>(characterController->Controller->getActor());
        else Assert(false,
            "Could not retrieve actor form either a dynamic body, static body or character controller. This indicates a mis-use of this method!")
        return nullptr;
    }
}
