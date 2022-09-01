/**
 * \brief
 * \author Daniel Götz
 */

#include <utils/EntityHierarchy.h>
#include <serialization/SerializedEntity.h>
#include <utils/SerializedObjectDrawUtils.h>
#include <ecs/PropertiesWindowSystem.h>
#include "EntityDebugContext.h"

namespace modulith::editor {

    void EntityDebugContext::OnInitialize() {
        auto& ctx = Context::Instance();
        recalculateSerializableComponentCache();

        auto ecsContext = ctx.Get<ECSContext>();
        _selectionSystem = ecsContext->TryGetSystem<SelectionSystem>().value();

        ecsContext->TryGetSystem<PropertiesWindowSystem>().value()->RegisterPropertyDrawer<Entity>(
            [this, &ctx](const std::vector<Entity>& selectedEntities) {
                entityPropertyDrawer(ctx, selectedEntities);
            }
        );
    }

    void EntityDebugContext::OnImGui(float deltaTime, bool renderingToImguiWindow) {
        if(renderingToImguiWindow){
            ImGui::Begin("Entity View");

            auto ecs = Context::GetInstance<ECSContext>()->GetEntityManager();

            if(ImGui::Button("Create Entity")){
                ecs->CreateEntityWith(NameData("New Entity"));
            }

            ImGui::SameLine();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            std::set<Entity, SortEntitiesById> rootEntities{};
            ecs->QueryAll(
                Each(), None<WithParentData>(), [&rootEntities](auto entity) {
                    rootEntities.insert(entity);
                }
            );
            for(auto entity : rootEntities)
                EntityHierarchy().Draw(entity, HierarchyData{ecs, _selectionSystem});

            ImGui::End();
        }
    }

    void EntityDebugContext::OnAfterLoadModules(const std::vector<Module>& modules) {
        recalculateSerializableComponentCache();
    }

    void EntityDebugContext::OnBeforeUnloadModules(const std::vector<Module>& modules) {
        // Because the map may contain references to type instances that will be unloaded
        _serializableComponents.clear();
    }

    void EntityDebugContext::OnAfterUnloadModules(const std::vector<Module>& modules) {
        recalculateSerializableComponentCache();
    }

    void EntityDebugContext::recalculateSerializableComponentCache() {
        auto& ctx = Context::Instance();
        _serializableComponents.clear();
        auto componentManager = ctx.Get<ECSContext>()->GetComponentManager();
        for(const auto& pair : ctx.Get<SerializationContext>()->AllRegisteredSerializables()){
            if(componentManager->IsRegistered(pair.first) && componentManager->GetInfoOf(pair.first).IsSerializable()){
                _serializableComponents.emplace(pair.first, pair.second);
            }
        }
    }

    void EntityDebugContext::entityPropertyDrawer(Context& ctx, const std::vector<Entity>& toDraw) {
        auto ecsContext = ctx.Get<ECSContext>();
        auto ecs = ecsContext->GetEntityManager();
        auto selectedEntityCount = toDraw.size();

        auto componentManager = ecsContext->GetComponentManager();
        auto serializationCtx = ctx.Get<SerializationContext>();

        auto skip = false;
        for (auto entity : toDraw) {
            if (!entity.IsAlive(ecs)) {
                skip = true;
                _selectionSystem->RemoveFromSelection(entity);
            }
        }

        if (skip)
            return;

        if (selectedEntityCount == 1) {
            auto entity = toDraw[0];
            auto chunk = ecs->GetChunk(entity);
            ImGui::PushID(entity.GetId());

            static char entityName[64] = "";
            if(auto* nameData = entity.Get<NameData>(ecs)){
                sprintf_s(entityName, "%.63s", nameData->Value.c_str());
            }else{
                sprintf_s(entityName, "%.63s", "");
            }
            std::string initialName = entityName;

            static bool shouldBeEnabled = false;
            auto disabled = entity.Has<DisabledTag>(ecs);
            auto indirectlyDisabled = entity.Has<IndirectlyDisabledTag>(ecs);
            shouldBeEnabled = !disabled;
            auto wasEnabled = !disabled;

            ImGui::Spacing();

            ImGui::PushID("EntityIsActive");
            ImGui::Checkbox("##value", &shouldBeEnabled);
            ImGui::SameLine();
            ImGui::PopID();

            auto changedComponents = false;

            if(shouldBeEnabled != wasEnabled){
                changedComponents = true;
                entity.SetIf<DisabledTag>(ecs, !shouldBeEnabled);
            }

            ImGui::TextDisabled("Id: %i", entity.GetId());
            ImGui::SameLine();

            ImGui::PushID("EntityName");
            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##value", entityName, 64);
            ImGui::PopID();

            auto newName = std::string(entityName);
            if(newName != initialName){
                changedComponents = true;
                if(newName.empty())
                    entity.Remove<NameData>(ecs);
                else
                    entity.Add(ecs, NameData(entityName));
            }

            ImGui::TextDisabled("%s, %s", (disabled ? "Entity Disabled" : "Entity Enabled"), (indirectlyDisabled ? "Disabled in Hierarchy" : "Enabled in Hierarchy"));

            ImGui::SameLine();
            if(ImGui::Button("Destroy")){
                ecs->DestroyEntity(entity);
                changedComponents = true;
            }

            ImGui::SameLine();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            modulith::ComponentSet typesToIgnore = { typeid(NameData), typeid(DisabledTag), typeid(IndirectlyDisabledTag), typeid(WithChildrenData), typeid(WithParentData)};

            if(!changedComponents){
                for (const auto& identifier : chunk->GetIdentifier()) {
                    if(typesToIgnore.count(identifier) > 0)
                        continue;

                    auto componentInfo = componentManager->GetInfoOf(identifier);
                    auto name = componentInfo.GetFullName();

                    static bool keepComponent = true;


                    if(ImGui::CollapsingHeader(name.c_str(), &keepComponent)){
                        ImGui::PushID(componentInfo.GetIndex());
                        ImGui::Spacing();

                        if(componentInfo.IsSerializable()){
                            auto asAny = componentInfo.CopyFromPointerToAny(chunk->GetComponentPtr(entity, identifier));

                            if(auto serializable = serializationCtx->TryGetSerializableFor(asAny.type())){
                                auto serializedObject = serializable.value()->Serialize(asAny);

                                auto serializedObjectAfterDraw = SerializedObjectDrawUtils::DrawSerializedObjectAndReturnIfChanged(serializedObject);
                                if(serializedObjectAfterDraw){
                                    if(auto deserialized = serializable.value()->TryDeserialize(serializedObjectAfterDraw.value())){
                                        asAny = *deserialized;
                                        componentInfo.CopyFromAnyToPointer(asAny, chunk->GetComponentPtr(entity, identifier));
                                    }else{
                                        LogWarn("Could not deserialize back into {}", identifier.get().name())
                                    }
                                }
                            }else{
                                ImGui::TextDisabled("This component is not serializable and cannot be displayed");
                                ImGui::TextDisabled("Reason: No Serializable object is registered for it");
                            }
                        }else{
                            ImGui::TextDisabled("This component is not serializable and cannot be displayed");
                            ImGui::TextDisabled("Reason: The component type is not copy constructable");
                        }

                        ImGui::Spacing();
                        ImGui::PopID();
                    }

                    if (!keepComponent) {
                        keepComponent = true;
                        ecs->RemoveComponent(entity, identifier);
                    }
                }

                ImGui::Spacing();
                ImGui::Spacing();

                if(ImGui::Button("Add Component")){
                    ImGui::OpenPopup("AddComponentPopup");
                }

                if(ImGui::BeginPopup("AddComponentPopup")){

                    ImGuiStyle& style = ImGui::GetStyle();
                    ImGui::BeginChild(ImGui::GetID((void*) nullptr), ImVec2(220.0f, 350.0f), true, ImGuiWindowFlags_MenuBar);

                    if (ImGui::BeginMenuBar()) {
                        ImGui::TextUnformatted("Available Components");
                        ImGui::EndMenuBar();
                    }

                    for(auto& kvp : _serializableComponents){
                        if(chunk->ContainsComponent(kvp.first))
                            continue;

                        auto info = componentManager->GetInfoOf(kvp.first);
                        if(ImGui::Button(info.GetFullName().c_str())){
                            auto value = kvp.second->Construct();
                            auto ptr = ecs->AddComponent(entity, kvp.first);
                            info.CopyFromAnyToPointer(value, ptr);

                            ImGui::CloseCurrentPopup();
                        }
                    }

                    ImGui::EndChild();

                    ImGui::EndPopup();
                }

            }

            ImGui::PopID();
        }
    }

}