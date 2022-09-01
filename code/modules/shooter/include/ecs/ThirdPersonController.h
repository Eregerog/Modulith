/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "ShooterModule.h"

class SHOOTER_API ThirdPersonController : public modulith::System {

public:
    ThirdPersonController() : modulith::System("Third Person Controller System") {}

    ~ThirdPersonController() override = default;

    void OnInitialize() override;

    void OnUpdate(float deltaTime) override;

    modulith::Entity GetPlayer() {return _player;}
    modulith::Entity GetPlayerModel() {return _playerModel;}
    void SetJumpVelocity(float newVelocity) {jumpVelocity = newVelocity;}

    void EnableCamera(modulith::Context& ctx);
    void DisableCamera(modulith::Context& ctx);
private:

    modulith::Entity createCrystalWeapon(modulith::Context&);

    void setActiveGun(modulith::Context&, modulith::Entity);

    modulith::float3 getCurrentCameraLocalPosition();

    // Player
    const float movementSpeedMetersPerSecond = 5.0f;
    const float forwardSpeedFactor = 1.2f;
    const float backwardsSpeedFactor = 0.6f;
    const float sidewaysSpeedFactor = 0.9f;

    const float sprintFactor = 1.5f;

    float jumpVelocity = 5.0f;

    modulith::Entity _player = modulith::Entity::Invalid();
    modulith::Entity _playerModel = modulith::Entity::Invalid();
    modulith::Entity _playerArm = modulith::Entity::Invalid();
    modulith::Entity _activeGun = modulith::Entity::Invalid();

    std::vector<modulith::Entity> _allGuns;

    bool _gunReadied = false;

    // Camera
    const float minZoom = 1.5f;
    const float maxZoom = 7.5f;
    const float cameraZoomFactor = 0.1f;

    modulith::Entity _cameraPivot = modulith::Entity::Invalid();
    modulith::Entity _cameraArm = modulith::Entity::Invalid();
    modulith::Entity _camera = modulith::Entity::Invalid();

    float _totalYawDegrees = 0;
    float _playerYawDegrees = 0;
    float _pitchDegrees = 0;

    float _zoom = 0.4f;

    bool _cameraEnabled = false;
};
