/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "GameModule.h"
#include "Components.h"
#include "ecs/building/BuildingContext.h"
#include <RenderComponents.h>
#include "PhysicsComponents.h"
#include <ecs/building/BuildingComponents.h>
#include "renderer/ModelLoaderUtils.h"

class LevelSetupSystem : public modulith::System {
public:
    LevelSetupSystem() : modulith::System("LevelSetupSystem") {}

    ~LevelSetupSystem() override = default;

    void OnInitialize() override;

    modulith::Entity CreateDeadTree(modulith::float3 position, float rotation);
    modulith::Entity CreateGravestone(modulith::float3 position, modulith::float3 lookAt);
    modulith::Entity SpawnGravestoneCluster(modulith::float3 position, modulith::float3 lookAt);

private:
    std::vector<modulith::Model> _gravestoneModels;
    std::vector<modulith::Model> _deadTree;

};