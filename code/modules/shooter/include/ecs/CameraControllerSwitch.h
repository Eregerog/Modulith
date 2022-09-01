/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "ShooterModule.h"
#include "ThirdPersonController.h"
#include "ecs/StrategyCameraController.h"

enum SHOOTER_API CameraMode : short{
    Strategy,
    FirstPerson
};

class SHOOTER_API CameraControllerSwitch : public modulith::System {

public:

    CameraControllerSwitch() : modulith::System("Camera Controller Switch System"){}

    ~CameraControllerSwitch() override = default;

    void OnInitialize() override;

    void OnUpdate(float deltaTime) override;

private:
    CameraMode _currentMode = CameraMode::Strategy;
    modulith::shared<ThirdPersonController> _thirdPersonController {};
    modulith::shared<StrategyCameraController> _strategyController {};
};
