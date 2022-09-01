/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "EditorModule.h"
#include <ecs/SelectionSystem.h>

namespace modulith::editor{

    class ProfilerSystem : public modulith::System{
    public:
        ProfilerSystem() : modulith::System("Profiler System") {}

        void OnImGui(float deltaTime) override;
    };

}
