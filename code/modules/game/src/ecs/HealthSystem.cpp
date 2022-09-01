/**
 * \brief
 * \author Daniel Götz
 */

#include <ecs/Components.h>
#include "GameModule.h"
#include "ecs/health/HealthSystem.h"
#include "ecs/health/HealthComponents.h"
#include "PhysicsComponents.h"
#include "ecs/building/BuildingComponents.h"
#include "GameState.h"

using namespace modulith;
using namespace modulith::physics;

void HealthSystem::OnUpdate(float deltaTime) {
    auto& ctx = Context::Instance();
    auto ecs = ctx.Get<ECSContext>()->GetEntityManager();
    auto gameState = ctx.Get<GameState>();

    ecs->QueryActive(Each<HealthData, PhysicsContactsData>(), [ecs](auto entity, auto& healthData, PhysicsContactsData& physicsContacts){
        for(Entity contact : physicsContacts.BeginContact){
            auto* damageOnContact = contact.Get<DamageOnContactData>(ecs);
            if(damageOnContact){
                healthData.Health -= damageOnContact->Damage;
            }
        }
    });

    ecs->QueryActive(Each<HealthData, RewardsOnDeathData>(), [ecs, &gameState](auto e, auto& health, auto& reward){
        if(health.Health <= 0){
            gameState->ModifySpiritResource(reward.SpiritGained);
            gameState->ModifyScore(reward.ScoreGained);
            e.template RemoveDeferred<RewardsOnDeathData>(ecs);
        }
    });

    ecs->QueryActive(Each<HealthData, DestroyOnNoHealthTag>(), [ecs](Entity entity, auto& healthData, auto& _){
        if(healthData.Health <= 0){
            entity.DestroyDeferred(ecs);}
    });

    auto damageSourcePositionPairs = std::vector<std::tuple<float, float, float3>>();

    ecs->QueryActive(Each<DamageNearbyEnemiesData, GlobalTransformData>(), [deltaTime, &damageSourcePositionPairs](auto e, auto& damageSource, auto& transform){
        damageSourcePositionPairs.emplace_back(damageSource.Radius, damageSource.DamagePerSecond * deltaTime, transform.Position());
    });

    ecs->QueryActive(Each<ExplodeData, GlobalTransformData>(), [ecs, &damageSourcePositionPairs](auto e, auto& explode, auto& transform){
        damageSourcePositionPairs.emplace_back(explode.Radius, explode.Damage, transform.Position());
        e.DestroyDeferred(ecs);
    });

    ecs->QueryActive(Each<HealthData, GlobalTransformData, EnemyTag>(), [&damageSourcePositionPairs, deltaTime](auto e, auto& health, auto& transform, auto& _){
       auto damageSum = 0.0f;
       for(auto pair : damageSourcePositionPairs){
           if(glm::distance(std::get<2>(pair), transform.Position()) <= std::get<0>(pair)){
               damageSum += std::get<1>(pair);
           }
       }
       health.Health -= damageSum;
    });
}
