/**
 * \brief
 * \author Daniel Götz
 */

#include "GameModule.h"
#include <PhysicsComponents.h>
#include <ecs/commands/CommandComponents.h>
#include "ecs/commands/CommandSystem.h"


using namespace modulith;
using namespace modulith::physics;

void CommandSystem::OnUpdate(float deltaTime) {
    updateMoveTo(deltaTime);
}

struct EffectDescription{
    EffectDescription(const float3& position, float strength, bool walkTowards) : Position(position),
                                                                                  Strength(strength),
                                                                                  WalkTowards(walkTowards) {}

    float3 Position;
    float Strength;
    bool WalkTowards;
};

void CommandSystem::updateMoveTo(float deltaTime) {
    auto& ctx = Context::Instance();
    auto ecs = ctx.Get<ECSContext>()->GetEntityManager();

    // position, strength, walk towards
    auto effects = std::vector<EffectDescription>();

    ecs->QueryActive(Each<GlobalTransformData, LureEffectData>(), [&effects](auto e, auto& transform, auto& lure){
        effects.emplace_back(transform.Position(), lure.Strength, true);
    });

    ecs->QueryActive(Each<GlobalTransformData, FearEffectData>(), [&effects](auto e, auto& transform, auto& fear){
        effects.emplace_back(transform.Position(), fear.Strength, false);
    });

    ecs->QueryActive(
        Each<ControlledByEffectsData, GlobalTransformData>(),
        Any<ControlledByEffectsData, MoveToData, LookAtData>(), None<>(),
        [&effects, &ecs](
            auto e, auto& data, auto& transform, auto* _, auto* moveTo, auto* lookAt
        ) {
            if (effects.empty()) {
                if (moveTo != nullptr) e.template RemoveDeferred<MoveToData>(ecs);
                if (lookAt != nullptr) e.template RemoveDeferred<LookAtData>(ecs);
            } else {
                auto strongestEffect = std::max_element(
                    effects.begin(), effects.end(), [&transform](auto lhs, auto rhs) {
                        const auto calculateLocalThreatOf = [&transform](auto effect) {
                            auto distance = glm::distance(transform.Position(), effect.Position);
                            return effect.Strength / distance;
                        };
                        return calculateLocalThreatOf(lhs) < calculateLocalThreatOf(rhs);
                    }
                );
                Assert(strongestEffect != effects.end(), "There must be an effect")

                auto destination = strongestEffect->WalkTowards ? strongestEffect->Position : transform.Position() + (
                    transform.Position() - strongestEffect->Position
                );
                auto speed = strongestEffect->WalkTowards ? data.LuredSpeed : data.FearedSpeed;

                if (moveTo != nullptr) {
                    moveTo->Destination = destination;
                    moveTo->Speed = speed;
                } else e.AddDeferred(ecs, MoveToData{destination, speed});

                if (lookAt != nullptr) {
                    lookAt->Destination = destination;
                } else e.AddDeferred(ecs, LookAtData{destination});
            }
        }
    );


    ecs->QueryActive(
        Each<PositionData, RotationData, CharacterControllerData, MoveToData>(), None<WithParentData>(),
        [deltaTime](auto entity, auto& position, auto& rotation, auto& characterController, auto& moveTo) {
            auto displacement = glm::normalize(moveTo.Destination - position.Value);
            characterController.CurrentDisplacement = displacement * moveTo.Speed * deltaTime;
        }
    );

    ecs->QueryActive(
        Each<PositionData, RotationData, LookAtData>(), None<WithParentData>(),
        [deltaTime](auto entity, auto& position, auto& rotation, auto& lookAt) {
            auto direction = glm::normalize(lookAt.Destination - position.Value);
            auto lookAtDirection = float3(direction.x, 0, direction.z);
            rotation.SetLookAt(lookAtDirection);
        }
    );
}
