/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "EditorModule.h"
#include "ecs/SelectionSystem.h"
#include "utils/TreeHierarchy.h"

namespace modulith::editor {

    struct HierarchyData {
        const modulith::ref<EntityManager>& ECS;
        modulith::shared<SelectionSystem>& SelectionSystem;
    };

    struct SortEntitiesById{
        bool operator()(const Entity& lhs, const Entity& rhs) const { return lhs.GetId() < rhs.GetId(); }
    };

    class EntityHierarchy : public TreeHierarchy<modulith::Entity, HierarchyData> {

    protected:
        std::vector<Entity> expand(Entity item, HierarchyData data) override {
            if (auto* withChildren = item.Get<WithChildrenData>(data.ECS)){
                auto entities = std::set<Entity, SortEntitiesById>();
                entities.insert(withChildren->Values.begin(), withChildren->Values.end());
                return std::vector<Entity>(entities.begin(), entities.end());
            }
            return std::vector<Entity>();
        }

        void onBeforeDrawItem(Entity item, HierarchyData data) override {
            if(item.Has<IndirectlyDisabledTag>(data.ECS))
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        }

        void onAfterDrawItem(Entity item, HierarchyData data) override {
            if(item.Has<IndirectlyDisabledTag>(data.ECS))
                ImGui::PopStyleColor();
        }

        int toId(Entity item, HierarchyData data) override {
            return item.GetId();
        }

        std::string toName(Entity item, HierarchyData data) override {
            std::ostringstream ss;
            if (item.Has<NameData>(data.ECS)) {
                ss << item.Get<NameData>(data.ECS)->Value;
                ss << " (";
                ss << item.GetId();
                ss << ")";
            } else {
                ss << "Unnamed (";
                ss << item.GetId();
                ss << ")";
            }
            return ss.str();
        }

        bool isSelected(Entity item, HierarchyData data) override {
            return data.SelectionSystem->SelectionContains(item);
        }

        void onSelect(Entity item, HierarchyData data) override {
            data.SelectionSystem->HandleImGuiSelection(item);
        }

        std::string getDragDropPayloadName() override {
            return "EntityHierarchyReparent";
        }

        std::string getDragDropDescription(Entity item, HierarchyData data) override {
            return "Choose a new parent for " + toName(item, data);
        }

        void onDragDrop(Entity source, Entity target, HierarchyData data) override {
            source.AddDeferred(data.ECS, WithParentData(target));
        }

        void setImGuiPayload(Entity& item, HierarchyData data) override {
            ImGui::SetDragDropPayload(getDragDropPayloadName().c_str(), (void*) &item, sizeof(Entity));
        }

        Entity getImGuiPayloadFrom(Entity& item, HierarchyData data, void* payload) override {
            return *((Entity*) payload);
        }
    };
}