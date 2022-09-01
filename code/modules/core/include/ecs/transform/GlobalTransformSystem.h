/**
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "CoreModule.h"
#include "ecs/systems/System.h"
#include "ecs/EntityManager.h"
#include "Context.h"

namespace modulith{

    /**
     * Updates the GlobalTransformData based on the WithChildrenData and LocalTransformData each frame.
     */
    class CORE_API GlobalTransformSystem : public System {
    public:
        explicit GlobalTransformSystem() : System("GlobalTransformSystem"){}

        ~GlobalTransformSystem() override = default;

        void OnUpdate(float deltaTime) override;

        static void UpdateGlobalTransformsBelow(ref<EntityManager> ecs, Entity entity);

    private:
        static void updateGlobalTransformRec(ref<EntityManager> manager, Entity current, float4x4 currentObjectToWorld, bool parentDisabled);
    };

}
