/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "PhysicsModule.h"

namespace modulith::physics{

    class PhysicsSystemsGroup : public modulith::SystemsGroup {
    public:
        std::string GetName() override {
            return "Physics Systems Group";
        }
    };
}
