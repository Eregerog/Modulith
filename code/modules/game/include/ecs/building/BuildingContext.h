/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "GameModule.h"
#include <ecs/StrategyCameraController.h>
#include "GameState.h"

class BuildingContext : public modulith::Subcontext{
public:
   BuildingContext();

    void OnInitialize() override;

    void OnUpdate(float deltaTime) override;

    void OnImGui(float deltaTime, bool renderingToImguiWindow) override;

    std::pair<modulith::Entity, modulith::Entity> CreateLantern(modulith::float3 position, bool preview);
    std::pair<modulith::Entity, modulith::Entity> CreateLamp(modulith::float3 position, bool preview);

    void createRangeIndicator(modulith::Entity at, float radius, bool active = false);

    void switchMaterialsUnderRec(modulith::Entity root, modulith::shared<modulith::Material> newMaterial);

private:

    const float lightBeamCostPerSecond = 5.0f;
    const float lightBeamDamagePerSecond = 12.0f;
    const float lightBeamRadius = 3.5f;

    const float lanternRange = 6.5f;
    const float lanternDamage = 30.0f;
    const float lanternExplodeAfter = 0.8f;
    const float lanternEnergyCost = 20.0f;
    const float lanternSpiritCost = 10.0f;

    const float lampPostDamagePerSecond = 15.0f;
    const float lampPostRadius = 8.0f;
    const float lampPostActivationRadius = 4.0f;
    const float lampPostEnergyCost = 20;
    const float lampPostSpiritCost = 30;

    modulith::shared<GameState> _gameState;
    modulith::shared<StrategyCameraController> _strategyCameraController;

    int _equippedBuildingIndex = 0;

    modulith::Entity _heldPreview = modulith::Entity::Invalid();
    modulith::Entity _heldPreviewModel = modulith::Entity::Invalid();
    bool _previewWasValid = false;

    modulith::Entity _lightBeam;
    modulith::Entity _lightBeamActive;
    bool lightBeamWasActive = false;

    modulith::shared<modulith::Material> _previewValidMaterial;
    modulith::shared<modulith::Material> _previewInvalidMaterial;
    modulith::shared<modulith::Material> _inactiveIndicatorMaterial;
    modulith::shared<modulith::Material> _activeIndicatorMaterial;

    std::vector<modulith::Model> _cachedLampPostModels;
    std::vector<modulith::Model> _cachedLanternModels;
};
