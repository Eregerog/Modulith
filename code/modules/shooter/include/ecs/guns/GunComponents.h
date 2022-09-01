/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "ShooterModule.h"

// TODO: Guns should be a form!

struct AmmunitionData {
    short Current;
    short Maximum;
    modulith::shared<modulith::Prefab> Ammunition;

    [[nodiscard]] bool IsNotEmpty() const { return Current > 0; }

    [[nodiscard]] bool IsFull() const { return Current >= Maximum; }
};

struct VisualizedAmmunitionData : modulith::InitializationTrait {

    VisualizedAmmunitionData(std::vector<modulith::Entity>  visualizationParents, modulith::shared<modulith::Prefab>  visualization)
        : VisualizationParents(std::move(visualizationParents)), Visualization(std::move(visualization)) {}

    std::vector<modulith::Entity> VisualizationParents;
    modulith::shared<modulith::Prefab> Visualization;

};

struct GunData{
    float Force;
    float ShootEvery;
    float RemainingShootCooldown;

    float ReloadTime;
    float RemainingReloadDuration;

    [[nodiscard]] bool CanShoot() const {return RemainingShootCooldown == 0 && RemainingReloadDuration == 0; }
};

struct GunAimData{
    modulith::Entity CameraAttachment;
};

struct ManualShootingTag{};

struct IsReloadingTag{};

struct IsAimingTag : modulith::InitializationTrait{};

struct IsShootingTag{};
