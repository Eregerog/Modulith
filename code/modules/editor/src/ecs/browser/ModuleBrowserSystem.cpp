/**
 * \brief
 * \author Daniel Götz
 */

#include <ecs/SelectionSystem.h>
#include <ecs/PropertiesWindowSystem.h>
#include <ecs/modules/EditedModuleSystem.h>

#include "ModuleBrowserSystem.h"
#include "ecs/modules/ModuleFactoryUtils.h"

namespace modulith::editor{

    void ModuleBrowserSystem::OnInitialize() {
        auto& ctx = Context::Instance();
        auto ecsCtx = ctx.Get<ECSContext>();

        ecsCtx->TryGetSystem<BrowserSystem>().value()->RegisterBrowserTab<ModuleBrowserTab>();

        ecsCtx->TryGetSystem<PropertiesWindowSystem>().value()->RegisterPropertyDrawer<Module>(
            [this, &ctx](std::vector<Module> selectedModules) {
                drawModuleProperties(std::move(selectedModules), ctx);
            }
        );
    }


    void ModuleBrowserSystem::OnShutdown() {
        auto ecsCtx = Context::GetInstance<ECSContext>();

        ecsCtx->TryGetSystem<BrowserSystem>().value()->DeregisterBrowserTab<ModuleBrowserTab>();

        ecsCtx->TryGetSystem<PropertiesWindowSystem>().value()->DeregisterPropertyDrawer<Module>();
    }

    void ModuleBrowserSystem::drawModuleProperties(std::vector<Module> selectedModules, Context& ctx) {
        auto ecsCtx = ctx.Get<ECSContext>();

        if (selectedModules.size() == 1) {
            auto module = selectedModules[0];
            if (!ctx.Get<ModuleContext>()->ModuleExists(module)) {
                ImGui::TextDisabled(
                    "The selected module %s no longer exists or is not valid!", module.Name().c_str());
                return;
            }

            auto moduleCtx = ctx.Get<ModuleContext>();
            auto editedModuleSystem = ecsCtx->TryGetSystem<EditedModuleSystem>().value();

            auto info = moduleCtx->InfoOf(module);

            ImGui::Spacing();

            if (moduleCtx->ModuleIsStillLoadedNextFrame(module)) {
                if (moduleCtx->CanUnload(module)) {
                    if (ImGui::Button("Unload")) {
                        moduleCtx->UnloadModuleAtEndOfFrame(module);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Reload")) {
                        moduleCtx->UnloadModuleAtEndOfFrame(module);
                        moduleCtx->LoadModuleAtBeginOfFrame(module);
                    }
                } else {
                    if (ImGui::Button("Unload with Dependants")) {
                        moduleCtx->UnloadModuleWithDependantsAtEndOfFrame(module);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Reload with Dependants")) {
                        auto modulesUnloadedInOrder = moduleCtx->UnloadModuleWithDependantsAtEndOfFrame(module);
                        std::reverse(modulesUnloadedInOrder.begin(), modulesUnloadedInOrder.end());
                        for(auto& unloaded : modulesUnloadedInOrder)
                            moduleCtx->LoadModuleAtBeginOfFrame(unloaded);
                    }
                }
                ImGui::SameLine();
                if(editedModuleSystem->IsEdited(module)){
                    if(ImGui::Button("Stop Editing")){
                        editedModuleSystem->StopEditing();
                    }
                }
                else{
                    if (ImGui::Button("Start Editing")) {
                        if(editedModuleSystem->AnyModuleEdited()){
                            ImGui::OpenPopup("Another module is currently active for editing");
                        }else {
                            editedModuleSystem->TryStartEditing(module);
                        }
                    }
                }

            } else {
                if (ctx.Get<ModuleContext>()->CanLoad(module)) {
                    if (ImGui::Button("Load")) {
                        ctx.Get<ModuleContext>()->LoadModuleAtBeginOfFrame(module);
                    }
                }else{
                    if (ImGui::Button("Load with Dependencies")) {
                        moduleCtx->LoadModuleWithDependenciesAtBeginOfFrame(module);
                    }
                }
            }

            if (ImGui::BeginPopupModal("Another module is currently active for editing", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Another module is currently being edited.\n"
                            "Do you want to stop editing that module to be able to edit this one?\n\n");
                ImGui::Separator();

                if (ImGui::Button("Proceed", ImVec2(120, 0))) {
                    editedModuleSystem->TryStartEditing(module);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::Spacing();

            if (ImGui::CollapsingHeader("General")) {

                ImGui::Spacing();

                ImGui::Columns(2, "moduleProperties");
                ImGui::Separator();

                ImGui::Text("Name");
                ImGui::NextColumn();
                ImGui::Text("%s", info.Name.c_str());
                ImGui::NextColumn();
                ImGui::Separator();

                ImGui::Text("Guid");
                ImGui::NextColumn();
                ImGui::Text("%s", info.Guid.str().c_str());
                ImGui::NextColumn();
                ImGui::Separator();


                ImGui::Text("Version");
                ImGui::NextColumn();
                ImGui::Text("%s", info.Version.ToString().c_str());
                ImGui::NextColumn();
                ImGui::Separator();

                ImGui::Text("Author(s)");
                ImGui::NextColumn();
                ImGui::TextWrapped("%s", info.Authors.c_str());
                ImGui::NextColumn();
                ImGui::Separator();

                ImGui::Text("Description");
                ImGui::NextColumn();
                ImGui::TextWrapped("%s", info.Description.c_str());
                ImGui::NextColumn();
                ImGui::Separator();

                ImGui::Columns(1);


                ImGui::Spacing();

            }

            if (ImGui::CollapsingHeader("Dependencies")) {

                ImGui::Spacing();


                if (info.Dependencies.empty())
                    ImGui::TextDisabled("This module has no direct dependencies");
                else {
                    ImGui::Columns(2, "dependencies");

                    ImGui::Separator();
                    ImGui::Text("Name");
                    ImGui::NextColumn();
                    ImGui::Text("Required Version");
                    ImGui::NextColumn();
                    ImGui::Separator();
                    ImGui::Separator();

                    for (auto& dependency : info.Dependencies) {
                        ImGui::Text("%s", dependency.ModuleName.c_str());
                        ImGui::NextColumn();
                        ImGui::Text("%s", dependency.Version.ToString().c_str());
                        ImGui::NextColumn();
                        ImGui::Separator();
                    }


                    ImGui::Columns(1);
                }


                ImGui::Spacing();
            }

            if(ImGui::CollapsingHeader("Project")){

                ImGui::Spacing();

                if(editedModuleSystem->HasCachedProjectPath(module)){

                    ImGui::Separator();
                    ImGui::Text("Cached Path");

                    ImGui::TextWrapped("%ls", editedModuleSystem->GetCachedProjectPath(module).c_str());

                    if(ImGui::Button("Clear Cached Path")){
                        editedModuleSystem->RemoveProjectPathFor(module);
                    }

                }else{

                    ImGui::TextDisabled("There is no cached project path");
                    if(ImGui::Button("Add Cached Path")){
                        editedModuleSystem->TrySelectAndAddProjectPathFor(module);
                    }
                }

                ImGui::Spacing();
            }
        }
    }


    std::string ModuleBrowserTab::DisplayName() {
        return "Modules";
    }

    void ModuleBrowserTab::Draw() {
        auto& ctx = Context::Instance();
        static bool invalidateModuleListNextFrame = false;

        if(ImGui::Button("Create...")){
            ImGui::OpenPopup("Module Factory");
        }

        ImGui::SameLine();
        if (ImGui::Button("Refresh Modules") || invalidateModuleListNextFrame) {
            ctx.Get<ModuleContext>()->RefreshAvailableModules();
            invalidateModuleListNextFrame = false;
        }


        ImGui::Columns(2, "moduleBrowser");
        ImGui::Separator();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Status");
        ImGui::NextColumn();
        ImGui::Separator();
        ImGui::Separator();

        auto ecsCtx = ctx.Get<ECSContext>();

        auto selectionSystem = ecsCtx->TryGetSystem<SelectionSystem>().value();
        auto editedModuleSystem = ecsCtx->TryGetSystem<EditedModuleSystem>().value();
        auto modCtx = ctx.Get<ModuleContext>();

        for (auto& module : modCtx->GetAvailableModules()) {

            auto info = modCtx->InfoOf(module);
            auto isLoaded = modCtx->ModuleIsStillLoadedNextFrame(module);

            auto shouldBeLoaded = isLoaded;

            auto cannotBeUnloaded = !modCtx->CanUnload(module);

            auto cannotBeLoaded = !modCtx->CanLoad(module);

            ImGui::PushID(info.Name.c_str());


            ImGui::Checkbox("", &shouldBeLoaded);
            ImGui::SameLine();

            ImGui::PushID("selectable");
            if (ImGui::Selectable("", selectionSystem->SelectionContains(module), ImGuiSelectableFlags_SpanAllColumns))
                selectionSystem->HandleImGuiSelection(module);

            ImGui::PopID();
            ImGui::SameLine();

            ImGui::Text("%s", info.Name.c_str());
            ImGui::NextColumn();

            if (editedModuleSystem->IsEdited(module)){
                ImGui::Text("Active for editing");
                ImGui::NextColumn();
            } else if (isLoaded && cannotBeUnloaded) {
                ImGui::Text("Loaded dependants");
                ImGui::NextColumn();
            } else if (!isLoaded && cannotBeLoaded) {
                ImGui::Text("Missing dependencies");
                ImGui::NextColumn();
            } else{
                ImGui::NextColumn();
            }

            auto moduleCtx = ctx.Get<ModuleContext>();

            static bool alwaysUnloadWithDependants = false;
            auto askToUnloadWithDependants = false;
            static bool alwaysLoadWithDependencies = false;
            auto askToLoadWithDependencies = false;

            if (isLoaded && !shouldBeLoaded) {

                if (cannotBeUnloaded) {

                    if(!alwaysUnloadWithDependants){
                        askToUnloadWithDependants = true;
                    }else{
                        moduleCtx->UnloadModuleWithDependantsAtEndOfFrame(module);
                    }

                } else {

                    moduleCtx->UnloadModuleAtEndOfFrame(module);
                    invalidateModuleListNextFrame = true;
                }

            } else if (!isLoaded && shouldBeLoaded) {
                if (cannotBeLoaded) {

                    if(!alwaysLoadWithDependencies){
                        askToLoadWithDependencies = true;
                    }else{
                        moduleCtx->LoadModuleWithDependenciesAtBeginOfFrame(module);
                    }

                } else {
                    ctx.Get<ModuleContext>()->LoadModuleAtBeginOfFrame(module);
                    invalidateModuleListNextFrame = true;
                }

            }

            ImGui::PopID();

            ImGui::Separator();

            ImGui::PushID(info.Name.c_str());

            if(askToUnloadWithDependants)
                ImGui::OpenPopup("Cannot unload module");

            if (ImGui::BeginPopupModal("Cannot unload module", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("This module has other loaded modules that depend on it.\n"
                            "It can only be unloaded if those modules are also unloaded.\n"
                            "Do you want to unload this module's dependant modules?\n\n");
                ImGui::Separator();

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::Checkbox("Always unload with dependants", &alwaysUnloadWithDependants);
                ImGui::PopStyleVar();

                if (ImGui::Button("Unload", ImVec2(120, 0))) {
                    moduleCtx->UnloadModuleWithDependantsAtEndOfFrame(module);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            if(askToLoadWithDependencies)
                ImGui::OpenPopup("Cannot load module");

            if (ImGui::BeginPopupModal("Cannot load module", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("This module has dependencies that are not loaded.\n"
                            "It can only be loaded if those modules are also loaded.\n"
                            "Do you want to load this module's dependencies?\n\n");
                ImGui::Separator();

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                ImGui::Checkbox("Always load with dependencies", &alwaysLoadWithDependencies);
                ImGui::PopStyleVar();

                if (ImGui::Button("Load", ImVec2(120, 0))) {
                    moduleCtx->LoadModuleWithDependenciesAtBeginOfFrame(module);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }

            ImGui::PopID();
        }

        if(ImGui::BeginPopupModal("Module Factory", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){

            ImGui::Spacing();

            ImGui::TextUnformatted("General");

            static std::unordered_map<Module, bool> dependencies;
            if (dependencies.size() != ctx.Get<ModuleContext>()->GetAvailableModules().size()) {
                dependencies.clear();
                for (auto& module : ctx.Get<ModuleContext>()->GetAvailableModules())
                    dependencies.insert(std::pair(module, false));
            }

            static char name[32] = "NewModule";
            static char authors[512] = "";
            static char description[1024] = "Enter a description here...";

            ImGui::InputText("Module Name", name, 32, ImGuiInputTextFlags_CharsNoBlank);
            ImGui::InputText("Authors", authors, 512);
            ImGui::InputTextMultiline("Description", description, 1024);

            auto existsAlready = modCtx->ModuleFromName(name).has_value();
            auto nameStr = std::string(name);
            auto hasName = !nameStr.empty();
            auto onlyAlphabeticCharacters = std::all_of(nameStr.begin(), nameStr.end(), isalpha);
            auto isValid = !existsAlready && hasName && onlyAlphabeticCharacters;

            if(existsAlready){
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "A module with this name already exists");
            }else if(!onlyAlphabeticCharacters){
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "System names may only consists of alphabetic letters");
            }else if(!hasName){
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "The module must have a name");
            }

            static int major = 1;
            static int minor = 0;
            static int fix = 0;

            ImGui::Spacing();

            ImGui::TextUnformatted("Version");

            ImGui::InputInt("Major", &major);
            ImGui::InputInt("Minor", &minor);
            ImGui::InputInt("Fix", &fix);

            ImGui::Spacing();

            ImGui::BeginGroup();

            ImGuiStyle& style = ImGui::GetStyle();
            float child_w = (ImGui::GetContentRegionAvail().x - 4 * style.ItemSpacing.x);
            ImGui::BeginChild(ImGui::GetID((void*) nullptr), ImVec2(child_w, 150.0f), true, ImGuiWindowFlags_MenuBar);

            if (ImGui::BeginMenuBar()) {
                ImGui::TextUnformatted("Dependencies");
                ImGui::EndMenuBar();
            }

            for (auto& kvp : dependencies) {
                auto info = ctx.Get<ModuleContext>()->InfoOf(std::get<0>(kvp));

                ImGui::Checkbox(info.Name.c_str(), &std::get<1>(kvp));
                ImGui::SameLine();
                ImGui::Text("%s", info.Version.ToString().c_str());
                ImGui::SameLine();
                ImGui::TextDisabled("(?)");
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("%s", info.Description.c_str());
            }

            ImGui::EndChild();

            ImGui::EndGroup();


            static bool triedToPlaceIntoModule = false;

            if(triedToPlaceIntoModule){
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "You cannot create the module project inside the engine's modules folder. Choose another folder!");
            }

            if (ImGui::Button("Create & Start Editing") && isValid) {

                auto moduleFolder = FileDialog::BrowseFolder("Select the folder to create the new module project in");


                if (moduleFolder) {
                    if (moduleFolder.value() == fs::current_path() / "modules") {
                        triedToPlaceIntoModule = true;
                    } else {
                        auto selectedDependencies = boolinq::from(dependencies.cbegin(), dependencies.cend())
                            .where([](const std::pair<Module, bool>& kvp) { return kvp.second; })
                            .select([](const std::pair<Module, bool>& kvp) { return kvp.first; })
                            .toStdVector();

                        auto moduleDependencies = boolinq::from(selectedDependencies)
                            .select(
                                [&ctx](auto& module) {
                                    auto info = ctx.Get<ModuleContext>()->InfoOf(module);
                                    return ModuleDependency{info.Name, info.Version};
                                }
                            )
                            .toStdVector();

                        auto info = ModuleInfo(
                            xg::newGuid(), name, description, authors, Version{major, minor, fix}, moduleDependencies
                        );
                        ModuleFactoryUtils::CreateModule(
                            info,
                            *moduleFolder,
                            boolinq::from(selectedDependencies)
                                .select([&ctx](auto module) { return ctx.Get<ModuleContext>()->InfoOf(module); })
                                .toStdVector()
                        );

                        auto moduleCtx = ctx.Get<ModuleContext>();
                        moduleCtx->RefreshAvailableModules();
                        moduleCtx->LoadModuleWithDependenciesAtBeginOfFrame(info.AsModule());

                        auto editedModuleSystem = ctx.Get<ECSContext>()->TryGetSystem<EditedModuleSystem>().value();
                        editedModuleSystem->TryAddProjectPathFor(
                            info.AsModule(), std::filesystem::path(moduleFolder.value()) / info.Name
                        );
                        auto success = editedModuleSystem->TryStartEditing(info.AsModule());
                        Assert(success, "The created module {} could not be edited, this should not happen", info.Name)

                        ImGui::CloseCurrentPopup();
                        triedToPlaceIntoModule = false;
                    }

                }
            }

            ImGui::SameLine();

            if(ImGui::Button("Cancel")){
                ImGui::CloseCurrentPopup();
                triedToPlaceIntoModule = false;
            }

            ImGui::EndPopup();
        }

        ImGui::Columns(1);
    }

}
