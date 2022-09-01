/*
 * \brief
 * \author Daniel Götz
 */

#include "utils/PreferencesContext.h"
#include "utils/YamlUtils.h"

void modulith::PreferencesContext::OnInitialize() {
    auto node = YamlUtils::TryLoadFromFile("Modulith.config");
    if(node && node.GetResult().IsMap()){
        for(auto& item : node.GetResult()){
            try{
                if(item.first.IsDefined() && item.second.IsDefined()){
                    auto key = item.first.as<std::string>();
                    auto value = item.second.as<std::string>();
                    CoreLogInfo("Read preference from Engine.config: key: {}, value: {}", key, value)
                    _cachedPreferences.emplace(key, value);
                }
            }catch(YAML::InvalidNode&){
                // ignore that entry
                continue;
            }
        }
    }
}

std::optional<std::string> modulith::PreferencesContext::TryGet(const std::string& key) {
    if(_cachedPreferences.count(key) > 0) return _cachedPreferences[key];
    return std::nullopt;
}
