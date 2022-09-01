/*
 * \brief
 * \author Daniel Götz
 */

#include "ShooterModule.h"
#include "ecs/CameraControllerSwitch.h"
#include "GameState.h"
#include "renderer/RenderContext.h"

using namespace modulith;

void CameraControllerSwitch::OnInitialize() {
    auto ecsCtx = Context::GetInstance<ECSContext>();
    _thirdPersonController = ecsCtx->TryGetSystem<ThirdPersonController>().value();
    _strategyController = ecsCtx->TryGetSystem<StrategyCameraController>().value();
}

void CameraControllerSwitch::OnUpdate(float deltaTime) {
    auto& ctx = Context::Instance();
    auto gameState = ctx.Get<GameState>();
    if(ctx.Get<RenderContext>()->Input().IsKeyPressed(KEY_TAB)){
        switch(_currentMode){
            case CameraMode::Strategy:
                _thirdPersonController->EnableCamera(ctx);
                gameState->SetCameraControlsOverride("[W,A,S,D] Move, [Space] Jump, [Shift] Sprint, [Left Mouse] Shoot, [Right Mouse] Zoom, [R] Reload, [F] Equip Weapon");
                _currentMode = CameraMode::FirstPerson;
                break;
            case CameraMode::FirstPerson:
                _thirdPersonController->DisableCamera(ctx);
                gameState->SetCameraControlsOverride(std::nullopt);
                _currentMode = CameraMode::Strategy;
                break;
            default:
                Assert(false, "CameraMode {} is not supported", _currentMode);
                break;
        }
        _strategyController->GetCamera().SetIf<DisabledTag>(ctx.Get<ECSContext>()->GetEntityManager(), _currentMode != CameraMode::Strategy);
    }
}
