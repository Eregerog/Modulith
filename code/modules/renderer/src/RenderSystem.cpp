/*
 * \brief
 * \author Daniel Götz
 */

#include "RendererModule.h"
#include <RenderSystem.h>
#include <RenderComponents.h>
#include "renderer/RenderContext.h"

namespace modulith::renderer {

    void RenderSystem::OnInitialize() {

        auto fallbackShader = Context::GetInstance<RenderContext>()->RendererAPI()->CreateShader(Address() / "shaders" / "PhongShader.glsl");

        _fallbackMaterial = std::make_shared<StandardMaterial>(std::move(fallbackShader), float4(1.0f, 0.2f, 0.7f, 1.0f), 0, 0);

    }

    void RenderSystem::OnShutdown() { }

    void RenderSystem::OnUpdate(float deltaTime) {

        auto& ctx = Context::Instance();
        auto ecs = ctx.Get<ECSContext>()->GetEntityManager();
        auto renderCtx = ctx.Get<RenderContext>();

        auto stats = RenderStats();

        auto directionalLight = std::optional<Renderer::DirectionalLight>();
        ecs->QueryActive(
            Each<DirectionalLightData, GlobalTransformData>(),
            [&directionalLight, &stats](auto entity, DirectionalLightData& light, auto& transform) {
                directionalLight = Renderer::DirectionalLight(transform.Forward(), light.Color, light.AmbientFactor);
                stats.ActiveDirectionalLights += 1;
            }
        );

        auto pointLights = std::vector<Renderer::PointLight>();
        ecs->QueryActive(
            Each<PointLightData, GlobalTransformData>(), [&pointLights, &stats](auto entity, PointLightData& light, auto& transform) {
                pointLights.emplace_back(transform.Position(), light.Color, light.Range);
                stats.ActivePointLights += 1;
            }
        );


        ecs->QueryActive(
            Each<CameraData, GlobalTransformData>(),
            [this, ecs, &ctx, &stats, &directionalLight, &pointLights, &renderCtx](
                auto entity, CameraData& camera, GlobalTransformData& transform
            ) {
                auto renderSize = renderCtx->GetWindow()->GetSize();
                camera.SetWidthAndHeight(renderSize);

                stats.ActiveCameras += 1;

                ctx.GetProfiler().BeginMeasurement("Rendering: Begin & Init Scene");
                renderCtx->GetRenderer()->BeginScene(
                    camera.ProjectionMatrix, glm::inverse(transform.UnscaledTransform()), transform.Position(),
                    directionalLight, pointLights
                );


                ctx.GetProfiler().EndMeasurement();
                ctx.GetProfiler().BeginMeasurement("Rendering: Submit Rendered Objects");

                ecs->QueryActive(
                    Each<RenderMeshData, GlobalTransformData>(),
                    [this, cameraPosition = transform.Position(), &ctx, &renderCtx](
                        auto entity, RenderMeshData& renderMesh, GlobalTransformData& transform
                    ) {
                        auto& material = renderMesh.Material != nullptr ? renderMesh.Material : _fallbackMaterial;

                        renderCtx->GetRenderer()->SubmitDeferred(
                            material,
                            renderMesh.Mesh,
                            transform.Value
                        );
                    }
                );

                ctx.GetProfiler().EndMeasurement();
                ctx.GetProfiler().BeginMeasurement("Rendering: End Scene");

                auto sceneStats = renderCtx->GetRenderer()->EndScene();
                stats.CombinedSceneStats = stats.CombinedSceneStats.CombineWith(sceneStats);

                ctx.GetProfiler().EndMeasurement();
            }
        );

        _lastRenderStats = stats;
    }

    void RenderSystem::OnImGui(float deltaTime) {
        ImGui::Begin("Rendering Stats");

        if(_lastRenderStats.has_value()){
            ImGui::Text("Active Cameras: %i", _lastRenderStats->ActiveCameras);
            ImGui::Text("Active Directional Lights: %i", _lastRenderStats->ActiveDirectionalLights);
            ImGui::Text("Active Point Lights: %i", _lastRenderStats->ActivePointLights);
            ImGui::Spacing();
            ImGui::Text("Immediate Draw Calls: %u", _lastRenderStats->CombinedSceneStats.ImmediateSubmits);
            ImGui::Text("Deferred Draw Calls: %u", _lastRenderStats->CombinedSceneStats.DeferredSubmits);
            ImGui::Spacing();
            ImGui::Text("Material Batches: %u", _lastRenderStats->CombinedSceneStats.MaterialBatches);
            ImGui::Text("Vertex Array Batches: %u", _lastRenderStats->CombinedSceneStats.VertexArrayBatches);
            ImGui::Text("Batched Draw Calls: %u", _lastRenderStats->CombinedSceneStats.BatchedDrawCalls);
            ImGui::Spacing();
            ImGui::Text("Vertices: %llu", _lastRenderStats->CombinedSceneStats.Vertices);
            ImGui::Text("Triangles: %llu", _lastRenderStats->CombinedSceneStats.Triangles);
        }

        ImGui::End();
    }


}
