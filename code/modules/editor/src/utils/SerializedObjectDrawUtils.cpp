/**
 * \brief
 * \author Daniel Götz
 */

#include "utils/SerializedObjectDrawUtils.h"

namespace modulith::editor {

    std::optional<SerializedObject> SerializedObjectDrawUtils::DrawSerializedObjectAndReturnIfChanged(SerializedObject& obj){

        ImGui::Columns(2, "moduleBrowser");
        ImGui::Spacing();

        auto res = drawRecursiveAndReturnChanged(obj, true);

        ImGui::Columns(1);
        ImGui::Spacing();
        return res;
    }

    std::optional<SerializedObject> SerializedObjectDrawUtils::drawRecursiveAndReturnChanged(SerializedObject& obj, bool autoExpandIfComposed){
        auto name = obj.GetName();
        auto propertyBeforeDraw = obj.GetUnderlyingValue();

        ImGui::PushID(name.c_str());

        std::optional<SerializedObject> res;
        if (auto* subObjects = obj.TryGetPtr<SerializedSubObjects>()) { // Special case: Composed object

            if(!autoExpandIfComposed) ImGui::AlignTextToFramePadding();
            if (autoExpandIfComposed || ImGui::TreeNode(name.c_str())) {
                if(!autoExpandIfComposed)
                    ImGui::Spacing();
                ImGui::NextColumn();
                ImGui::NextColumn();

                std::vector<SerializedObject> newSubObjects{};
                auto anyHasChanged = false;
                for (auto& subObject : subObjects->Items()) {
                    auto subObjectPostDraw = drawRecursiveAndReturnChanged(subObject);

                    if (subObjectPostDraw.has_value()) anyHasChanged = true;
                    newSubObjects.push_back(subObjectPostDraw.value_or(subObject));
                }
                res = anyHasChanged ? std::optional(SerializedObject(name, SerializedSubObjects(newSubObjects)))
                                    : std::nullopt;

                if (!autoExpandIfComposed)
                    ImGui::TreePop();
            } else {
                ImGui::NextColumn();
                ImGui::NextColumn();
                res = std::nullopt;
            }

        } else {
            ImGui::AlignTextToFramePadding();
            ImGui::TreeNodeEx(
                "%s",
                ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet,
                name.c_str()
            );

            ImGui::NextColumn();
            ImGui::SetNextItemWidth(-1);

            if (auto* asQuat = obj.TryGetPtr<quat>()) { // Special case: Quaternion, displayed in euler angles (degrees)
                auto euler = (glm::degrees(glm::eulerAngles(*asQuat)));
                auto eulerBefore = float3(euler.x, euler.y, euler.z);
                ImGui::DragFloat3("##value", (float*) &euler);

                auto diffX = std::abs(euler.x - eulerBefore.x);
                auto diffY = std::abs(euler.y - eulerBefore.y);
                auto diffZ = std::abs(euler.z - eulerBefore.z);

                if (diffX > 0 || diffY > 0 || diffZ > 0) {
                    euler = glm::radians(euler);

                    res = SerializedObject(name, quat(euler));
                } else {
                    res = std::nullopt;
                }

            }
            else if (auto* asString = obj.TryGetPtr<std::string>()) { // Special case: std::string is not natively supported by imgui
                auto stringData = std::vector<char>(asString->begin(), asString->end());
                stringData.reserve(64);
                ImGui::InputText("##value", stringData.data(), stringData.capacity());
                auto stringAfter = stringData.data();
                res = *asString != stringAfter ? std::optional(SerializedObject(name, stringAfter)) : std::nullopt;
            }
            else if(auto* asFloat3x3 = obj.TryGetPtr<float3x3>()){
                auto transposed = glm::transpose(*asFloat3x3);

                ImGui::PushID("row1");
                ImGui::DragFloat3("##value", (float*) &transposed[0]);
                ImGui::PopID();

                ImGui::PushID("row2");
                ImGui::SetNextItemWidth(-1);
                ImGui::DragFloat3("##value", (float*) &transposed[1]);
                ImGui::PopID();

                ImGui::PushID("row3");
                ImGui::SetNextItemWidth(-1);
                ImGui::DragFloat3("##value", (float*) &transposed[2]);
                ImGui::PopID();

                res = obj.HasSameUnderlyingValueAs(propertyBeforeDraw) ? std::nullopt : std::optional(SerializedObject(name, obj.GetUnderlyingValue()));
            }
            else if(auto* asFloat4x4 = obj.TryGetPtr<float4x4>()){
                auto transposed = glm::transpose(*asFloat4x4);

                ImGui::PushID("row1");
                ImGui::DragFloat4("##value", (float*) &transposed[0]);
                ImGui::PopID();

                ImGui::PushID("row2");
                ImGui::SetNextItemWidth(-1);
                ImGui::DragFloat4("##value", (float*) &transposed[1]);
                ImGui::PopID();

                ImGui::PushID("row3");
                ImGui::SetNextItemWidth(-1);
                ImGui::DragFloat4("##value", (float*) &transposed[2]);
                ImGui::PopID();

                ImGui::PushID("row4");
                ImGui::SetNextItemWidth(-1);
                ImGui::DragFloat4("##value", (float*) &transposed[3]);
                ImGui::PopID();

                res = obj.HasSameUnderlyingValueAs(propertyBeforeDraw) ? std::nullopt : std::optional(SerializedObject(name, obj.GetUnderlyingValue()));
            }
            else { // Trivial cases
                // Int Vectors
                if (auto* asInt = obj.TryGetPtr<int>()) ImGui::DragInt("##value", asInt);
                else if (auto* asInt2 = obj.TryGetPtr<int2>()) ImGui::DragInt2("##value", (int*) asInt2);
                else if (auto* asInt3 = obj.TryGetPtr<int3>()) ImGui::DragInt3("##value", (int*) asInt3);
                else if (auto* asInt4 = obj.TryGetPtr<int4>()) ImGui::DragInt4("##value", (int*) asInt4);
                    // Float Vectors
                else if (auto* asFloat = obj.TryGetPtr<float>()) ImGui::DragFloat("##value", asFloat);
                else if (auto* asFloat2 = obj.TryGetPtr<float2>()) ImGui::DragFloat2("##value", (float*) asFloat2);
                else if (auto* asFloat3 = obj.TryGetPtr<float3>()) ImGui::DragFloat3("##value", (float*) asFloat3);
                else if (auto* asFloat4 = obj.TryGetPtr<float4>()) ImGui::DragFloat4("##value", (float*) asFloat4);
                    // Misc
                else if (auto* asBool = obj.TryGetPtr<bool>()) ImGui::Checkbox("##value", asBool);
                else if (auto* asColor3 = obj.TryGetPtr<color3>()) ImGui::ColorEdit3("##value", (float*) &asColor3->Value);
                else if (auto* asColor4 = obj.TryGetPtr<color4>()) ImGui::ColorEdit4("##value", (float*) &asColor4->Value, ImGuiColorEditFlags_AlphaPreview);
                else if (auto* asEntity = obj.TryGetPtr<Entity>())ImGui::Text("Entity (Id: %i)", asEntity->GetId());
                else ImGui::TextDisabled("The serialized variant %zu cannot be displayed", propertyBeforeDraw.index());

                res = obj.HasSameUnderlyingValueAs(propertyBeforeDraw) ? std::nullopt : std::optional(SerializedObject(name, obj.GetUnderlyingValue()));
            }
            ImGui::NextColumn();
        }


        ImGui::PopID();
        return res;

    }

}