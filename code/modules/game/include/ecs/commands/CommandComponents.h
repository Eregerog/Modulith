/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "GameModule.h"

struct GAME_API MoveToData{
    modulith::float3 Destination;
    float Speed;
};

struct GAME_API LookAtData{
    modulith::float3 Destination;
};

struct GAME_API ControlledByEffectsData{
    float LuredSpeed;
    float FearedSpeed;
};

struct GAME_API LureEffectData {
    float Strength;
};

struct GAME_API FearEffectData {
    float Strength;
};