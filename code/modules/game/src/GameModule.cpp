/*
 * \brief
 * \author Daniel Götz
 */

#include "GameModule.h"

#include "resources/SubcontextResource.h"
#include "GameState.h"

#include <ecs/LevelSetupSystem.h>
#include <ecs/building/BuildingContext.h>
#include <ecs/StrategyCameraController.h>
#include <ecs/LifetimeSystem.h>
#include <ecs/health/HealthSystem.h>
#include <ecs/DestroyOnCollisionSystem.h>
#include <ecs/Components.h>
#include <ecs/health/HealthComponents.h>
#include <ecs/commands/CommandSystem.h>
#include <ecs/commands/CommandComponents.h>
#include <ecs/building/BuildingComponents.h>

using namespace modulith;

extern "C++" {

void __declspec(dllexport) __cdecl Initialize(modulith::ModuleResources& module) {
    module.Register<SubcontextResource<GameState>>();
    module.Register<SubcontextResource<BuildingContext>>();

    module.Register<SystemResource<LevelSetupSystem, InGroup<InputSystemsGroup>>>();
    module.Register<SystemResource<StrategyCameraController, InGroup<InputSystemsGroup>>>();

    module.Register<SystemResource<LifetimeSystem>>();
    module.Register<SystemResource<HealthSystem>>();
    module.Register<SystemResource<CommandSystem>>();

    // Needs to come after the health system, so colliding objects are not destroyed before the collision is registered
    // TODO: Is this still needed since we have deferred entity destruction?
    module.Register<SystemResource<DestroyOnCollisionSystem, InGroup<LogicSystemsGroup>, ExecuteAfter<HealthSystem>>>();

    module.Register<ComponentResource<LifetimeData>>("Lifetime");
    module.Register<ComponentResource<DestroyOnCollisionTag>>("DestroyOnCollision");

    module.Register<ComponentResource<HealthData>>("Health");
    module.Register<SerializerResource<HealthData>>();
    module.Register<ComponentResource<RewardsOnDeathData>>("Rewards On Death");
    module.Register<ComponentResource<DamageOnContactData>>("DamageOnContact");
    module.Register<ComponentResource<DestroyOnNoHealthTag>>("DestroyOnNoHealth");

    module.Register<ComponentResource<EnemyTag>>("Enemy");
    module.Register<ComponentResource<SpawnEnemiesTag>>("Spawn Enemies");

    module.Register<ComponentResource<MoveToData>>("MoveTo");
    module.Register<ComponentResource<LookAtData>>("LookAt");
    module.Register<ComponentResource<FearEffectData>>("Fear Effect");
    module.Register<ComponentResource<LureEffectData>>("Lure Effect");
    module.Register<ComponentResource<ControlledByEffectsData>>("Controlled By Effects");


    module.Register<ComponentResource<LampPostData>>("Lamp Post");
    module.Register<ComponentResource<LanternData>>("Lantern");
    module.Register<ComponentResource<GroundTag>>("Ground");

    module.Register<ComponentResource<ExplodeData>>("Explosion");
    module.Register<ComponentResource<DamageNearbyEnemiesData>>("Damage Nearby Enemies");
}


void __declspec(dllexport) __cdecl Shutdown(modulith::ModuleResources& module){
}


}
