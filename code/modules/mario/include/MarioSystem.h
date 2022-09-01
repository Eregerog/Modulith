/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "MarioModule.h"
#include <RenderComponents.h>
#include <renderer/ModelLoaderUtils.h>
#include "PhysicsComponents.h"
#include "ecs/health/HealthComponents.h"
#include "ecs/ThirdPersonController.h"
#include "ecs/transform/TransformComponents.h"
#include "ecs/Components.h"
#include "renderer/RenderContext.h"

using namespace modulith;
using namespace modulith::renderer;
using namespace modulith::physics;

class MarioSystem : public System {
public:

    MarioSystem() : System("Mario System") {}

    void OnInitialize() override {
        auto& ctx = Context::Instance();
        auto ecs = ctx.Get<ECSContext>()->GetEntityManager();

        auto renderCtx = ctx.Get<RenderContext>();
        renderCtx->Shaders()->Load("Standard", Address() / "shaders" / "PhongShader.glsl");

        _thirdPersonController = ctx.Get<ECSContext>()->TryGetSystem<ThirdPersonController>().value();
        auto player = _thirdPersonController->GetPlayer();
        player.Add<PhysicsContactsData>(ecs);
        _thirdPersonController->SetJumpVelocity(8.0f);

        auto bodyMat = std::make_shared<StandardMaterial>(
            renderCtx->Shaders()->Get("Standard"), renderCtx->RendererAPI()->CreateTexture2D(Address() / "mario_all.png"), nullptr, 0.0f
        );
        bodyMat->SpecularColor = float4(0.0f);

        auto eyeMat = std::make_shared<StandardMaterial>(
            renderCtx->Shaders()->Get("Standard"), renderCtx->RendererAPI()->CreateTexture2D(Address() / "mario_eye.0.png"), nullptr, 0.0f
        );
        eyeMat->SpecularColor = float4(0.0f);

        auto modelRoot = ecs->CreateEntityWith(
            WithParentData(player),
            PositionData(0, 0, 0),
            RotationData(180, float3(0, 1, 0))
        );

        auto meshes = ModelLoaderUtils::LoadMeshesFromFile(Address() / "mario.obj");

        ecs->CreateEntityWith(
            WithParentData(modelRoot),
            RenderMeshData(meshes[0], eyeMat)
        );

        ecs->CreateEntityWith(
            WithParentData(modelRoot),
            RenderMeshData(meshes[1], bodyMat)
        );

        _thirdPersonController->GetPlayerModel().Add<DisabledTag>(ecs);
    }

    void OnUpdate(float deltaTimex) override {
        auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();

        auto player = _thirdPersonController->GetPlayer();
        auto contacts = player.Get<PhysicsContactsData>(ecs);
        auto controller = player.Get<CharacterControllerData>(ecs);
        auto playerTransform = player.Get<GlobalTransformData>(ecs);

        if (controller->TouchedGround) {
            ecs->QueryActive(
                Each<EnemyTag, GlobalTransformData, HealthData>(),
                [playerPos = playerTransform->Position()](
                    auto entity, auto& enemy, auto& enemyTransform, auto& health
                ) {
                    auto enemyPos = enemyTransform.Position();
                    if (glm::distance(float3(enemyPos.x, 0, enemyPos.z), float3(playerPos.x, 0, playerPos.z)) < 0.6f) {
                        if (playerPos.y > enemyPos.y + 0.5f) {
                            health.Health -= 100;
                        }
                    }
                }
            );

        }

    }

private:
    shared<ThirdPersonController> _thirdPersonController;
};