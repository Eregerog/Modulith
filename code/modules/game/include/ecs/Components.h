/*
 * \brief
 * \author Daniel Götz
 */

# pragma once


#include "GameModule.h"

struct GAME_API EnemyTag {};

struct GAME_API SpawnEnemiesTag{};

struct GAME_API LifetimeData{

    LifetimeData() = default;

    explicit LifetimeData(float maximum) : Maximum(maximum) {}

    float Current = 0.0f;
    float Maximum = 0.0f;
};

struct GAME_API DestroyOnCollisionTag{
    // TODO: Requires PhysicsContactsData
};