/**
 * \brief
 * \author Daniel Götz
 */

#include "ShooterModule.h"
#include <PhysicsComponents.h>
#include "ecs/guns/GunSystem.h"

using namespace modulith;
using namespace modulith::physics;

void GunSystem::instantiateBullet(ref<EntityManager> ecs, Entity gun, float force, const shared<Prefab>& ammo) {
    auto gunTransform = ecs->GetComponent<GlobalTransformData>(gun);
    auto bulletRotation = glm::rotation(float3(0, 0, -1), gunTransform->Forward());
    ecs->Defer(
        [gunTransform, ammo, force, bulletRotation](ref<EntityManager> ecs) {
            auto bullet = ammo->InstantiateIn(ecs);
            ecs->AddComponents(
                bullet,
                PositionData(gunTransform->Position() - (gunTransform->Forward() * gunTransform->Scale().z)),
                RotationData(bulletRotation),
                RigidbodyData(0.1f, -gunTransform->Forward() * force)
            );
            TransformUtils::UpdateTransformOf(ecs, bullet);
        }
    );
}

void GunSystem::OnUpdate(float deltaTime) {
    auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();

    ecs->QueryActive(
        Each<GunData>(), [deltaTime](auto entity, GunData& gun) {
            gun.RemainingShootCooldown = std::max(gun.RemainingShootCooldown - deltaTime, 0.0f);
        }
    );

    // RELOADING
    {
        ecs->QueryActive(
            Each<GunData, AmmunitionData, IsReloadingTag>(),
            [&ecs, deltaTime](Entity entity, GunData& gun, AmmunitionData& ammunition, auto& _) {
                if (!ammunition.IsFull() && gun.RemainingReloadDuration == 0.0f) {
                    // Begin the reload
                    gun.RemainingReloadDuration = gun.ReloadTime;
                } else if (ammunition.IsFull() || gun.RemainingReloadDuration - deltaTime <= 0.0f) {
                    // End the reload
                    gun.RemainingReloadDuration = 0;
                    ammunition.Current = ammunition.Maximum;
                    entity.RemoveDeferred<IsReloadingTag>(ecs);
                } else {
                    // Progress the reload
                    gun.RemainingReloadDuration -= deltaTime;
                }
            }
        );

        // Can't aim while reloading!
        ecs->QueryActive(
            Each<IsReloadingTag, IsAimingTag>(), [&ecs](auto entity, auto& _0, auto& _1) {
                entity.template RemoveDeferred<IsAimingTag>(ecs);
            }
        );
    }

    // SHOOTING
    {
        ecs->QueryActive(
            Each<GunData, AmmunitionData, IsShootingTag>(), Any(), None<IsReloadingTag>(), Has<ManualShootingTag>(), [ecs, this]
                (auto gunEntity, GunData& gunData, AmmunitionData& ammunitionData, auto& _, bool isManual) {
                if (ammunitionData.IsNotEmpty()) {
                    ammunitionData.Current--;
                    gunData.RemainingShootCooldown = gunData.ShootEvery;
                    instantiateBullet(ecs, gunEntity, gunData.Force, ammunitionData.Ammunition);
                }
                if (isManual) {
                    gunEntity.template RemoveDeferred<IsShootingTag>(ecs);
                }
            }
        );
    }

    // AIMING
    {
        ecs->QueryActive(
            Each<GunAimData, IsAimingTag>(), None<InitializedTag<IsAimingTag>, IsReloadingTag>(),
            [&ecs](auto entity, GunAimData& aimData, auto& _) {
                aimData.CameraAttachment.RemoveDeferred<DisabledTag>(ecs);
                entity.AddDeferred(ecs, InitializedTag<IsAimingTag>());
            }
        );

        ecs->QueryActive(
            Each<GunAimData, InitializedTag<IsAimingTag>>(), None<IsAimingTag>(),
            [&ecs](auto entity, GunAimData& aimData, auto& _) {
                aimData.CameraAttachment.AddDeferred(ecs, DisabledTag());
                entity.template RemoveDeferred<InitializedTag<IsAimingTag>>(ecs);
            }
        );
    }

    // AMMUNITION VISUALIZATION
    {
        ecs->QueryAll(
            Each<AmmunitionData, VisualizedAmmunitionData>(), None<InitializedTag<VisualizedAmmunitionData>>(),
            [ecs](Entity entity, AmmunitionData& ammunition, VisualizedAmmunitionData& visualizedAmmunition) {
                auto capacity = std::max(
                    ammunition.Maximum, (short) visualizedAmmunition.VisualizationParents.size());

                ecs->Defer(
                    [entity, capacity, visualizedAmmunition, current = ammunition.Current](
                        ref<EntityManager> ecs
                    ) {
                        for (int index = 0; index < capacity; ++index) {
                            auto parent = visualizedAmmunition.VisualizationParents[index];
                            auto instance = visualizedAmmunition.Visualization->InstantiateIn(ecs);
                            instance.Add(ecs, WithParentData(parent));
                            parent.SetIf<DisabledTag>(ecs, index >= current);
                        }
                        ecs->AddComponent<InitializedTag<VisualizedAmmunitionData>>(entity);
                    }
                );
            }
        );

        ecs->QueryActive(
            Each<AmmunitionData, VisualizedAmmunitionData>(),
            [ecs](Entity entity, AmmunitionData& ammunition, VisualizedAmmunitionData& visualizedAmmunition) {
                auto capacity = std::max(
                    ammunition.Maximum, (short) visualizedAmmunition.VisualizationParents.size());

                auto current = ammunition.Current;

                for (int index = 0; index < capacity; ++index) {
                    auto parent = visualizedAmmunition.VisualizationParents[index];
                    if (parent.Has<DisabledTag>(ecs) && index < current)
                        parent.RemoveDeferred<DisabledTag>(ecs);
                    else if (!parent.Has<DisabledTag>(ecs) && index >= current)
                        parent.AddDeferred<DisabledTag>(ecs);
                }
            }
        );
    }
}
