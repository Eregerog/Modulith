/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "PhysicsModule.h"

namespace modulith::physics{

    /**
     * The information of a sucessful raycast
     */
    struct PHYSICS_API HitInfo{
        /**
         * The entity that was hit
         */
        modulith::Entity Entity = Entity::Invalid();
        /**
         * The position (in worldspace) that was hit
         */
        modulith::float3 Point{};
        /**
         * The normal of the entitie's collider that was hit
         */
        modulith::float3 Normal{};
        /**
         * The distance of the Point to the raycast origin
         */
        float Distance{};
    };

    /**
     * Performs a raycast in the scene using the provided hit
     * @param ctx The engine's context
     * @param ray The ray used for raycasting
     * @param maxDistance The maximum distance to raycast for
     * @param hitInfo If the function returns true, this contains additional information about the hit
     * @return Whether the raycast was successful and a collider was hit
     */
    PHYSICS_API bool Raycast(modulith::Context& ctx, modulith::Ray ray, float maxDistance, HitInfo& hitInfo);


    /**
     * Performs a raycast in the scene using the provided hit
     * @param ctx The engine's context
     * @param from The origin of the raycast
     * @direction The direction of the raycast
     * @param maxDistance The maximum distance to raycast for
     * @param hitInfo If the function returns true, this contains additional information about the hit
     * @return Whether the raycast was successful and a collider was hit
     */
    PHYSICS_API bool Raycast(modulith::Context& ctx, modulith::float3 from, modulith::float3 direction, float maxDistance, HitInfo& hitInfo);

}
