/*
 * \brief
 * \author Daniel Götz
 */

#include "modules/ModulePathUtils.h"

namespace modulith {

    fs::path ModulePathUtils::GetModulesRootFolder() {
        std::error_code err;
        auto res = fs::canonical(fs::current_path() / "modules", err);
        CoreAssert(!err, "Could not find the modules folder!")
        return res;
    }

    fs::path ModulePathUtils::GetModuleFolder(const std::string& moduleName) {
        return GetModulesRootFolder() / moduleName;
    }

    fs::path ModulePathUtils::GetModuleConfigFile(const fs::path& moduleFolder) {
        return moduleFolder / "Module.modconfig";
    }

    fs::path ModulePathUtils::GetModuleConfigFile(const std::string& moduleName) {
        return GetModuleConfigFile(GetModuleFolder(moduleName));
    }

    fs::path ModulePathUtils::GetHotloadableModuleDllPath(const fs::path& moduleFolder, const std::string& moduleName) {
        return moduleFolder / (moduleName + "_hotloadable.dll");
    }


}
