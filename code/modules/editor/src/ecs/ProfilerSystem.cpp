/*
 * \brief
 * \author Daniel Götz
 */

#include "ecs/ECSContext.h"
#include "ecs/ProfilerSystem.h"
#include <ecs/PropertiesWindowSystem.h>
#include <utils/SerializedObjectDrawUtils.h>
#include "utils/TreeHierarchy.h"
#include "serialization/SerializationContext.h"
#include "serialization/SerializedEntity.h"
#include "utils/EntityHierarchy.h"

namespace modulith::editor{

    void ProfilerSystem::OnImGui(float deltaTime) {

        static float secondsSinceLastRefresh = 0.0f;
        const float refreshEvery = 0.1f;
        bool refreshThisFrame = false;
        if (secondsSinceLastRefresh > refreshEvery) {
            refreshThisFrame = true;
            secondsSinceLastRefresh = 0.0f;
        }

        secondsSinceLastRefresh += deltaTime;

        ImGui::Begin("Profiling");

        static float fps;
        if (refreshThisFrame)
            fps = 1.0f / deltaTime;

        ImGui::Text("%.3f FPS", fps);

        Context::Instance().GetProfiler().ForEachMeasurement(
            [](auto& measurement) {
                ImGui::Text("%.3fms - %s", measurement.Duration, measurement.Name.c_str());
            }
        );

        Context::Instance().GetProfiler().ClearMeasurements();

        ImGui::End();
    }

}
