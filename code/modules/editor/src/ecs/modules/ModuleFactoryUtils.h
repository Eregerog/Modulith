/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "EditorModule.h"
#include "utils/BlueprintUtils.h"

namespace fs = std::filesystem;

namespace modulith::editor{

    class ModuleFactoryUtils{
    public:
        static void CreateModule(const modulith::ModuleInfo& moduleInfo, const fs::path& createIn, const std::vector<modulith::ModuleInfo>& dependencies);
    private:
        static fs::path createModuleFolderStructure(const std::string& moduleName, const fs::path& createIn);

        static std::string toCmakeString(const fs::path& path);

        static void createModuleCMakeFile(const std::string& moduleName, const fs::path& moduleRoot, const std::vector<modulith::ModuleInfo>& moduleDependencies);
        static void createModuleCodeFiles(const std::string& moduleName, const fs::path& moduleRoot, bool dependsOnCoreModule);

        static std::vector<BlueprintVariable> getDefaultReplacements(const std::string& moduleName);
    };
}
