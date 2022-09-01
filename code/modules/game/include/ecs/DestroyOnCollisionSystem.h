/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "GameModule.h"

class DestroyOnCollisionSystem : public modulith::System {
public:
    DestroyOnCollisionSystem() : modulith::System("DestroyOnCollisionSystem") {}

    void OnUpdate(float deltaTime) override;
};

