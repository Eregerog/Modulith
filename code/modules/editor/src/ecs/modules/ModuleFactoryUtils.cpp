/*
 * \brief
 * \author Daniel Götz
 */

#include "ModuleFactoryUtils.h"
#include "EditedModuleSystem.h"

namespace modulith::editor{

    void ModuleFactoryUtils::CreateModule(
        const ModuleInfo& moduleInfo, const fs::path& createIn, const std::vector<ModuleInfo>& dependencies
    ) {
        auto dependsOnCoreModule = std::find_if(
            dependencies.begin(), dependencies.end(),
            [](const ModuleInfo& info) {return info.Name == "Core"; }
        ) != dependencies.end();

        auto name = moduleInfo.Name;
        auto moduleRoot = createModuleFolderStructure(name, createIn);
        createModuleCodeFiles(name, moduleRoot, dependsOnCoreModule);
        createModuleCMakeFile(name, moduleRoot, dependencies);

        ModuleUtils::WriteInfoToFile(moduleInfo);
        YamlUtils::WriteToFile(ModuleProjectInfo(moduleInfo), moduleRoot / "ModuleProject.projconfig");
    }


    fs::path ModuleFactoryUtils::createModuleFolderStructure(const std::string& moduleName, const fs::path& createIn) {
        auto moduleFolder = fs::absolute(createIn / moduleName);
        std::error_code err;
        fs::create_directories(moduleFolder, err);
        Assert(!err, "Could not create the module folder directory at {}, error code {}", moduleFolder.string(), err)
        fs::create_directory(moduleFolder / "include");
        fs::create_directory(moduleFolder / "src");


        return moduleFolder;
    }


    void ModuleFactoryUtils::createModuleCMakeFile(
        const std::string& moduleName, const fs::path& moduleRoot, const std::vector<ModuleInfo>& moduleDependencies
    ) {
        std::error_code err;
        auto moduleTargetBlueprint = fs::canonical(BlueprintUtils::GetEditorBlueprintDirectory() / "ModuleTargetCMakeLists.blueprint", err);
        Assert(!err, "The module cmake lists blueprint could not be found")

        auto moduleProjectBlueprint = fs::canonical(BlueprintUtils::GetEditorBlueprintDirectory() / "ModuleProjectCMakeLists.blueprint", err);
        Assert(!err, "The independent cmake lists blueprint could not be found")


        auto replacements = getDefaultReplacements(moduleName);

        std::ostringstream ss{};

        // Module Info replacements
        replacements.push_back(VariableReplacement{"ModuleVersion", "1.0"});
        replacements.push_back(VariableReplacement{"ModuleDescription", "An auto-generated module"});

        // Dependency replacements
        if (!moduleDependencies.empty()) {
            auto dependency = moduleDependencies.begin();
            ss << (*(dependency++)).Name;

            for (; dependency != moduleDependencies.end(); ++dependency)
                ss << " " << (*dependency).Name;
        }
        // Always replace the variable, even if there are no dependencies because otherwise the variable would still be in there!
        replacements.push_back(VariableReplacement{"ModuleDependencies", ss.str()});

        // Directory replacement
        replacements.push_back(VariableReplacement{"ExecutableDir", toCmakeString(fs::current_path())});
        replacements.push_back(VariableReplacement{"SourceDir", toCmakeString(moduleRoot)});

        BlueprintUtils::InstantiateBlueprintsAt(
            {moduleProjectBlueprint, moduleTargetBlueprint}, moduleRoot / "CMakeLists.txt", replacements
        );
    }


    void ModuleFactoryUtils::createModuleCodeFiles(const std::string& moduleName, const fs::path& moduleRoot, bool dependsOnCoreModule) {
        auto blueprintFolder = BlueprintUtils::GetEditorBlueprintDirectory();
        auto moduleHeaderBlueprint = blueprintFolder / "ModuleHeader.blueprint";
        auto moduleCppBlueprint = blueprintFolder / "ModuleCpp.blueprint";
        auto modulePrecompiledHeaderBlueprint = blueprintFolder / (dependsOnCoreModule
            ? "ModulePreCompiledHeader.blueprint"
            : "ModulePreCompiledHeaderNoCore.blueprint");

        std::error_code err;
        moduleHeaderBlueprint = fs::canonical(moduleHeaderBlueprint, err);
        Assert(!err, "The module header blueprint could not be found")
        moduleCppBlueprint = fs::canonical(moduleCppBlueprint, err);
        Assert(!err, "The module cpp blueprint could not be found")
        modulePrecompiledHeaderBlueprint = fs::canonical(modulePrecompiledHeaderBlueprint, err);
        Assert(!err, "The module pre compiled header blueprint could not be found")

        auto blueprintReplacements = getDefaultReplacements(moduleName);

        auto headerDestination = moduleRoot / "include" / (moduleName + "Module.h");
        BlueprintUtils::InstantiateBlueprintAt(moduleHeaderBlueprint, headerDestination, blueprintReplacements);

        auto cppDestination = moduleRoot / "src" / (moduleName + "Module.cpp");
        BlueprintUtils::InstantiateBlueprintAt(moduleCppBlueprint, cppDestination, blueprintReplacements);

        auto preCompiledHeaderDestination = moduleRoot / "include" / ("ModulithPreCompiledHeader.h");
        BlueprintUtils::InstantiateBlueprintAt(modulePrecompiledHeaderBlueprint, preCompiledHeaderDestination, blueprintReplacements);
    }


    std::vector<BlueprintVariable> ModuleFactoryUtils::getDefaultReplacements(const std::string& moduleName) {
        std::string moduleNameCaps = moduleName;
        std::transform(
            moduleName.begin(), moduleName.end(), moduleNameCaps.begin(),
            [](char c) { return toupper(c); }
        );
        std::vector<BlueprintVariable> res = {VariableReplacement{"ModuleName", moduleName},
                                              VariableReplacement{"ModuleNameCaps", moduleNameCaps}
        };
        return res;
    }

    std::string ModuleFactoryUtils::toCmakeString(const fs::path& path) {
        return "\"" + fs::canonical(path).generic_string() + "\"";
    }
}
