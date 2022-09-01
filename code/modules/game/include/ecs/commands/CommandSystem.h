/**
 * \brief
 * \author Daniel Götz
 */

#pragma once


#include "GameModule.h"


class CommandSystem : public modulith::System {
public:
    CommandSystem() : modulith::System("CommandSystem") {}

    void OnUpdate(float deltaTime) override;

private:

    void updateMoveTo(float deltaTime);
};

