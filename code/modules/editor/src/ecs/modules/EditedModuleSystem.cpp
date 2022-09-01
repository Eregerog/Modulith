/*
 * \brief
 * \author Daniel Götz
 */

#include "EditedModuleSystem.h"

namespace modulith::editor{

    void EditedModuleSystem::OnInitialize() {
        auto path = std::filesystem::path("modules") / "Editor" / "cache" / "ProjectPathCache.yaml";
        std::error_code err;
        auto pathAbsolute = std::filesystem::canonical(path, err);
        if (!err) {
            auto cache = YAML::LoadFile(pathAbsolute.generic_string());
            if (cache.IsSequence()) {
                for (auto entry : cache) {
                    auto moduleGuidString = entry["module"].as<std::string>("");
                    auto moduleGuid = xg::Guid(moduleGuidString);
                    if (!moduleGuid.isValid())
                        continue;
                    auto module = modulith::Module("placeholder", moduleGuid);
                    if (!Context::GetInstance<ModuleContext>()->ModuleExists(module))
                        continue;
                    // use the actual name of the module
                    module = Context::GetInstance<ModuleContext>()->InfoOf(module).AsModule();

                    auto projectPath = std::filesystem::path(entry["path"].as<std::string>(""));

                    if (!TryAddProjectPathFor(module, projectPath)) LogWarn(
                        "The cached project path for module {} is no longer valid, and has been ignored", module)
                }
            }
        }
    }

    void EditedModuleSystem::OnShutdown() {
        auto path = std::filesystem::path("modules") / "Editor" / "cache" / "ProjectPathCache.yaml";

        std::filesystem::create_directories(path.parent_path());

        YAML::Node node;

        for (auto& entry : _moduleProjectPathCache) {
            YAML::Node entryNode;
            entryNode["module"] = entry.first.Guid().str();
            entryNode["path"] = entry.second.generic_string();
            node.push_back(entryNode);
        }

        std::ofstream stream{};
        stream.open(path);
        stream << node;
        stream.flush();
        stream.close();
    }

    bool
    EditedModuleSystem::TryAddProjectPathFor(const modulith::Module& module, const std::filesystem::path& projectPath) {
        if (!std::filesystem::exists(projectPath))
            return false;
        auto projectConfigPath = projectPath / "ModuleProject.projconfig";
        if (!std::filesystem::exists(projectConfigPath))
            return false;

        auto node = YAML::LoadFile(projectConfigPath.generic_string());
        if (node.IsDefined()) {
            if (node.as<ModuleProjectInfo>().ModuleInfo.AsModule() == module) {
                _moduleProjectPathCache.emplace(module, projectPath);
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    void EditedModuleSystem::OnUpdate(float deltaTime) {
        if(_editedModule && !Context::GetInstance<ModuleContext>()->ModuleIsStillLoadedNextFrame(_editedModule.value().Module)) {
            LogWarn("The actively edited module was unloaded, editing has been stopped")
            StopEditing();
        }
    }

}
