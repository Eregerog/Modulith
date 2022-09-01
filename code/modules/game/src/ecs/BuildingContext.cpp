/*
 * \brief
 * \author Daniel Götz
 */


#include "GameModule.h"
#include "GameState.h"

#include <RenderComponents.h>
#include <ecs/Components.h>
#include <ecs/commands/CommandComponents.h>
#include "ecs/building/BuildingContext.h"
#include "PhysicsComponents.h"
#include "ecs/building/BuildingComponents.h"
#include "Raycast.h"
#include "RenderUtils.h"
#include "renderer/RenderContext.h"


using namespace modulith;
using namespace modulith::renderer;
using namespace modulith::physics;

void setTooltipBuildingSelection(const shared<GameState>& gs) {
    gs->SetCurrentTooltip(
        "Controls: [1] Equip Light Beam, [2] Equip Lantern, [3] Equip Lamp Post"
    );
}

void setTooltipLightBeamSelection(const shared<GameState>& gs) {
    gs->SetCurrentTooltip(
        "Controls: [Hold Left Mouse] Use Light Beam, [Esc] Unequip Light Beam"
    );
}

void setTooltipLightBeamNoResourcesSelection(const shared<GameState>& gs) {
    gs->SetCurrentTooltip(
        "Not enough resources to use the Light Beam! - Controls: [Esc] Unequip Light Beam"
    );
}

void setTooltipLanternSelection(const shared<GameState>& gs) {
    gs->SetCurrentTooltip(
        "Controls: [Left Mouse] Place Lantern, [Esc] Unequip Lantern"
    );
}

void setTooltipLanternInvalidNoResourcesSelection(const shared<GameState>& gs) {
    gs->SetCurrentTooltip(
        "Not enough resources to place a Lantern! - Controls: [Esc] Unequip Lantern"
    );
}

void setTooltipLightPostSelection(const shared<GameState>& gs) {
    gs->SetCurrentTooltip(
        "Controls: [Left Mouse] Place Lamp Post, [Esc] Unequip Lamp Post"
    );
}

void setTooltipLightPostInvalidNotOnGroundSelection(const shared<GameState>& gs) {
    gs->SetCurrentTooltip(
        "Lamp Posts can only be placed on the ground! - Controls: [Esc] Unequip Lamp Post"
    );
}

void setTooltipLightPostInvalidTooCloseSelection(const shared<GameState>& gs) {
    gs->SetCurrentTooltip(
        "Lamp Posts cannot be too close to another! - Controls: [Esc] Unequip Lamp Post"
    );
}

void setTooltipLightPostInvalidNoResourcesSelection(const shared<GameState>& gs) {
    gs->SetCurrentTooltip(
        "Not enough resources to place a Lamp Post! - Controls: [Esc] Unequip Lamp Post"
    );
}

BuildingContext::BuildingContext() : Subcontext("Building Context") {
}

void BuildingContext::OnInitialize() {
    auto& ctx = Context::Instance();
    _gameState = ctx.Get<GameState>();

    auto ecsCtx = ctx.Get<ECSContext>();
    auto ecs = ecsCtx->GetEntityManager();
    _strategyCameraController = ecsCtx->TryGetSystem<StrategyCameraController>().value();

    _equippedBuildingIndex = 0;
    _heldPreview = Entity::Invalid();

    _lightBeam = ecs->CreateEntityWith(NameData("Light Beam"), PositionData(), DisabledTag());
    _lightBeamActive = ecs->CreateEntityWith(
        WithParentData(_lightBeam),
        DamageNearbyEnemiesData{lightBeamRadius, lightBeamDamagePerSecond},
        PositionData(),
        PointLightData(float3(1.0f, 0.8f, 0.3f), 10.0f)
    );

    auto renderCtx = ctx.Get<RenderContext>();
    auto phongShader = renderCtx->Shaders()->Load(Address() / "shaders" / "PhongShader.glsl");
    _inactiveIndicatorMaterial = std::make_shared<StandardMaterial>(
        phongShader, float4(0.7f, 0.7f, 0.7f, 1.0f), 0.0f, 32
    );
    _activeIndicatorMaterial = std::make_shared<StandardMaterial>(
        phongShader, float4(1.0f, 0.3f, 0.2f, 1.0f), 0.0f, 32
    );

    _previewValidMaterial = std::make_shared<StandardMaterial>(phongShader, float4(0, 1.0f, 0, 1), 0.0f, 32);
    _previewInvalidMaterial = std::make_shared<StandardMaterial>(phongShader, float4(1.0f, 0, 0, 1.0f), 0.0f, 32);

    createRangeIndicator(_lightBeam, lightBeamRadius);

    _cachedLampPostModels = ModelLoaderUtils::LoadSceneFromFile(Address() / "lamp post" / "lamp post.obj");
    _cachedLanternModels = ModelLoaderUtils::LoadSceneFromFile(Address() / "lantern" / "model.obj");

    setTooltipBuildingSelection(_gameState);
}

void BuildingContext::OnUpdate(float deltaTime) {
    auto& ctx = Context::Instance();

    auto ecs = ctx.Get<ECSContext>()->GetEntityManager();
    auto camera = _strategyCameraController->GetCamera();

    auto cameraWorldTransform = ecs->GetComponent<GlobalTransformData>(camera);
    auto cameraComponent = ecs->GetComponent<CameraData>(camera);

    auto resetGrabbedBuilding = [this, ecs]() {
        setTooltipBuildingSelection(_gameState);
        _equippedBuildingIndex = 0;
        if (_heldPreview.IsAlive(ecs)) {
            if (_heldPreview != _lightBeam) {
                _heldPreview.Destroy(ecs);
            } else {
                _heldPreview.Add<DisabledTag>(ecs);
            }
        }
        _heldPreviewModel = Entity::Invalid();
        _heldPreview = Entity::Invalid();
    };

    HitInfo info{};
    auto input = ctx.Get<RenderContext>()->Input();
    if (!camera.Has<DisabledTag>(ecs)) {
        auto didHit = Raycast(
            ctx, cameraComponent->ScreenPointRay(*cameraWorldTransform, input.GetMousePosition()),
            500.0f,
            info
        );
        auto validPosition = didHit && info.Entity.Has<GroundTag>(ecs);

        if (_gameState->IsInGame()) {
            if (input.IsKeyPressed(KEY_ESCAPE)) {
                resetGrabbedBuilding();
            } else if (input.IsKeyPressed(KEY_1) && _equippedBuildingIndex != 1) {
                resetGrabbedBuilding();
                _equippedBuildingIndex = 1;
                _heldPreview = _lightBeam;
            } else if (input.IsKeyPressed(KEY_2) && didHit && _equippedBuildingIndex != 2) {
                resetGrabbedBuilding();
                _equippedBuildingIndex = 2;
                auto pair = CreateLantern(info.Point, true);
                _heldPreview = pair.first;
                _heldPreviewModel = pair.second;
                _previewWasValid = false;
            } else if (input.IsKeyPressed(KEY_3) && didHit && _equippedBuildingIndex != 3) {
                resetGrabbedBuilding();
                _equippedBuildingIndex = 3;
                auto pair = CreateLamp(info.Point, true);
                _heldPreview = pair.first;
                _heldPreviewModel = pair.second;
                _previewWasValid = false;
            }

            // May be modified inside the if/else blocks below
            bool validPreviewPosition = validPosition;

            if (_equippedBuildingIndex == 1) {
                auto cost = lightBeamCostPerSecond * deltaTime;
                auto canUse = _gameState->GetEnergyResource() >= cost;
                if (canUse) {
                    setTooltipLightBeamSelection(_gameState);
                } else {
                    setTooltipLightBeamNoResourcesSelection(_gameState);
                }

                if (didHit && input.IsMouseKeyDown(MOUSE_BUTTON_LEFT) && canUse) {
                    _gameState->ModifyEnergyResource(-cost);
                    _lightBeamActive.Remove<DisabledTag>(ecs);
                    if (!lightBeamWasActive) {
                        switchMaterialsUnderRec(_lightBeam, _activeIndicatorMaterial);
                        lightBeamWasActive = true;
                    }
                } else {
                    _lightBeamActive.Add<DisabledTag>(ecs);
                    if (lightBeamWasActive) {
                        switchMaterialsUnderRec(_lightBeam, _inactiveIndicatorMaterial);
                        lightBeamWasActive = false;
                    }
                }
            } else if (_equippedBuildingIndex == 2) {
                auto hasResources = _gameState->GetEnergyResource() >= lanternEnergyCost
                                    && _gameState->GetSpiritResource() >= lanternSpiritCost;

                auto isValid = validPosition && hasResources;
                if (isValid) {
                    setTooltipLanternSelection(_gameState);
                } else if (!hasResources) {
                    validPreviewPosition = false;
                    setTooltipLanternInvalidNoResourcesSelection(_gameState);
                }

                if (isValid && input.IsMouseKeyDown(MOUSE_BUTTON_LEFT)) {
                    _gameState->ModifyEnergyResource(-lanternEnergyCost);
                    _gameState->ModifySpiritResource(-lanternSpiritCost);
                    auto position = _heldPreview.Get<PositionData>(ecs);
                    CreateLantern(position->Value, false);
                    resetGrabbedBuilding();
                }
            } else if (_equippedBuildingIndex == 3) {
                auto hasResources = _gameState->GetEnergyResource() >= lampPostEnergyCost
                                    && _gameState->GetSpiritResource() >= lampPostSpiritCost;

                auto otherLampPostPositions = std::vector<float3>();
                ecs->QueryActive(
                    Each<LampPostData, GlobalTransformData>(),
                    [&otherLampPostPositions](auto e, auto& _, auto& transform) {
                        otherLampPostPositions.push_back(transform.Position());
                    }
                );

                auto heldPreviewPosition = _heldPreview.template Get<PositionData>(ecs)->Value;
                auto anyLampPostTooClose = false;
                for (auto otherLamp : otherLampPostPositions) {
                    if (glm::distance(otherLamp, heldPreviewPosition) < lampPostRadius * 2) {
                        anyLampPostTooClose = true;
                        break;
                    }
                }

                auto isValid = validPosition && !anyLampPostTooClose && hasResources;
                if (isValid) {
                    setTooltipLightPostSelection(_gameState);
                } else if (!validPosition) {
                    setTooltipLightPostInvalidNotOnGroundSelection(_gameState);
                } else if (anyLampPostTooClose) {
                    validPreviewPosition = false;
                    setTooltipLightPostInvalidTooCloseSelection(_gameState);
                } else if (!hasResources) {
                    validPreviewPosition = false;
                    setTooltipLightPostInvalidNoResourcesSelection(_gameState);
                }

                if (isValid && input.IsMouseKeyDown(MOUSE_BUTTON_LEFT)) {
                    _gameState->ModifyEnergyResource(-lampPostEnergyCost);
                    _gameState->ModifySpiritResource(-lampPostSpiritCost);
                    auto position = _heldPreview.Get<PositionData>(ecs);
                    CreateLamp(position->Value, false);
                    resetGrabbedBuilding();
                }
            }


            if (ecs->IsAlive(_heldPreview)) {
                if (didHit) {
                    _heldPreview.Get<PositionData>(ecs)->Value = info.Point;
                    _heldPreview.Remove<DisabledTag>(ecs);
                } else {
                    _heldPreview.Add<DisabledTag>(ecs);
                }
            }
            if (ecs->IsAlive(_heldPreviewModel)) {
                if (validPreviewPosition != _previewWasValid) {
                    switchMaterialsUnderRec(
                        _heldPreviewModel, validPreviewPosition ? _previewValidMaterial : _previewInvalidMaterial
                    );
                    _previewWasValid = validPreviewPosition;
                }
            }
        }


    }


    auto enemyPositions = std::vector<float3>();

    ecs->QueryActive(
        Each<GlobalTransformData, EnemyTag>(), [&enemyPositions](auto e, auto& transform, auto& _) {
            enemyPositions.push_back(transform.Position());
        }
    );

    ecs->QueryActive(
        Each<LampPostData, GlobalTransformData>(),
        [&ecs, &ctx, &enemyPositions, this](auto e, auto& lamp, auto& transform) {
            const auto anyEnemyInRadius = [&enemyPositions, &transform](auto radius) {
                for (auto& enemyPos : enemyPositions)
                    if (glm::distance(enemyPos, transform.Position()) <= radius)
                        return true;
                return false;
            };

            if (lamp.WasActivated) {
                // keep the lamp active if there are enemies in its damage radius, otherwise shut it down
                if (!anyEnemyInRadius(lamp.DamageRadius)) {
                    switchMaterialsUnderRec(lamp.ActivationRangePreviewEntity, _inactiveIndicatorMaterial);
                    switchMaterialsUnderRec(lamp.DamageRangePreviewEntity, _inactiveIndicatorMaterial);
                    lamp.DamageEntity.template AddDeferred<DisabledTag>(ecs);
                    lamp.WasActivated = false;
                }
            } else {
                // activate the lamp if there are enemies in its activation radius, otherwise keep it off

                if (anyEnemyInRadius(lamp.ActivationRadius)) {
                    switchMaterialsUnderRec(lamp.ActivationRangePreviewEntity, _activeIndicatorMaterial);
                    switchMaterialsUnderRec(lamp.DamageRangePreviewEntity, _activeIndicatorMaterial);
                    lamp.DamageEntity.template RemoveDeferred<DisabledTag>(ecs);
                    lamp.WasActivated = true;
                }
            }
        }
    );

    ecs->QueryActive(
        Each<LanternData, GlobalTransformData>(), [ecs, deltaTime](auto e, auto& lantern, auto& transform) {
            lantern.ExplodeIn -= deltaTime;
            if (lantern.ExplodeIn <= 0) {
                ecs->Defer(
                    [position = transform.Position(), lantern, e](auto ecs) {
                        ecs->DestroyEntity(e);
                        ecs->CreateEntityWith(
                            NameData("Explosion"), ExplodeData{lantern.Radius, lantern.ExplosionDamage},
                            PositionData(position));
                    }
                );
            }
        }
    );
}

static void helpIcon(const char* desc) {
    ImGui::SameLine();
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void BuildingContext::OnImGui(float deltaTime, bool renderingToImguiWindow) {
    if (renderingToImguiWindow)
        return;

    ImGui::Begin("Spells & Buildings");

    auto width = ImGui::GetContentRegionAvailWidth();
    ImGui::Columns(4);
    ImGui::SetColumnWidth(0, width * 0.4f);
    ImGui::SetColumnWidth(1, width * 0.2f);
    ImGui::SetColumnWidth(2, width * 0.2f);
    ImGui::SetColumnWidth(3, width * 0.2f);

    ImGui::Text("Name");
    ImGui::NextColumn();
    ImGui::Text("Energy Cost");
    ImGui::NextColumn();
    ImGui::Text("Spirit Cost");
    ImGui::NextColumn();
    ImGui::Text("Damage");
    ImGui::NextColumn();

    ImGui::Separator();

    {
        ImGui::Text("[1] - Light Beam");
        helpIcon("A spell that can be cast to damage enemies in its range.");

        ImGui::NextColumn();

        ImGui::Text("%.1f/s", lightBeamCostPerSecond);
        ImGui::NextColumn();
        ImGui::NextColumn();
        ImGui::Text("%.1f/s", lightBeamDamagePerSecond);

    }

    ImGui::NextColumn();

    {

        ImGui::Text("[2] - Lantern");
        helpIcon("Place a lantern that fears enemies and explodes after a short delay.");

        ImGui::NextColumn();

        ImGui::Text("%.1f", lanternEnergyCost);
        ImGui::NextColumn();
        ImGui::Text("%.1f", lanternSpiritCost);
        ImGui::NextColumn();
        ImGui::Text("%.1f", lanternDamage);

    }

    ImGui::NextColumn();

    {

        ImGui::Text("[3] - Lamp Post");
        helpIcon("A lamp post can be placed. Once enemies get close enough to it, it will activate, fearing and damaging enemies in a larger radius.");

        ImGui::NextColumn();

        ImGui::Text("%.1f", lampPostEnergyCost);
        ImGui::NextColumn();
        ImGui::Text("%.1f", lampPostSpiritCost);
        ImGui::NextColumn();
        ImGui::Text("%.1f/s", lampPostDamagePerSecond);

    }

    ImGui::Columns();

    ImGui::End();
}


std::pair<modulith::Entity, modulith::Entity>
BuildingContext::CreateLamp(modulith::float3 position, bool preview) {
    auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();
    auto lamp = ecs->CreateEntityWith(
        NameData("Building - Lamp"),
        PositionData(position)
    );

    auto lampInnerIndicator = ecs->CreateEntityWith(NameData("Lamp Inner Range"), WithParentData(lamp));
    auto lampOuterIndicator = ecs->CreateEntityWith(NameData("Lamp Outer Range"), WithParentData(lamp));

    createRangeIndicator(lampInnerIndicator, lampPostRadius);
    createRangeIndicator(lampOuterIndicator, lampPostActivationRadius);

    if (!preview) {
        auto activeLampEntity = ecs->CreateEntityWith(
            NameData("Lamp Damage Object"),
            DisabledTag(),
            WithParentData(lamp),
            PositionData(0, 1.5f, 0),
            PointLightData(float4(1.0f, 0.7f, 0.3f, 1.0f), 12.0f),
            DamageNearbyEnemiesData{lampPostRadius, lampPostDamagePerSecond},
            FearEffectData{lampPostRadius * 2.0f}
        );

        lamp.Add(
            ecs, LampPostData{
                lampPostActivationRadius,
                lampPostRadius,
                false,
                activeLampEntity,
                lampInnerIndicator,
                lampOuterIndicator
            }
        );
    }

    auto lampModel = ecs->CreateEntityWith(NameData("Lamp Model"), WithParentData(lamp), ScaleData(1.4f));

    RenderUtils::InstantiateModelsUnder(ecs, lampModel, _cachedLampPostModels);

    if (preview) {
        switchMaterialsUnderRec(lampModel, _previewInvalidMaterial);
    }

    return std::make_pair(lamp, lampModel);
}

std::pair<modulith::Entity, modulith::Entity>
BuildingContext::CreateLantern(modulith::float3 position, bool preview) {
    auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();
    auto lantern = ecs->CreateEntityWith(
        NameData("Building - Lantern"),
        PositionData(position)
    );

    auto lanternRangeIndicator = ecs->CreateEntityWith(NameData("Lantern Range"), WithParentData(lantern));

    createRangeIndicator(lanternRangeIndicator, lanternRange, preview ? false : true);

    if (!preview) {
        ecs->AddComponents(
            lantern,
            LanternData{
                lanternExplodeAfter,
                lanternRange,
                lanternDamage
            },
            PointLightData(float4(1.0f, 0.3f, 0.2f, 1.0f), 12.0f)
        );
    }

    auto lanternModel = ecs->CreateEntityWith(NameData("Lantern Model"), WithParentData(lantern), ScaleData(2.0f));

    RenderUtils::InstantiateModelsUnder(ecs, lanternModel, _cachedLanternModels);

    if (preview) {
        switchMaterialsUnderRec(lanternModel, _previewInvalidMaterial);
    }

    return std::make_pair(lantern, lanternModel);
}


void BuildingContext::createRangeIndicator(modulith::Entity at, float radius, bool active) {
    const float defaultIndicatorEvery = 1.0f;
    const float pi = 3.14159f;
    auto d = radius * 2 * pi;

    float indicatorCount = d / defaultIndicatorEvery;

    auto& ctx = Context::Instance();

    for (int idx = 0; idx < indicatorCount; ++idx) {
        auto rad = (pi * 2) * (idx / indicatorCount);
        ctx.Get<ECSContext>()->GetEntityManager()->CreateEntityWith(
            WithParentData(at),
            PositionData(std::sin(rad) * radius, 0.05f, std::cos(rad) * radius),
            ScaleData(0.2f),
            RenderMeshData(Mesh::CreateCube(), active ? _activeIndicatorMaterial : _inactiveIndicatorMaterial)
        );
    }
}

void BuildingContext::switchMaterialsUnderRec(
    modulith::Entity root, modulith::shared<modulith::Material> newMaterial
) {
    TransformUtils::ForAllChildren(
        Context::Instance().Get<ECSContext>()->GetEntityManager(), root, [&newMaterial](auto ecs, auto entity) {
            if (auto* renderMesh = entity.template Get<RenderMeshData>(ecs)) {
                renderMesh->Material = newMaterial;
            }
        }
    );
}
