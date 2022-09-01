/**
 * \brief
 * \author Daniel Götz
 */

#pragma once


#include "GameModule.h"

struct GAME_API HealthData{
    float Health;
};

namespace modulith{

    /**
     * Allows serialization of the HealthData
     */
    template<>
    struct Serializer<HealthData> {
        [[nodiscard]] static HealthData Construct() { return HealthData(); }

        [[nodiscard]] static SerializedObject Serialize(const HealthData& healthData) {
            return {"Root", {
                {"Health", healthData.Health }
            }};
        }

        [[nodiscard]] static std::optional<HealthData> TryDeserialize(const SerializedObject& so) {
            auto health = so.TryGet<float>("Health");

            if (health)
                return HealthData{health.value()};

            return std::nullopt;
        }
    };
}

struct GAME_API RewardsOnDeathData{
    float ScoreGained;
    float SpiritGained;
};

struct GAME_API DamageOnContactData{
    int Damage;
};

struct GAME_API DestroyOnNoHealthTag{};