

#include "GameModule.h"
#include "ecs/LifetimeSystem.h"
#include "ecs/Components.h"

using namespace modulith;

void LifetimeSystem::OnUpdate(float deltaTime) {

    auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();

    ecs->QueryActive(
        Each<LifetimeData>(),
        [ecs, deltaTime](auto entity, auto& lifetime) {
            if (lifetime.Current < lifetime.Maximum) {
                lifetime.Current += deltaTime;
            } else {
                entity.DestroyDeferred(ecs);
            }
        }
    );
}
