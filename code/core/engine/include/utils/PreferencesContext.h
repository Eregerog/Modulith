/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "Core.h"
#include "Subcontext.h"

namespace modulith{
    /**
     * This context loads a table of strings at the start of the engine from "Engine.config"
     * which can contain preference settings that can be queried
     */
    class ENGINE_API PreferencesContext : public Subcontext{
    public:
        PreferencesContext() : Subcontext("Preferences Context") {}

        void OnInitialize() override;

        /**
         * Tries to get the value of a preference by key
         * @param key The key of the preference entry to get
         * @return The value string if the key existed, std::nullopt otherwise
         */
        std::optional<std::string> TryGet(const std::string& key);

    private:
        std::unordered_map<std::string, std::string> _cachedPreferences{};
    };
}

