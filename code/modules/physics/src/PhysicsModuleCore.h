/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "PhysicsModule.h"

namespace modulith::physics{

    physx::PxFoundation* GetPhysXFoundation();
    physx::PxPhysics* GetPhysXPhysics();
}
