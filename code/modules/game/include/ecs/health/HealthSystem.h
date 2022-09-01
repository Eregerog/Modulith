/**
 * \brief
 * \author Daniel Götz
 */

#pragma once


#include "GameModule.h"


class HealthSystem : public modulith::System {
public:
    HealthSystem() : modulith::System("HealthSystem") {}

    void OnUpdate(float deltaTime) override;
};

