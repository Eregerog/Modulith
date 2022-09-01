/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "ShooterModule.h"
#include "GunComponents.h"

struct GunSystem : public modulith::System {
public:
    GunSystem() : modulith::System("Gun System") {}

    void OnUpdate(float deltaTime) override;

private:
    static void instantiateBullet(modulith::ref<modulith::EntityManager> ecs, modulith::Entity gun, float force, const modulith::shared<modulith::Prefab>& ammo);
};