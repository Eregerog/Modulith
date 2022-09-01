/*
 * \brief
 * \author Daniel Götz
 */

#include "FileBrowserSystem.h"
#include <ecs/PropertiesWindowSystem.h>
#include "utils/TreeHierarchy.h"
#include "utils/BlueprintUtils.h"

namespace modulith::editor{

    void FileBrowserSystem::OnInitialize() {
        auto ecsContext = Context::GetInstance<ECSContext>();
        auto editingSystem = ecsContext->TryGetSystem<EditedModuleSystem>().value();
        auto selectionSystem = ecsContext->TryGetSystem<SelectionSystem>().value();

        ecsContext->TryGetSystem<BrowserSystem>().value()->RegisterBrowserTab<FileBrowserTab>(editingSystem, selectionSystem);

        auto propertySystem = ecsContext->TryGetSystem<PropertiesWindowSystem>().value();
        propertySystem->RegisterPropertyDrawer<std::filesystem::path>([](auto selected){
            if(selected.size() == 1){
                ImGui::Text("Selected file path");
            }
        });
    }

    void FileBrowserSystem::OnShutdown() {
        auto ecsContext = Context::GetInstance<ECSContext>();
        auto propertySystem = ecsContext->TryGetSystem<PropertiesWindowSystem>().value();
        propertySystem->DeregisterPropertyDrawer<std::filesystem::path>();

        ecsContext->TryGetSystem<BrowserSystem>().value()->DeregisterBrowserTab<FileBrowserTab>();
    }

    FileBrowserTab::FileBrowserTab(
        shared<EditedModuleSystem>  editingSystem, shared<SelectionSystem>  selectionSystem
    ) : _editingSystem(std::move(editingSystem)), _selectionSystem(std::move(selectionSystem)) {}


    std::string FileBrowserTab::DisplayName() {
        return "Scripts";
    }

    struct FileBrowserData {
        shared<SelectionSystem> SelectionSystem;
    };

    class FileBrowserHierarchy : public TreeHierarchy<std::filesystem::path, FileBrowserData> {

    protected:
        std::vector<std::filesystem::path> expand(std::filesystem::path item, FileBrowserData data) override {
            auto res = std::vector<std::filesystem::path>();
            auto insertFolderAtIndex = 0;
            auto insertFileAtIndex = 0;
            if(std::filesystem::is_directory(item))
                for(const auto& child : std::filesystem::directory_iterator(item)) {
                    if(std::filesystem::is_directory(child)){
                        res.insert(res.begin() + insertFolderAtIndex, child);
                        ++insertFolderAtIndex;
                        ++insertFileAtIndex;
                    }else{
                        res.insert(res.begin() + insertFileAtIndex, child);
                        ++insertFileAtIndex;
                    }
                }
            return res;
        }

        int toId(std::filesystem::path item, FileBrowserData data) override {
            return std::hash<std::string>()(item.generic_string());
        }

        std::string toName(std::filesystem::path item, FileBrowserData data) override {
            return item.filename().generic_string();
        }

        bool isSelected(std::filesystem::path item, FileBrowserData data) override {
            return data.SelectionSystem->SelectionContains(item);
        }

        void onSelect(std::filesystem::path item, FileBrowserData data) override {
            data.SelectionSystem->HandleImGuiSelection(item);
        }

        std::string getDragDropPayloadName() override {
            return "FileHierarchyReparent";
        }

        std::string getDragDropDescription(std::filesystem::path item, FileBrowserData data) override {
            std::ostringstream ss;
            ss << "Move ";
            ss << item.filename().generic_string();
            ss << " to a new location";
            return ss.str();
        }

        void onDragDrop(std::filesystem::path source, std::filesystem::path target, FileBrowserData data) override {
            if(std::filesystem::is_directory(target)){
                if(std::filesystem::is_regular_file(source)){
                    target = target / source.filename();
                }

                std::error_code err;
                std::filesystem::rename(source, target, err);
                if(err)
                LogWarn("Could not move  Error Code: {}", err.message())
            }else{
                LogWarn("Cannot move a folder or file into another file")
            }
        }

        void setImGuiPayload(std::filesystem::path& item, FileBrowserData data) override {
            auto gString = item.generic_string();
            auto cString = gString.c_str();
            ImGui::SetDragDropPayload(getDragDropPayloadName().c_str(), (void*) cString, (strlen(cString) + 1) * sizeof(char));
        }

        std::filesystem::path getImGuiPayloadFrom(std::filesystem::path& item, FileBrowserData data, void* payload) override {
            return (const char*) payload;
        }
    };

    template<class TData>
    void maybeShowPopup(
        const std::string& popupName,
        const std::string& confirmActionName,
        const std::function<TData()>& draw,
        const std::function<bool(TData currentData)>& tryConfirm
    ){
        if(ImGui::BeginPopupModal(popupName.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
            auto data = draw();

            if (ImGui::Button(confirmActionName.c_str(), ImVec2(120, 0))) {
                if(tryConfirm(data))
                    ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void FileBrowserTab::Draw() {
        if (_editingSystem->AnyModuleEdited()){

            auto selectedPath = _selectionSystem->TryGetSingleItem<std::filesystem::path>();
            if(selectedPath){
                if (ImGui::Button("Create.."))
                    ImGui::OpenPopup("FileBrowserCreatePopup");

                ImGui::SameLine();
                if(ImGui::Button("Delete")){
                    ImGui::OpenPopup("Delete File or Folder");
                }
            }else{
                ImGui::TextDisabled("Select a single folder / file to edit it");
            }


            if (ImGui::BeginPopup("FileBrowserCreatePopup"))
            {
                auto containedFolderPath = std::filesystem::is_directory(selectedPath.value()) ? selectedPath.value() : selectedPath.value().parent_path();

                static char createName[64] = "Uninitialized";

                if(ImGui::Button("Folder..")){
                    strcpy_s(createName, "New Folder");
                    ImGui::OpenPopup("Create Folder");
                }

                auto moduleCtx = Context::GetInstance<ModuleContext>();

                auto coreModule = moduleCtx->ModuleFromName("Core");
                auto editedDependsOnCore = boolinq::from(
                    moduleCtx->DependencyGraph().AllPrevsOf(_editingSystem->Get().Module)
                ).any([](auto& module) { return module.Name() == "Core"; });

                if(editedDependsOnCore){
                    if(ImGui::Button("System..")){
                        strcpy_s(createName, "NewSystem");
                        ImGui::OpenPopup("Create System");
                    }
                    if(ImGui::Button("Component..")){
                        strcpy_s(createName, "NewComponent");
                        ImGui::OpenPopup("Create Component");
                    }
                }

                { // region: All the create popups
                    maybeShowPopup<std::pair<std::filesystem::path, bool>>("Create Folder", "Create",
                        [containedFolderPath](){ // draw()

                            ImGui::Spacing();
                            ImGui::Text("Please enter a folder name");

                            ImGui::InputText("", createName, 32);

                            auto folderPath = containedFolderPath / createName;
                            auto existsAlready = std::filesystem::exists(folderPath);

                            if(existsAlready){
                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "This folder already exists");
                            }

                            ImGui::Spacing();
                            ImGui::Separator();
                            ImGui::Spacing();

                            return std::make_pair(folderPath, existsAlready);
                        },
                        [](auto pair){ // tryConfirm()
                            if(!pair.second){
                                std::filesystem::create_directory(pair.first);
                                return true;
                            }
                            return false;
                        }
                    );

                    maybeShowPopup<std::tuple<bool, bool, bool, bool, bool>>("Create System", "Create",
                        [containedFolderPath](){
                            ImGui::Spacing();

                            ImGui::Text("Please enter a system name");

                            ImGui::InputText("", createName, 32);

                            auto name = std::string(createName);
                            auto headerPath = containedFolderPath / (name + ".h");
                            auto existsAlready = std::filesystem::exists(headerPath);
                            auto onlyAlphabeticCharacters = std::all_of(name.begin(), name.end(), isalpha);
                            auto hasCharacters = !name.empty();

                            if(existsAlready){
                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "A header with this system's name already exists");
                            }else if(!onlyAlphabeticCharacters){
                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "System names may only consists of alphabetic letters");
                            }else if(!hasCharacters){
                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "The system must have a name");
                            }

                            ImGui::Spacing();
                            ImGui::Separator();
                            ImGui::Spacing();

                            static bool overrideOnInitialize = false;
                            ImGui::Checkbox("Override OnInitialize()", &overrideOnInitialize);

                            static bool overrideOnUpdate = false;
                            ImGui::Checkbox("Override OnUpdate()", &overrideOnUpdate);

                            static bool overrideOnImGui = false;
                            ImGui::Checkbox("Override OnImGui()", &overrideOnImGui);

                            static bool overrideOnShutdown = false;
                            ImGui::Checkbox("Override OnShutdown()", &overrideOnShutdown);

                            ImGui::Spacing();
                            ImGui::Separator();
                            ImGui::Spacing();

                            auto isValid = !existsAlready && onlyAlphabeticCharacters && hasCharacters;

                            return std::make_tuple(isValid, overrideOnInitialize, overrideOnUpdate, overrideOnImGui, overrideOnShutdown);
                        },
                        [containedFolderPath, editingSystem = _editingSystem](auto res){
                            if(!std::get<0>(res)) // is valid?
                                return false;

                            auto variables = std::vector<BlueprintVariable>();

                            auto moduleName = editingSystem->Get().Module.Name();
                            auto moduleNameCaps = moduleName;
                            std::transform(
                                moduleName.begin(), moduleName.end(), moduleNameCaps.begin(),
                                [](char c) { return toupper(c); }
                            );

                            variables.emplace_back(VariableReplacement{ "ModuleName", moduleName });
                            variables.emplace_back(VariableReplacement{ "ModuleNameCaps", moduleNameCaps });
                            variables.emplace_back(VariableReplacement{ "SystemName", std::string(createName) });

                            if(std::get<1>(res)) variables.emplace_back(VariableDefine{"OnInitialize"});
                            if(std::get<2>(res)) variables.emplace_back(VariableDefine{"OnUpdate"});
                            if(std::get<3>(res)) variables.emplace_back(VariableDefine{"OnImGui"});
                            if(std::get<4>(res)) variables.emplace_back(VariableDefine{"OnShutdown"});

                            BlueprintUtils::InstantiateBlueprintAt(
                                BlueprintUtils::GetEditorBlueprintDirectory() / "SystemHeader.blueprint",
                                containedFolderPath / (std::string(createName) + ".h"),
                                variables
                            );

                            BlueprintUtils::InstantiateBlueprintAt(
                                BlueprintUtils::GetEditorBlueprintDirectory() / "SystemCpp.blueprint",
                                containedFolderPath / (std::string(createName) + ".cpp"),
                                variables
                            );

                            return true;
                        }
                    );

                    maybeShowPopup<std::pair<bool, bool>>("Create Component", "Create",
                        [containedFolderPath](){
                            ImGui::Spacing();

                            ImGui::Text("Please enter a component name");

                            ImGui::InputText("", createName, 32);

                            auto name = std::string(createName);
                            auto headerPath = containedFolderPath / (name + ".h");
                            auto existsAlready = std::filesystem::exists(headerPath);
                            auto onlyAlphabeticCharacters = std::all_of(name.begin(), name.end(), isalpha);
                            auto hasCharacters = !name.empty();

                            if(existsAlready){
                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "A header with this components's name already exists");
                            }else if(!onlyAlphabeticCharacters){
                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Component names may only consists of alphabetic letters");
                            }else if(!hasCharacters){
                                ImGui::Spacing();
                                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "The component must have a name");
                            }

                            ImGui::Spacing();
                            ImGui::Separator();
                            ImGui::Spacing();

                            static bool createSerializer = false;
                            ImGui::Checkbox("Create Serializer", &createSerializer);


                            ImGui::Spacing();
                            ImGui::Separator();
                            ImGui::Spacing();

                            return std::make_pair(!existsAlready && onlyAlphabeticCharacters && hasCharacters, createSerializer);
                        },
                        [containedFolderPath, editingSystem = _editingSystem](std::pair<bool, bool> pair){
                            if(!pair.first) return false;

                            auto variables = std::vector<BlueprintVariable>();

                            auto moduleName = editingSystem->Get().Module.Name();
                            auto moduleNameCaps = moduleName;
                            std::transform(
                                moduleName.begin(), moduleName.end(), moduleNameCaps.begin(),
                                [](char c) { return toupper(c); }
                            );

                            variables.emplace_back(VariableReplacement{ "ModuleName", moduleName });
                            variables.emplace_back(VariableReplacement{ "ModuleNameCaps", moduleNameCaps });
                            variables.emplace_back(VariableReplacement{ "ComponentName", std::string(createName) });

                            if(pair.second)
                                variables.emplace_back(VariableDefine{"Serializer"});

                            BlueprintUtils::InstantiateBlueprintAt(
                                BlueprintUtils::GetEditorBlueprintDirectory() / "ComponentHeader.blueprint",
                                containedFolderPath / (std::string(createName) + ".h"),
                                variables
                            );

                            return true;
                        }
                    );
                } // endregion

                ImGui::EndPopup();
            }

            // Since void is not a valid template type, we use void* and simply ignore it (treat it as if the type was void...)
            maybeShowPopup<void*>("Delete File or Folder", "Delete",
                [selectedPath](){
                    ImGui::Spacing();
                    ImGui::Text("Are you sure you want to delete:\n%ls", selectedPath->c_str());
                    ImGui::Spacing();
                    return nullptr;
                },
                [selectedPath, selectionSystem = _selectionSystem](void*) {
                    if(std::filesystem::exists(selectedPath.value())) {
                        if(std::filesystem::is_directory(selectedPath.value()))
                            std::filesystem::remove_all(selectedPath.value());
                        else
                            std::filesystem::remove(selectedPath.value());
                        selectionSystem->ClearSelection();
                        return true;
                    }
                    return false;
                }
            );

            auto fileBrowserHierarchy = FileBrowserHierarchy();
            auto fileBrowserData = FileBrowserData{_selectionSystem};

            auto drawFolderContents = [&fileBrowserHierarchy, &fileBrowserData](const std::string& headerName, const std::filesystem::path& path){
                ImGui::SetNextItemOpen(true, ImGuiCond_Once);
                if(ImGui::CollapsingHeader(headerName.c_str())){
                    ImGui::Indent();
                    fileBrowserHierarchy.Draw(path, fileBrowserData);
                    ImGui::Unindent();
                }
            };

            drawFolderContents("Includes", _editingSystem->Get().ModuleProjectRoot / "include");
            drawFolderContents("Sources", _editingSystem->Get().ModuleProjectRoot / "src");
        }else{
            ImGui::TextDisabled("Start editing a module in order to browse its scripts");
        }
    }

}
