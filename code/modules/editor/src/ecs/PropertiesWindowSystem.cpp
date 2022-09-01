/*
 * \brief
 * \author Daniel Götz
 */

#include <ecs/SelectionSystem.h>
#include "ecs/PropertiesWindowSystem.h"

namespace modulith::editor{

    void PropertiesWindowSystem::OnImGui(float deltaTime) {
        auto selectionSystem = Context::GetInstance<ECSContext>()->TryGetSystem<SelectionSystem>().value();

        ImGui::Begin("Properties");

        auto selectedType = selectionSystem->TryGetSingleType();

        if (selectionSystem->Count() > 0) {
            if (selectedType.has_value()) {

                if (selectionSystem->Count() > 1) {
                    ImGui::TextDisabled("Multiple items selected");
                    ImGui::Separator();
                }

                auto selectedTypeHash = selectedType.value().get().hash_code();
                if (_drawers.count(selectedTypeHash) > 0) {
                    _drawers[selectedTypeHash](selectionSystem->AllOf(selectedType.value()));
                } else {
                    ImGui::TextDisabled("Unsupported selection of type: %s", selectedType.value().get().name());
                }

            } else {
                ImGui::Text("Please narrow down the selection");
            }
        } else {
            ImGui::TextDisabled("Nothing selected");
        }

        ImGui::End();
    }

    void PropertiesWindowSystem::OnShutdown() {
        _drawers.clear();
    }

}
