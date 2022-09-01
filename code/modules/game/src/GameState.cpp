/*
 * \brief
 * \author Daniel Götz
 */

#include <ecs/Components.h>
#include <ecs/commands/CommandComponents.h>
#include <ecs/health/HealthComponents.h>
#include <PhysicsComponents.h>
#include "GameState.h"
#include "Raycast.h"
#include "ecs/StrategyCameraController.h"
#include "RenderComponents.h"
#include "renderer/RenderContext.h"

using namespace modulith;
using namespace modulith::renderer;
using namespace modulith::physics;

void GameState::OnInitialize() {
    auto& ctx = Context::Instance();
    auto ecsCtx = ctx.Get<ECSContext>();
    auto ecs = ecsCtx->GetEntityManager();

    _currentWave = 0;
    _currentOverallState = OverallGameState::PreGame;
    _currentScore = startingScore;
    _currentEnergyResource = startingEnergy;
    _currentSpiritResource = startingSpirit;

    auto renderCtx = ctx.Get<RenderContext>();
    _enemyShader = renderCtx->Shaders()->Load(Address() / "shaders" / "PhongShader.glsl");
    _enemyMesh = ModelLoaderUtils::LoadSingleFromFile(Address() / "ghost" / "ghost.obj");

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

void GameState::OnImGui(float deltaTime, bool renderingToImguiWindow) {
    if(renderingToImguiWindow)
        return;

    if (_currentOverallState == OverallGameState::PreGame) {
        ImGui::Begin("Welcome");

        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.1f, 1.0f), "Welcome to Midnight Madness!");

        ImGui::Separator();

        ImGui::Text(
            "The ghost are trying to reach the clock tower to steal the time and usher in an age of eternal night. You must use the forces of light to stop them!"
        );

        ImGui::Separator();

        ImGui::Text(
            "Have a look at the various windows to learn about the game's mechanics. When you are ready, press the button below"
        );


        if (ImGui::Button("Start Game")) {
            _timeUntilNextWave = 10.0f;
            _currentOverallState = OverallGameState::InGameBetweenWaves;
        }

        ImGui::End();
    }

    /// ------------------------------------------------------------------------

    if(_currentOverallState == OverallGameState::PostGame){
        ImGui::Begin("Game Over");

        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "GAME OVER!");

        ImGui::Separator();

        ImGui::Text("Score: %.1f", _currentScore);


        if (ImGui::Button("Retry")) {
            auto moduleCtx = Context::GetInstance<ModuleContext>();
            auto ownModule = moduleCtx->ModuleFromName("Game");
            Assert(ownModule, "Could not find the Game Module, this should not happen! Has it been renamed?")
            auto modulesUnloadedInOrder = moduleCtx->UnloadModuleWithDependantsAtEndOfFrame(ownModule.value());
            std::reverse(modulesUnloadedInOrder.begin(), modulesUnloadedInOrder.end());
            for(auto& unloaded : modulesUnloadedInOrder)
                moduleCtx->LoadModuleAtBeginOfFrame(unloaded);
        }

        ImGui::End();
    }

    /// ------------------------------------------------------------------------

    ImGui::Begin("Game State");

    ImGui::Text("Clocktower Health: %.2f", _currentClocktowerHealth);
    helpIcon("The health of the clocktower. It is reduced when ghost are near it. When it reaches zero, you lose!");

    ImGui::Text("Score: %.1f", _currentScore);
    helpIcon("Your overall score. Is increased upon surviving a wave of enemies of when enemies are killed.");

    ImGui::Text("Current Wave: %i", _currentWave);

    if(_currentOverallState == OverallGameState::InGameBetweenWaves){
        ImGui::Text("Next Wave In: %.2f seconds", _timeUntilNextWave);
    }else if(_currentOverallState == OverallGameState::InGameInWave){
        ImGui::Text("Remaining Enemies: %i", _remainingEnemies);
    }

    ImGui::Separator();

    ImGui::Text("Resources:");
    ImGui::Text("Energy: %f", _currentEnergyResource);
    helpIcon("Regenerates naturally.");

    ImGui::Text("Spirit: %f", _currentSpiritResource);
    helpIcon("Killed enemies drop this resource.");

    ImGui::Separator();

    if(ImGui::Button("Quit Game")){
        Context::GetInstance<RenderContext>()->GetWindow()->Quit();
    }

    ImGui::End();

    /// ------------------------------------------------------------------------

    ImGui::Begin("Tooltips & Controls");

    if(_cameraTooltipOverride.has_value())
        ImGui::Text("%s", _cameraTooltipOverride.value().c_str());
    else {
        ImGui::Text("[W,A,S,D] Move Camera, [Q,E] Rotate Camera, [Mouse Wheel] Zoom");
        ImGui::Text("%s", _currentTooltip.c_str());
    }

    ImGui::End();
}

void GameState::OnUpdate(float deltaTime) {
    auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();

    if (_currentOverallState == OverallGameState::InGameInWave || _currentOverallState == OverallGameState::InGameBetweenWaves) {
        _currentEnergyResource += energyPerSecond * deltaTime;
        _currentEnergyResource = std::min(_currentEnergyResource, maximumEnergy);
    }

    if (_currentOverallState == OverallGameState::InGameBetweenWaves) {
        _timeUntilNextWave -= deltaTime;
        if(_timeUntilNextWave <= 0){
            spawnWave(ecs, ++_currentWave);
        }
    }else if(_currentOverallState == OverallGameState::InGameInWave){
        int enemyCount = 0;
        auto enemyPositions = std::vector<float3>();
        ecs->QueryActive(Each<GlobalTransformData, EnemyTag>(), [&enemyCount, &enemyPositions](auto e, auto& transform, auto& _){
            enemyPositions.push_back(transform.Position());
            enemyCount += 1;
        });

        for(auto pos : enemyPositions){
            auto distance = glm::distance(pos, float3(0,0,0));
            if(distance < enemiesDamageClocktowerBelowRange){
                _currentClocktowerHealth -= (distance  / enemiesDamageClocktowerBelowRange) * deltaTime * enemyDamageFactor;
            }
        }

        _remainingEnemies = enemyCount;

        if(_remainingEnemies <= 0){
            ModifyScore(scorePerWaveBase + scorePerWaveGrowth * _currentWave);
            _timeUntilNextWave = 10.0f;
            _currentOverallState = OverallGameState::InGameBetweenWaves;
        }
    }

    if(_currentClocktowerHealth <= 0.0f){
        _currentClocktowerHealth = 0;
        _currentOverallState = OverallGameState::PostGame;
    }
}

modulith::Entity GameState::spawnEnemy(
    const ref<modulith::EntityManager>& ecs, modulith::float3 position, modulith::float3 lookAt,
    const EnemyProperties& properties
) {
    auto height = (1.5f * properties.HeightFactor);
    auto radius = (0.5f * properties.RadiusFactor);

    auto rotation = RotationData();
    rotation.SetLookAt(lookAt - position);

    auto enemy = ecs->CreateEntityWith(
        NameData(properties.Name),
        PositionData(position + float3(0, 0, 0)),
        std::move(rotation),
        EnemyTag(),
        ControlledByEffectsData{properties.LureSpeed, properties.FearSpeed},
        CharacterControllerData(radius, height - 2 * radius),
        PhysicsContactsData(),
        HealthData{properties.MaxHealth},
        RewardsOnDeathData{ properties.ScoreReward, properties.SpiritReward },
        DestroyOnNoHealthTag()
    );


    auto enemyModel = ecs->CreateEntityWith(
        NameData("Model"),
        RenderMeshData(_enemyMesh, std::make_shared<StandardMaterial>(_enemyShader, float4(properties.Color, 1.0f), 0.6f, 32)),
        WithParentData(enemy)
    );

    return enemy;
}

void GameState::spawnWave(const ref<modulith::EntityManager>& ecs, int waveNumber) {
    static float enemyPositionVariation = 15.0f;

    const auto spawnEnemyBatch = [&ecs, this](float3 where, int count, EnemyProperties properties){
        for(int i = 0; i < count; ++i){
            auto xOffset = (rand() / RAND_MAX) * enemyPositionVariation;
            auto zOffset = (rand() / RAND_MAX) * enemyPositionVariation;
            spawnEnemy(ecs, where + float3(xOffset, 0, zOffset), float3(0,0,0), properties);
        }
    };

    auto enemySpawnPositions = std::vector<float3>();

    ecs->QueryActive(Each<GlobalTransformData, SpawnEnemiesTag>(), [&enemySpawnPositions](auto e, auto& transform, auto& _){
        enemySpawnPositions.push_back(transform.Position());
    });

    for(auto spawnPos : enemySpawnPositions){
        auto currentNormalGhostCount = (int) std::floor(normalGhostBaseCount + (normalGhostPerWaveIncrement * waveNumber));
        spawnEnemyBatch(spawnPos, currentNormalGhostCount, normalGhost);

        auto currentRunnerGhostCount = (int) std::floor(runnerGhostBaseCount + (runnerGhostPerWaveIncrement * waveNumber));
        spawnEnemyBatch(spawnPos, currentRunnerGhostCount, runnerGhost);

        auto currentBruteGhostCount = (int) std::floor(bruteGhostBaseCount + (bruteGhostPerWaveIncrement * waveNumber));
        spawnEnemyBatch(spawnPos, currentBruteGhostCount, bruteGhost);

        auto currentBossGhostCount = (int) std::floor(bossGhostBaseCount + (bossGhostPerWaveIncrement * waveNumber));
        spawnEnemyBatch(spawnPos, currentBossGhostCount, bossGhost);
    }

    _currentOverallState = OverallGameState::InGameInWave;
}
