/*
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "GameModule.h"

class LifetimeSystem : public modulith::System{
public:
    LifetimeSystem() : modulith::System("Lifetime System"){}

    void OnUpdate(float deltaTime) override;
};
