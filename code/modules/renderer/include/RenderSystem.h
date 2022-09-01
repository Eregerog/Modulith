/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "RendererModule.h"
#include "RenderComponents.h"
#include "renderer/Renderer.h"

namespace modulith::renderer{

    /**
     * Contains information about a rendered frame's stats
     */
    struct RenderStats {
        uint32_t ActiveCameras = 0;
        uint32_t ActiveDirectionalLights = 0;
        uint32_t ActivePointLights = 0;

        Renderer::SceneStats CombinedSceneStats{};
    };

    /**
     * This system will use the ECS components
     * CameraData, RenderMeshData, PointLightData and DirectionalLightData
     * to render a scene every frame.
     *
     * All of these components must have a GlobalTransformData attached to them
     * (either directly, or indirectly by attaching a PositionData)
     * in order to be used for rendering.
     */
    class RenderSystem : public modulith::System{
    public:
        RenderSystem() : modulith::System("RenderSystem"){
        }

        ~RenderSystem() override = default;

        void OnInitialize() override;

        void OnUpdate(float deltaTime) override;

        void OnImGui(float deltaTime) override;

        void OnShutdown() override;

    private:

        std::optional<RenderStats> _lastRenderStats;

        modulith::shared<modulith::Material> _fallbackMaterial;
    };


}
