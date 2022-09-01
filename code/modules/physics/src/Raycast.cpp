/*
 * \brief
 * \author Daniel Götz
 */

#include "PhysicsModule.h"
#include "Raycast.h"
#include "PhysXSystem.h"
#include "InternalPhysXComponents.h"

using namespace physx;
using namespace modulith;

namespace modulith::physics{

    bool Raycast(Context& ctx, float3 from, float3 direction, float maxDistance, HitInfo& hitInfo) {
        return Raycast(ctx, Ray(from, direction), maxDistance, hitInfo);
    }

    bool Raycast(Context& ctx, Ray ray, float maxDistance, HitInfo& hitInfo) {
        const auto& physicsSystem = ctx.Get<ECSContext>()->TryGetSystem<PhysXSystem>().value();
        Assert(physicsSystem, "You cannot raycast without a registered PhysX system!");


        auto pxFrom = PxVec3(ray.Origin.x, ray.Origin.y, ray.Origin.z);
        auto pxDir = PxVec3(ray.Direction.x, ray.Direction.y, ray.Direction.z).getNormalized();
        auto pxHitBuffer = PxRaycastBuffer();
        auto hitAnything = physicsSystem->GetScene()->raycast(pxFrom, pxDir, PxReal(maxDistance), pxHitBuffer);

        if (hitAnything) {
            Assert(pxHitBuffer.hasBlock,
                "Unexpected error: The raycast did not have any result even though it hit something.")
            auto point = pxHitBuffer.block.position;
            hitInfo.Point = {point.x, point.y, point.z};

            auto normal = pxHitBuffer.block.normal;
            hitInfo.Normal = {normal.x, normal.y, normal.z};

            hitInfo.Distance = pxHitBuffer.block.distance;

            hitInfo.Entity = *reinterpret_cast<Entity*>(pxHitBuffer.block.actor->userData);

            if (hitInfo.Entity == Entity::Invalid())
            LogWarn("No matching entity found for raycasted rigidbody");
        }

        return hitAnything;
    }


}
