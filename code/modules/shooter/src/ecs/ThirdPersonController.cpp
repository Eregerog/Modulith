/*
 * \brief
 * \author Daniel Götz
 */

#include "ShooterModule.h"
#include <RenderComponents.h>
#include <PhysicsComponents.h>
#include "ecs/ThirdPersonController.h"
#include "ecs/Components.h"
#include "ecs/commands/CommandComponents.h"
#include "ecs/guns/GunComponents.h"
#include "ecs/health/HealthComponents.h"
#include "renderer/ModelLoaderUtils.h"
#include "RenderUtils.h"
#include "renderer/RenderContext.h"

using namespace modulith;
using namespace modulith::renderer;
using namespace modulith::physics;

void ThirdPersonController::OnInitialize() {
    auto& ctx = Context::Instance();
    auto ecs = ctx.Get<ECSContext>()->GetEntityManager();
    auto renderCtx = ctx.Get<RenderContext>();

    auto width = renderCtx->GetWindow()->GetWidth();
    auto height = renderCtx->GetWindow()->GetHeight();

    _player = ecs->CreateEntityWith(
        NameData("Player"),
        PositionData(0.0f, 0.0f, 25.0f),
        RotationData(),
        CharacterControllerData(0.2f, 0.7f),
        LureEffectData { 8.0f }
    );

    _playerModel = ecs->CreateEntityWith(
        NameData("Model"),
        ScaleData(0.7f),
        WithParentData(_player)
    );

    ecs->CreateEntityWith(
        NameData("Light"),
        PositionData(0.0f, 1.0f, 0.0f),
        WithParentData(_player),
        PointLightData(float3(0.7f, 1.0f, 0.7f), 7.0f)
    );

    _playerArm = ecs->CreateEntityWith(
        NameData("Arm"),
        PositionData(0.38f, 0.85f, -0.65f),
        RotationData(),
        WithParentData(_player)
    );

    auto crystalWeapon = createCrystalWeapon(ctx);
    _allGuns.push_back(crystalWeapon);

    _activeGun = crystalWeapon;

    _activeGun.SetIf<DisabledTag>(ecs, !_gunReadied);

    RenderUtils::InstantiateModelsUnder(ecs, _playerModel, ModelLoaderUtils::LoadSceneFromFile(Address() / "spirit" / "spirit.obj"));

    _cameraPivot = ecs->CreateEntityWith(
        NameData("3rd Person Camera Pivot"),
        PositionData(0.0f, 0.52f, 0.0f),
        RotationData(),
        WithParentData(_player)
    );

    _cameraArm = ecs->CreateEntityWith(
        NameData("3rd Person Camera Arm"),
        PositionData(0.38f, 0.0f, 0.0f),
        RotationData(),
        WithParentData(_cameraPivot)
    );

    _camera = ecs->CreateEntityWith(
        NameData("3rd Person Camera"),
        PositionData(getCurrentCameraLocalPosition()),
        RotationData(),
        WithParentData(_cameraArm),
        CameraData(60.0f, modulith::int2(width, height), 0.1f, 1000.0f),
        DisabledTag()
    );
}

void ThirdPersonController::OnUpdate(float deltaTime) {
    if (!_cameraEnabled)
        return;

    auto& ctx = Context::Instance();

    auto ecs = ctx.Get<ECSContext>()->GetEntityManager();
    auto renderCtx = ctx.Get<RenderContext>();
    auto& input = renderCtx->Input();

    // Move the player root
    {
        auto characterController = ecs->GetComponent<CharacterControllerData>(_player);
        auto rootTransform = ecs->GetComponent<GlobalTransformData>(_player);

        float3 movement = {0.0f, 0.0f, 0.0f};

        auto sprint = input.IsKeyDown(KEY_LEFT_SHIFT) ? sprintFactor : 1.0f;

        if (input.IsKeyDown(KEY_W))
            movement -= rootTransform->Forward() * deltaTime * movementSpeedMetersPerSecond * forwardSpeedFactor * sprint;
        if (input.IsKeyDown(KEY_S))
            movement += rootTransform->Forward() * deltaTime * movementSpeedMetersPerSecond * backwardsSpeedFactor * sprint;

        if (input.IsKeyDown(KEY_A))
            movement -= rootTransform->Right() * deltaTime * movementSpeedMetersPerSecond * sidewaysSpeedFactor * sprint;
        if (input.IsKeyDown(KEY_D))
            movement += rootTransform->Right() * deltaTime * movementSpeedMetersPerSecond * sidewaysSpeedFactor * sprint;

        if (input.IsKeyPressed(KEY_SPACE) && characterController->TouchedGround) {
            characterController->Velocity = float3(0.0f, jumpVelocity, 0.0f);
            characterController->TouchedGround = false;
        }

        characterController->CurrentDisplacement = movement;
    }

    // Rotate the camera
    {
        auto isMoving =
            input.IsKeyDown(KEY_W) || input.IsKeyDown(KEY_S) || input.IsKeyDown(KEY_A) || input.IsKeyDown(KEY_D);

        auto mouseDelta = input.GetMouseDelta();

        auto aspect = (float) renderCtx->GetWindow()->GetWidth() / renderCtx->GetWindow()->GetHeight();

        _totalYawDegrees -= mouseDelta.x * 0.02f * aspect;
        _pitchDegrees -= mouseDelta.y * 0.02f;

        if (isMoving || _gunReadied) {
            _playerYawDegrees = _totalYawDegrees;
        }

        auto playerRotation = ecs->GetComponent<RotationData>(_player);
        playerRotation->SetRotation(_playerYawDegrees, float3(0, 1, 0));

        auto cameraPivotRotation = ecs->GetComponent<RotationData>(_cameraPivot);
        cameraPivotRotation->SetRotation(_totalYawDegrees - _playerYawDegrees, float3(0, 1, 0));

        auto cameraArmRotation = ecs->GetComponent<RotationData>(_cameraArm);
        cameraArmRotation->SetRotation(_pitchDegrees, float3(1, 0, 0));

        auto playerArmRotation = ecs->GetComponent<RotationData>(_playerArm);
        playerArmRotation->SetRotation(_pitchDegrees, float3(1, 0, 0));

        // TODO: Temporary: Once we have fine-grained update events & update control flow, this update call should be split
        // Needed because the bullets use the global transform of the gun, which needs to be updated after rotating the player
        TransformUtils::UpdateTransformOf(ecs, _player);
    }

    // Camera Zoom
    {
        auto scroll = input.GetScrollDelta().y;
        _zoom -= scroll * cameraZoomFactor;
        if (_zoom < 0) _zoom = 0;
        if (_zoom > 1) _zoom = 1;

        auto cameraPosition = ecs->GetComponent<PositionData>(_camera);
        cameraPosition->Value = getCurrentCameraLocalPosition();
    }

    // Shooting
    {

        if (_gunReadied) {
            if (input.IsMouseKeyPressed(MOUSE_BUTTON_LEFT)) {
                _activeGun.Add(ecs, IsShootingTag());
            }
            if (input.IsMouseKeyReleased(MOUSE_BUTTON_LEFT)) {
                _activeGun.Remove<IsShootingTag>(ecs);
            }

            if (input.IsKeyPressed(KEY_R)) {
                _activeGun.Add(ecs, IsReloadingTag());
            }

            if (input.IsMouseKeyPressed(MOUSE_BUTTON_RIGHT)) {
                _activeGun.Toggle<IsAimingTag>(ecs);
            }
            if (input.IsKeyPressed(KEY_F)) {
                _activeGun.Add<DisabledTag>(ecs);
                _gunReadied = false;
            }
            if (!_activeGun.Has<IsAimingTag>(ecs) && !_activeGun.Has<IsReloadingTag>(ecs)) {
                auto gunCount = _allGuns.size();
                if (gunCount > 0 && input.IsKeyPressed(KEY_1)) {
                    setActiveGun(ctx, _allGuns[0]);
                }
            }
        } else if (input.IsKeyPressed(KEY_F)) {
            _activeGun.Remove<DisabledTag>(ecs);
            _gunReadied = true;
        }

        _camera.SetIf<DisabledTag>(ecs, _activeGun.Has<InitializedTag<IsAimingTag>>(ecs));
    }
}

void ThirdPersonController::EnableCamera(Context& ctx) {
    ctx.Get<RenderContext>()->GetWindow()->ToggleCursorVisibility(false);
    ctx.Get<ECSContext>()->GetEntityManager()->RemoveComponent<DisabledTag>(_camera);
    _cameraEnabled = true;
}

void ThirdPersonController::DisableCamera(Context& ctx) {
    auto ecs = ctx.Get<ECSContext>()->GetEntityManager();

    _activeGun.Remove<IsAimingTag>(ecs);

    ctx.Get<RenderContext>()->GetWindow()->ToggleCursorVisibility(true);
    ecs->AddComponent(_camera, DisabledTag());
    _cameraEnabled = false;
}

float3 ThirdPersonController::getCurrentCameraLocalPosition() {
    return {0.0f, 0.6f, minZoom + (maxZoom - minZoom) * _zoom};
}

Entity ThirdPersonController::createCrystalWeapon(Context& ctx) {

    auto renderCtx = ctx.Get<RenderContext>();

    auto gunMaterial = std::make_shared<StandardMaterial>(
        renderCtx->RendererAPI()->CreateShader(
            Address() / "shaders" / "PhongShader.glsl"
        ),
        float4{0.22f, 0.85f, 0.0f, 1.0f},
        0.0f, 0.2f
    );

    auto ecsCtx = ctx.Get<ECSContext>();
    auto ecs = ecsCtx->GetEntityManager();

    auto crystalModel = ModelLoaderUtils::LoadSceneFromFile(Address() / "crystal" / "crystal.fbx")[0];

    auto ammo = ecsCtx->CreatePrefab(
        NameData("Ammo"),
        ScaleData(0.007f, 0.007f, 0.007f),
        BoxColliderData{float3(0.1f, 0.1f, 0.1f)},
        RigidbodyData(0.1f, float3(0, 0, 0)),
        RenderMeshData(crystalModel.Mesh, crystalModel.Material),
        LifetimeData(3.0f),
        PhysicsContactsData(),
        DestroyOnCollisionTag(),
        EnableContinuousCollisionDetectionTag(),
        DamageOnContactData{15}
    );

    auto ammoVis = ecsCtx->CreatePrefab(
        NameData("Pea Ammo Visualization"),
        ScaleData(0.0025f, 0.0025f, 0.0025f),
        RenderMeshData(crystalModel.Mesh, crystalModel.Material)
    );

    auto crystalBall = ecs->CreateEntityWith(
        NameData("Crystal Ball"),
        PositionData(0.0f, 0.1f, 0.0f),
        WithParentData(_playerArm),
        AmmunitionData{8, 8, ammo},
        GunData{1300, 0.22f, 0.0f, 1.4f, 0.0f},
        ManualShootingTag()
    );

    const float radius = 0.25f;

    auto visualizationParents = std::vector<Entity>();

    for(auto i = 0; i < 8; i++){
        auto angle = 6.28f * (i / 8.0f);
        auto vis = ecs->CreateEntityWith(
            NameData("Ammo Slot"),
            PositionData(std::sin(angle) * radius, std::cos(angle) * radius, 0.0f),
            WithParentData(crystalBall)
        );
        visualizationParents.push_back(vis);
    }

    crystalBall.Add(
        ecs,
        VisualizedAmmunitionData(
            visualizationParents,
            ammoVis
        )
    );

    crystalBall.Add<DisabledTag>(ecs);

    auto gunAimAttachment = ecs->CreateEntityWith(
        NameData("Gun Aim Camera"),
        PositionData(0.0f, 0.12f, 1.0f),
        WithParentData(crystalBall),
        CameraData(40.0f, modulith::int2(renderCtx->GetWindow()->GetWidth(), renderCtx->GetWindow()->GetHeight()), 0.1f, 1000.0f),
        DisabledTag()
    );

    crystalBall.Add(ecs, GunAimData{gunAimAttachment});

    auto modelParent = ecs->CreateEntityWith(WithParentData(crystalBall), ScaleData(0.004f));

    RenderUtils::InstantiateModelsUnder(ecs, modelParent, ModelLoaderUtils::LoadSceneFromFile(Address() / "crystal ball" / "Crystal Ball.FBX"));


    return crystalBall;
}


void ThirdPersonController::setActiveGun(Context& ctx, Entity newGun) {
    auto ecs = ctx.Get<ECSContext>()->GetEntityManager();

    _activeGun.Add<DisabledTag>(ecs);
    _activeGun = newGun;
    _activeGun.Remove<DisabledTag>(ecs);
}
