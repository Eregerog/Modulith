/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "CoreModule.h"
#include "ecs/systems/System.h"

namespace modulith{

    /**
     * Updates the entity hierarchy and WithChildrenData based on the WithParentData each frame.
     */
    class CORE_API ParentSystem : public System {
    public:
        explicit ParentSystem() : System("Parent System"){}
        ~ParentSystem() override = default;
        void OnUpdate(float deltaTime) override;
    };

}
