/**
 * \brief
 * \author Daniel Götz
 */

#include "GameModule.h"
#include <PhysicsComponents.h>
#include "ecs/DestroyOnCollisionSystem.h"
#include "ecs/Components.h"

using namespace modulith;
using namespace modulith::physics;

void DestroyOnCollisionSystem::OnUpdate(float deltaTime) {

    auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();

    ecs->QueryAll(
        Each<PhysicsContactsData, DestroyOnCollisionTag>(),
        [ecs](Entity entity, PhysicsContactsData& contacts, auto& tag) {
            if (contacts.Count > 0)
                entity.DestroyDeferred(ecs);
        }
    );

}
