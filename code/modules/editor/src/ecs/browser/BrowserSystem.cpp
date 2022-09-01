/*
 * \brief
 * \author Daniel Götz
 */

#include "BrowserSystem.h"
#include "ecs/SelectionSystem.h"
#include "ecs/modules/EditedModuleSystem.h"

namespace modulith::editor{

    void BrowserSystem::OnImGui(float deltaTime) {
        ImGui::Begin("Browser");

        auto flags = ImGuiTabBarFlags_None;

        if(_browsers.empty()){
            ImGui::TextDisabled("There are currently no browsers to show");
        }else{
            if (ImGui::BeginTabBar("BrowserSystemTabs", flags))
            {
                for(const auto& kvp : _browsers){
                    if (ImGui::BeginTabItem(kvp.second->DisplayName().c_str())) {
                        kvp.second->Draw();
                        ImGui::EndTabItem();
                    }
                }
                ImGui::EndTabBar();
            }
        }

        ImGui::Separator();

        ImGui::End();
    }

    void BrowserSystem::OnShutdown() {
        _browsers.clear();
    }

}
