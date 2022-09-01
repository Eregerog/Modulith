/*
 * \brief
 * \author Daniel Götz
 */

#include "GameModule.h"
#include "ecs/StrategyCameraController.h"
#include "ecs/transform/TransformComponents.h"
#include "RenderComponents.h"
#include "renderer/RenderContext.h"

using namespace modulith;
using namespace modulith::renderer;

void StrategyCameraController::OnInitialize() {
    auto& ctx = Context::Instance();
    _cameraRoot = ctx.Get<ECSContext>()->GetEntityManager()->CreateEntityWith(
        NameData("Top-down Camera Root"),
        PositionData(0.0f, 0.0f, 0.0f),
        RotationData()
    );

    _cameraTop = ctx.Get<ECSContext>()->GetEntityManager()->CreateEntityWith(
        NameData("Top-down Camera"),
        PositionData(0.0f, 25.0f, 5.0f),
        RotationData(-75.0f, float3(1.0f, 0.0f, 0.0f)),
        WithParentData(_cameraRoot),
        CameraData(60.0f, ctx.Get<RenderContext>()->GetWindow()->GetSize(), 0.01f, 1000.0f)
    );
}

void StrategyCameraController::OnUpdate(float deltaTime) {
    auto& ctx = Context::Instance();
    auto ecs = ctx.Get<ECSContext>()->GetEntityManager();

    if (_cameraTop.Has<DisabledTag>(ecs))
        return;

    auto renderCtx = ctx.Get<RenderContext>();

    renderCtx->GetWindow()->ToggleCursorVisibility(true);

    auto& input = renderCtx->Input();

    if(input.IsMouseInsideWindow()) {

        // Movement
        {
            auto mousePosition = input.GetMousePosition();

            auto width = renderCtx->GetWindow()->GetWidth();
            auto height = renderCtx->GetWindow()->GetHeight();

            auto horizontalMoveZone = width * mouseMovementZoneSize;
            auto verticalMoveZone = height * mouseMovementZoneSize;

            auto mouseHorizontal = isInsideScreen(mousePosition.x, width) ? compareToInterval(
                mousePosition.x, horizontalMoveZone, width - horizontalMoveZone
            ) : 0;
            auto mouseVertical = isInsideScreen(mousePosition.y, height) ? compareToInterval(
                mousePosition.y, verticalMoveZone, height - verticalMoveZone
            ) : 0;

            auto cameraRootPosition = ecs->GetComponent<PositionData>(_cameraRoot);
            auto cameraRootTransform = ecs->GetComponent<GlobalTransformData>(_cameraRoot);

            if (!cameraRootPosition || !cameraRootTransform)
                return;

            auto cameraMovementSpeed = lerp(_zoom, minCameraMovementSpeed, maxCameraMovementSpeed);
            if (mouseHorizontal < 0 || input.IsKeyDown(KEY_A)) {
                cameraRootPosition->Value -= cameraRootTransform->Right() * deltaTime * cameraMovementSpeed;
            } else if (mouseHorizontal > 0 || input.IsKeyDown(KEY_D)) {
                cameraRootPosition->Value += cameraRootTransform->Right() * deltaTime * cameraMovementSpeed;
            }

            if (mouseVertical < 0 || input.IsKeyDown(KEY_W)) {
                cameraRootPosition->Value -= cameraRootTransform->Forward() * deltaTime * cameraMovementSpeed;

            } else if (mouseVertical > 0 || input.IsKeyDown(KEY_S)) {
                cameraRootPosition->Value += cameraRootTransform->Forward() * deltaTime * cameraMovementSpeed;
            }
        }

        // Zoom
        {
            auto scroll = input.GetScrollDelta().y;
            _zoom -= scroll * cameraZoomFactor;
            if (_zoom < 0) _zoom = 0;
            if (_zoom > 1) _zoom = 1;

            auto cameraTopPosition = ecs->GetComponent<PositionData>(_cameraTop);
            cameraTopPosition->Value = float3(0.0f, lerp(_zoom, minCameraHeight, maxCameraHeight), 0.0f);

            auto cameraTopRotation = _cameraTop.Get<RotationData>(ecs);
            auto t = 1 - std::pow(1 - _zoom, 4);
            auto angle = lerp(t, rotationAtMinHeight, rotationAtMaxHeight);
            cameraTopRotation->SetRotation(angle, float3(1,0,0));
        }

        // Rotation
        {
            if (input.IsKeyDown(KEY_Q))
                _rotationDegrees += rotationDegreesPerSecond * deltaTime;
            if (input.IsKeyDown(KEY_E))
                _rotationDegrees -= rotationDegreesPerSecond * deltaTime;

            auto cameraRootRotation = ecs->GetComponent<RotationData>(_cameraRoot);

            cameraRootRotation->SetRotation(_rotationDegrees, float3(0.0f, 1.0f, 0.0f));
        }
    }
}

bool StrategyCameraController::isInsideScreen(float value, float size) {
    return compareToInterval(value, 0, size) == 0;
}

int StrategyCameraController::compareToInterval(float value, float min, float max) {
    if (value < min) return -1;
    if (value > max) return 1;
    return 0;
}

float StrategyCameraController::lerp(float value, float min, float max) {
    return value * (max - min) + min;
}