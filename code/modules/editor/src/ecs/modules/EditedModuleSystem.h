/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "EditorModule.h"

namespace modulith::editor {

    struct EditedModuleInfo {
        modulith::Module Module;
        std::filesystem::path ModuleProjectRoot;
    };

    struct ModuleProjectInfo {
        ModuleProjectInfo() = default;

        explicit ModuleProjectInfo(modulith::ModuleInfo moduleInfo) : ModuleInfo(std::move(moduleInfo)) {}

        modulith::ModuleInfo ModuleInfo;
    };
}

namespace YAML {
    template<>
    struct convert<modulith::editor::ModuleProjectInfo> {

        static Node encode(const modulith::editor::ModuleProjectInfo& info) {
            Node node;

            node["info"] = info.ModuleInfo;

            return node;
        }

        static bool decode(const Node& node, modulith::editor::ModuleProjectInfo& info) {
            info.ModuleInfo = node["info"].as<modulith::ModuleInfo>();

            return true;
        }
    };
}

namespace modulith::editor {

    class EditedModuleSystem : public modulith::System {

    public:
        EditedModuleSystem() : modulith::System("Edited Module System") {}

        void OnInitialize() override;

        void OnShutdown() override;

        void OnUpdate(float deltaTime) override;

        bool TryStartEditing(const modulith::Module& toEdit) {
            if (_moduleProjectPathCache.count(toEdit) > 0) {
                _editedModule = EditedModuleInfo{toEdit, _moduleProjectPathCache.at(toEdit)};
                return true;
            } else {
                return TrySelectAndAddProjectPathFor(toEdit);
            }
        }

        void StopEditing() { _editedModule = std::nullopt; }

        bool TrySelectAndAddProjectPathFor(const modulith::Module& module){
            std::ostringstream ss;
            ss << "Select the project folder for module";
            ss << module.Name();
            auto path = modulith::FileDialog::BrowseFolder(ss.str());
            if (path) {
                if (TryAddProjectPathFor(module, path.value())) {
                    _editedModule = EditedModuleInfo{module, path.value()};
                    return true;
                }else{
                    LogError("The selected directory is not a valid module project")
                    // TODO: error report to user
                    return false;
                }
            } else {
                return false;
            }
        }

        bool TryAddProjectPathFor(const modulith::Module& module, const std::filesystem::path& projectPath);

        void RemoveProjectPathFor(const modulith::Module& module) {
            _moduleProjectPathCache.erase(module);
        }

        [[nodiscard]] bool AnyModuleEdited() const { return _editedModule.has_value(); }

        [[nodiscard]] bool IsEdited(const modulith::Module& module) const {
            return AnyModuleEdited() && _editedModule->Module == module;
        }

        [[nodiscard]] const EditedModuleInfo& Get() { return _editedModule.value(); }

        [[nodiscard]] bool HasCachedProjectPath(const modulith::Module& module) { return _moduleProjectPathCache.count(module); }
        [[nodiscard]] std::filesystem::path GetCachedProjectPath(const modulith::Module& module) { return _moduleProjectPathCache.at(module); }

    private:
        std::optional<EditedModuleInfo> _editedModule = std::nullopt;
        std::unordered_map<modulith::Module, std::filesystem::path> _moduleProjectPathCache{};
    };
}
