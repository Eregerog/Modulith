/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "../../include/EditorModule.h"

namespace modulith::editor{

    template<class TItem, class TData>
    class TreeHierarchy {

    public:
        void Draw(TItem current, TData data){
            drawHierarchyRecursive(current, data);
        }

    protected:
        virtual std::vector<TItem> expand(TItem item, TData data) = 0;

        virtual int toId(TItem item, TData data) = 0;

        virtual std::string toName(TItem item, TData data) = 0;

        virtual bool isSelected(TItem item, TData data) = 0;

        virtual void onSelect(TItem item, TData) = 0;

        virtual std::string getDragDropPayloadName() = 0;

        virtual std::string getDragDropDescription(TItem item, TData data) = 0;

        virtual void onDragDrop(TItem source, TItem target, TData data) = 0;

        virtual void setImGuiPayload(TItem& item, TData data) = 0;
        virtual TItem getImGuiPayloadFrom(TItem& item, TData data, void* payload) = 0;

        virtual void onBeforeDrawItem(TItem item, TData data) {}
        virtual void onAfterDrawItem(TItem item, TData data) {}

    private:
        void drawItem(TItem current, TData data){
            auto currentName = toName(current, data);

            ImGui::SameLine();

            onBeforeDrawItem(current, data);
            if (ImGui::Selectable(currentName.c_str(), isSelected(current, data)))
                onSelect(current, data);
            addDragDropSourceAndTarget(current, data);
            onAfterDrawItem(current, data);
            ImGui::Separator();
        }

        void drawHierarchyRecursive(TItem current, TData data){
            ImGui::PushID(toId(current, data));
            auto children = expand(current, data);

            if(children.empty()){
                ImGui::PushID("Node");
                ImGui::TreeNodeEx(
                    "",
                    ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet,
                    ""
                );
                ImGui::PopID();

                drawItem(current, data);
            }
            else{

                ImGui::PushID("Node");
                auto expanded = ImGui::TreeNodeEx("", ImGuiTreeNodeFlags_OpenOnArrow, "");
                ImGui::PopID();

                drawItem(current, data);


                if (expanded) {
                    for (auto child : children)
                        drawHierarchyRecursive(child, data);

                    ImGui::TreePop();
                }

            }

            ImGui::PopID();
        }

        void addDragDropSourceAndTarget(TItem current, TData data){
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                setImGuiPayload(current, data);
                ImGui::Text("%s", getDragDropDescription(current, data).c_str());
                ImGui::EndDragDropSource();
            }
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(getDragDropPayloadName().c_str())) {
                    auto typedPayload = getImGuiPayloadFrom(current, data, payload->Data);
                    onDragDrop(typedPayload, current, data);
                }
                ImGui::EndDragDropTarget();
            }
        }

    };
}
