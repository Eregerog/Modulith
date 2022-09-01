/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "GameModule.h"

struct GAME_API GroundTag{};

struct GAME_API LampPostData{
    float ActivationRadius;
    float DamageRadius;
    bool WasActivated;

    modulith::Entity DamageEntity;
    modulith::Entity ActivationRangePreviewEntity;
    modulith::Entity DamageRangePreviewEntity;
};

struct GAME_API LanternData{
    float ExplodeIn;
    float Radius;
    float ExplosionDamage;
};

struct GAME_API ExplodeData{
    float Radius;
    float Damage;
};

struct GAME_API DamageNearbyEnemiesData {
    float Radius;
    float DamagePerSecond;
};