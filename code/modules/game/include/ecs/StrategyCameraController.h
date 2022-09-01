/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "GameModule.h"

class GAME_API StrategyCameraController : public modulith::System {

public:
    StrategyCameraController() : System("Strategy Camera Controller"){}

    ~StrategyCameraController() override = default;

    void OnInitialize() override;

    void OnUpdate(float deltaTime) override;

    modulith::Entity GetCamera() { return _cameraTop; }

private:
    static bool isInsideScreen(float value, float size);
    static int compareToInterval(float value, float min, float max);
    static float lerp(float value, float min, float max);

    const float mouseMovementZoneSize = 0.03f;

    const float minCameraMovementSpeed = 10.0f;
    const float maxCameraMovementSpeed = 100.0f;

    const float minCameraHeight = 1.8f;
    const float maxCameraHeight = 100.0f;

    const float rotationAtMinHeight = -20.0f;
    const float rotationAtMaxHeight = -82.0f;

    const float cameraZoomFactor = 0.03f;

    const float rotationDegreesPerSecond = 135.0f;

    float _zoom = 0.4f;
    float _rotationDegrees = 0.0f;
    modulith::Entity _cameraRoot = modulith::Entity::Invalid();
    modulith::Entity _cameraTop = modulith::Entity::Invalid();
};
