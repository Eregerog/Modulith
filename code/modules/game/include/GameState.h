/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "GameModule.h"


enum OverallGameState {
    PreGame,
    InGameBetweenWaves,
    InGameInWave,
    PostGame
};

struct EnemyProperties{
    std::string Name;

    float HeightFactor;
    float RadiusFactor;
    modulith::float3 Color;

    float MaxHealth;
    float LureSpeed;
    float FearSpeed;

    float ScoreReward;
    float SpiritReward;
};

class GAME_API GameState : public modulith::Subcontext {

public:
    GameState() : modulith::Subcontext("Game State") {}

    void OnInitialize() override;

    void OnUpdate(float deltaTime) override;

    void OnImGui(float deltaTime, bool renderingToImguiWindow) override;

    bool IsInGame() { return _currentOverallState == OverallGameState::InGameInWave || _currentOverallState == OverallGameState::InGameBetweenWaves; }

    [[nodiscard]] float GetScore() const {
        return _currentScore;
    }

    void ModifyScore(float modifyBy) {
        _currentScore += modifyBy;
    }


    [[nodiscard]] float GetEnergyResource() const {
        return _currentEnergyResource;
    }

    void ModifyEnergyResource(float modifyBy) {
        _currentEnergyResource = std::max(std::min(_currentEnergyResource + modifyBy, maximumEnergy), 0.0f);
    }

    [[nodiscard]] float GetSpiritResource() const {
        return _currentSpiritResource;
    }

    void ModifySpiritResource(float modifyBy) {
        _currentSpiritResource = std::max(_currentSpiritResource + modifyBy, 0.0f);
    }

    void SetCurrentTooltip(std::string newValue){
        _currentTooltip = newValue;
    }

    void SetCameraControlsOverride(std::optional<std::string> value){
        _cameraTooltipOverride = value;
    }

private:

    const EnemyProperties normalGhost = EnemyProperties {
        "Normal Ghost",
        1.0f, 1.0f, modulith::float3(1.0f, 1.0f, 1.0f),
        30, 1.4f, 2.2f,
        10.0f, 3.0f
    };

    const EnemyProperties runnerGhost = EnemyProperties {
        "Runner Ghost",
        0.5f, 0.4f, modulith::float3(1.0f, 1.0f, 0.4f),
        18, 2.0f, 3.0f,
        15.0f, 5.0f
    };

    const EnemyProperties bruteGhost = EnemyProperties {
        "Brute Ghost",
        1.6f, 1.9f, modulith::float3(0.4f, 0.4f, 0.3f),
        80, 1.2f, 1.4f,
        25.0f, 8.0f
    };

    const EnemyProperties bossGhost = EnemyProperties {
        "Boss Ghost",
        5.0f, 7.0f, modulith::float3(0.8f, 0.2f, 0.1f),
        300, 0.6f, 0.6f,
        150.0f, 30.0f
    };

    const float normalGhostBaseCount = 1.0f;
    const float normalGhostPerWaveIncrement = 0.3f;

    const float runnerGhostBaseCount = 0.4f;
    const float runnerGhostPerWaveIncrement = 0.2f;

    const float bruteGhostBaseCount = 0.4f;
    const float bruteGhostPerWaveIncrement = 0.15f;

    const float bossGhostBaseCount = 0.0f;
    const float bossGhostPerWaveIncrement = 0.1f;

    void spawnWave(const modulith::ref<modulith::EntityManager>& ecs, int waveNumber);

    modulith::Entity spawnEnemy(
        const modulith::ref<modulith::EntityManager>& ecs,
        modulith::float3 position, modulith::float3 lookAt, const EnemyProperties& properties
    );

    std::optional<std::string> _cameraTooltipOverride{};
    std::string _currentTooltip = "";

    const float scorePerWaveBase = 50;
    const float scorePerWaveGrowth = 15;

    float _currentClocktowerHealth = 100;
    const float enemiesDamageClocktowerBelowRange = 10;
    const float enemyDamageFactor = 1.5f;

    int _currentWave = 0;

    int _remainingEnemies = 0;
    float _timeUntilNextWave = 0;

    float _currentScore{};
    float _currentEnergyResource{};
    float _currentSpiritResource{};

    const float startingScore = 0;
    const float startingEnergy = 100;
    const float startingSpirit = 0;

    const float energyPerSecond = 2.0f;
    const float maximumEnergy = 100;

    OverallGameState _currentOverallState{};

    modulith::shared<modulith::Shader> _enemyShader;

    modulith::shared<modulith::Mesh> _enemyMesh;
};