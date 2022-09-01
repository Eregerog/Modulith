/*
 * \brief
 * \author Daniel Götz
 */

#include "modules/ModuleUtils.h"
#include "modules/ModulePathUtils.h"
#include "modules/resources/ModuleResources.h"
#include "utils/YamlUtils.h"

namespace modulith {

    std::vector<std::string> ModuleUtils::ParseModlist() {

        auto res = std::vector<std::string>();

        try {
            std::error_code err;
            auto absolutePath = fs::absolute(fs::path("modules") / "Default.modlist");
            auto modlistPath = fs::canonical(absolutePath, err);
            CoreAssert(!err, "The Default.modlist at '{}' could not be found! Error: {}", absolutePath.generic_string(),
                err.message());

            auto modList = YamlUtils::TryLoadFromFile(modlistPath);

            if (modList->IsNull()) {
                CoreLogWarn("The modlist file {} is empty, no modules were loaded!", modlistPath.string())
            } else {
                CoreAssert(modList->IsSequence(), "Could not parse the modlist! Expected a YAML-style list");

                for (const auto& moduleName : modList.GetResult()) {
                    CoreLogInfo("Found module '{}'", moduleName);
                    res.push_back(moduleName.as<std::string>());
                }
            }

        }
        catch (YAML::BadFile&) {
            CoreAssert(false, "The modlist 'Default.modlist' could not be parsed, aborting!");
        }

        return res;
    }


    std::optional<ModuleInfo> ModuleUtils::LoadInfoFromFile(const fs::path& modconfigPath) {
        auto modconfig = YamlUtils::TryLoadFromFile(modconfigPath);
        auto res = YamlUtils::TryParse<ModuleInfo>(modconfig);

        if(!res)
            CoreLogWarn("The modconfig at {} could not be parsed, reason: {}", modconfigPath.string(), res.GetError().Reason())

        return res.AsOptional();
    }

    void ModuleUtils::WriteInfoToFile(const ModuleInfo& info) {
        YamlUtils::WriteToFile(info, ModulePathUtils::GetModuleConfigFile(info.Name));
    }

    std::vector<ModuleInfo> ModuleUtils::GetAllAvailableModules() {
        auto res = std::vector<ModuleInfo>();
        for (const auto& item : fs::directory_iterator(ModulePathUtils::GetModulesRootFolder())) {
            if (fs::is_directory(item.status())) {
                auto modconfigPath = ModulePathUtils::GetModuleConfigFile(item.path());
                auto isModule = fs::exists(modconfigPath);
                if (isModule) {
                    auto moduleInfo = LoadInfoFromFile(modconfigPath);
                    if (moduleInfo) {
                        res.push_back(moduleInfo.value());
                    } else {
                        CoreLogWarn("The module config under {} could not be parsed and the module can therefore not be loaded!", modconfigPath.string())
                    }
                }
            }
        }
        return res;
    }
}
