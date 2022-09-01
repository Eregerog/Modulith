/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "CoreModule.h"
#include <ecs/ECSUtils.h>
#include "serialization/Serializer.h"

namespace modulith{

    /**
     * This component stores the parent of an entity in the scene graph
     */
    struct CORE_API WithParentData {
        WithParentData() = default;

        explicit WithParentData(Entity value) : Value(value) {}

        Entity Value{};
    };

    /**
     * Allows serialization of WithParentData
     */
    template<>
    struct Serializer<WithParentData>{

        [[nodiscard]] static WithParentData Construct() { return WithParentData(Entity::Invalid()); }

        [[nodiscard]] static SerializedObject Serialize(const WithParentData& withParentData) {
            return { "Root", {{ "Value", withParentData.Value }} };
        }

        [[nodiscard]] static std::optional<WithParentData> TryDeserialize(const SerializedObject& so) {
            if(auto value = so.TryGet("Value")){
                if(auto position = value->TryGet<Entity>()){
                    return WithParentData(position.value());
                }
            }
            return std::nullopt;
        }
    };

    /**
     * This component stores the children of an entity in the scene graph
     * These are re-calculated every frame by the TransformSystem.
     * To change the children on an entity, the parents of the to-be children need to be changed instead.
     * @see WithParentData
     */
    struct CORE_API WithChildrenData {
        std::vector<Entity> Values;
    };


    /**
     * Stores the local position of an entity in the world space
     */
    struct CORE_API PositionData {

        PositionData() = default;

        explicit PositionData(const float3& value) : Value(value) {}

        PositionData(float x, float y, float z) : Value(float3{x, y, z}) {}

        float3 Value = float3(0.0f);
    };


    /**
     * Allows serialization of PositionData
     */
    template<>
    struct Serializer<PositionData>{
        [[nodiscard]] static PositionData Construct() { return PositionData(); }

        [[nodiscard]] static SerializedObject Serialize(const PositionData& nameData) {
            return { "Root", {{ "Value", nameData.Value }} };
        }

        [[nodiscard]] static std::optional<PositionData> TryDeserialize(const SerializedObject& so) {
            if(auto value = so.TryGet("Value")){
                if(auto position = value->TryGet<float3>()){
                    return PositionData(position.value());
                }
            }
            return std::nullopt;
        }
    };

    /**
     * Stores the local rotation of an entity in the world space
     */
    struct CORE_API RotationData {

        RotationData() = default;

        explicit RotationData(float3 eulerAngles) : Value(quat(eulerAngles)) {}

        explicit RotationData(const float angleDegrees, const float3 axis) : Value(
            glm::rotate(glm::identity<glm::quat>(), glm::radians(angleDegrees), axis)) {}

        explicit RotationData(const glm::quat& value) : Value(value) {}

        glm::quat Value = glm::quat();

        inline void SetRotation(const float angleDegrees, const float3 axis) {
            Value = glm::rotate(glm::identity<glm::quat>(), glm::radians(angleDegrees), axis);
        }

        inline void SetLookAt(const float3 direction){
            Value = glm::quatLookAt(glm::normalize(direction), float3(0,1,0));
        }

        [[nodiscard]] inline float3 Right() const { return glm::normalize(glm::mat4_cast(Value)[0]); }

        [[nodiscard]] inline float3 Up() const { return glm::normalize(glm::mat4_cast(Value)[1]); }

        [[nodiscard]] inline float3 Forward() const { return glm::normalize(glm::mat4_cast(Value)[2]); }
    };


    /**
     * Allows serialization of RotationData
     */
    template<>
    struct Serializer<RotationData>{

        [[nodiscard]] static RotationData Construct() { return RotationData(); }

        [[nodiscard]] static SerializedObject Serialize(const RotationData& nameData) {
            return { "Root", {{ "Value", nameData.Value }} };
        }

        [[nodiscard]] static std::optional<RotationData> TryDeserialize(const SerializedObject& so) {
            if(auto value = so.TryGet("Value")){
                if(auto quaternion = value->TryGet<quat>()){
                    return RotationData(quaternion.value());
                }
            }
            return std::nullopt;
        }
    };

    /**
     * Stores the local scale of an entity in the world space
     */
    struct CORE_API ScaleData {

        ScaleData() = default;

        explicit ScaleData(float uniformValue) : Value(float3(uniformValue)) {}

        explicit ScaleData(const float3& value) : Value(value) {}

        ScaleData(float x, float y, float z) : Value(float3{x, y, z}) {}

        float3 Value = float3(1.0f);
    };


    /**
     * Allows serialization of ScaleData
     */
    template<>
    struct Serializer<ScaleData>{

        [[nodiscard]] static ScaleData Construct() { return ScaleData(); }

        [[nodiscard]] static SerializedObject Serialize(const ScaleData& nameData) {
            return { "Root", {{ "Value", nameData.Value }} };
        }

        [[nodiscard]] static std::optional<ScaleData> TryDeserialize(const SerializedObject& so) {
            if(auto value = so.TryGet("Value")){
                if(auto scale = value->TryGet<float3>()){
                    return ScaleData(scale.value());
                }
            }
            return std::nullopt;
        }
    };

    /**
     * Stores the local transform matrix of ane entity relative to its parent.
     * It is calculated from the PositionData, RotationData and ScaleData and
     * should not be manually set, because it is re-calculated every frame.
     */
    struct CORE_API LocalTransformData {
        LocalTransformData() = default;

        explicit LocalTransformData(const glm::mat4& transform) : Value(transform) {}

        /**
         * The local transform matrix
         */
        float4x4 Value = float4x4(1.0f);

        /**
         * @return Returns the right vector (positive x) relative to its parent
         */
        [[nodiscard]] float3 Right() const;

        /**
         * @return Returns the up vector (positive y) relative to its parent
         */
        [[nodiscard]] float3 Up() const;

        /**
         * @return Returns the forward vector (positive z) relative to its parent
         */
        [[nodiscard]] float3 Forward() const;

        /**
         * @return Returns the position relative to its parent
         */
        [[nodiscard]] float3 Position() const;

        /**
         * @return Returns the rotation relative to its parent
         */
        [[nodiscard]] quat Rotation() const;

        /**
         * @return Returns the scale relative to its parent
         */
        [[nodiscard]] float3 Scale() const;

        /**
         * @return Returns the local transform matrix without any scaling
         */
        [[nodiscard]] float4x4 UnscaledTransform() const;
    };


    /**
     * Allows serialization of LocalTransformData
     */
    template<>
    struct Serializer<LocalTransformData>{

        [[nodiscard]] static LocalTransformData Construct() { return LocalTransformData(); }

        [[nodiscard]] static SerializedObject Serialize(const LocalTransformData& localTransformData) {
            return { "Root", {{"Value", localTransformData.Value }} };
        }

        [[nodiscard]] static std::optional<LocalTransformData> TryDeserialize(const SerializedObject& so) {
            if(auto value = so.TryGet("Value")){
                if(auto matrix = value->TryGet<float4x4>()){
                    return LocalTransformData(matrix.value());
                }
            }
            return std::nullopt;
        }
    };

    /**
     * Stores the global transform matrix of ane entity in the world space.
     * It is calculated from the LocalTransformData and the scene graph entity hierarchy and
     * should not be manually set, because it is re-calculated every frame.
     */
    struct CORE_API GlobalTransformData {
        GlobalTransformData() = default;

        explicit GlobalTransformData(const glm::mat4& transform) : Value(transform) {}

        /**
         * The global transform matrix
         */
        float4x4 Value = float4x4(1.0f);

        /**
         * @return Returns the right vector (positive x) relative to world space
         */
        [[nodiscard]] float3 Right() const;

        /**
         * @return Returns the up vector (positive y) relative to world space
         */
        [[nodiscard]] float3 Up() const;

        /**
         * @return Returns the forward vector (positive z) relative to world space
         */
        [[nodiscard]] float3 Forward() const;

        /**
         * @return Returns the position relative to world space
         */
        [[nodiscard]] float3 Position() const;

        /**
         * @return Returns the rotation relative to world space
         */
        [[nodiscard]] quat Rotation() const;

        /**
         * @return Returns the scale relative to world space
         */
        [[nodiscard]] float3 Scale() const;

        /**
         * @return Returns the global transform matrix without any scaling
         */
        [[nodiscard]] float4x4 UnscaledTransform() const;
    };

    /**
     * Allows serialization of GlobalTransformData
     */
    template<>
    struct Serializer<GlobalTransformData>{

        [[nodiscard]] static GlobalTransformData Construct() { return GlobalTransformData(); }

        [[nodiscard]] static SerializedObject Serialize(const GlobalTransformData& globalTransformData) {
            return { "Root", {{"Value", globalTransformData.Value }} };
        }

        [[nodiscard]] static std::optional<GlobalTransformData> TryDeserialize(const SerializedObject& so) {
            if(auto value = so.TryGet("Value")){
                if(auto matrix = value->TryGet<float4x4>()){
                    return GlobalTransformData(matrix.value());
                }
            }
            return std::nullopt;
        }
    };
}
