/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "RendererModule.h"

namespace modulith {
    namespace renderer {

        /**
         * This component contains data for rendering a 3D-object.
         * In order to render an object, it must also have a GlobalTransformData attached to it.
         */
        struct RENDERER_API RenderMeshData {
            explicit RenderMeshData(modulith::shared<modulith::Mesh> mesh) : Mesh(std::move(mesh)) {}

            RenderMeshData(modulith::shared<modulith::Mesh> mesh, modulith::shared<modulith::Material> material)
                : Mesh(std::move(mesh)), Material(std::move(material)) {}

            /**
             * The mesh of the object. Must be assigned
             */
            modulith::shared<modulith::Mesh> Mesh = nullptr;

            /**
             * The material of the object. If null, a fallback material will be used.
             */
            modulith::shared<modulith::Material> Material = nullptr;
        };

        /**
         * This component contains data for a camera.
         * If an object has this component and a GlobalsTransformData, it will render the scene.
         * The camera "looks" into the -z direction
         */
        struct RENDERER_API CameraData {

            CameraData(float fieldOfView, modulith::int2 dimension, float nearPlane, float farPlane)
                : FieldOfViewRadians(glm::radians(fieldOfView)), Dimension(dimension), NearPlane(nearPlane),
                  FarPlane(farPlane) {
                SetWidthAndHeight(Dimension);
            }

            glm::mat4 ProjectionMatrix = glm::mat4(1.0f);
            float FieldOfViewRadians;

            /**
             * The dimensions the camera renders in (in pixels)
             */
            modulith::int2 Dimension;
            float NearPlane;
            float FarPlane;

            /**
             * Changes the rendering dimension of the camera, updating its projection matrix
             * @param dimension The new rendering dimensions. If it is 0 in either dimension, 1 will be used instead.
             */
            void SetWidthAndHeight(modulith::int2 dimension) {
                if (dimension.x == 0 || dimension.y == 0)
                    return;
                Dimension = dimension;
                ProjectionMatrix = glm::perspective(
                    FieldOfViewRadians,
                    std::max<float>(1, (float) dimension.x) / std::max<float>(1, (float) dimension.y), NearPlane,
                    FarPlane
                );
            }

            [[nodiscard]] float AspectRatio() const { return ((float) Dimension.x / (float) Dimension.y); }

            /**
             * Creates a ray from the camera that goes into the direction looked at
             * @param globalTransform The global transform of this camera
             * @param screenPoint The point on the screen the ray should originate from (in pixels)
             * @return A ray object
             */
            [[nodiscard]] modulith::Ray
            ScreenPointRay(const modulith::GlobalTransformData& globalTransform, modulith::float2 screenPoint) const {
                auto dirX = screenPoint.x - (Dimension.x - 1) / 2.0f;
                auto dirY = (Dimension.y - 1) / 2.0f - screenPoint.y;
                auto dirZ = -((Dimension.x / 2.0f) / std::tan(FieldOfViewRadians / 2)) / AspectRatio();

                auto globalDirection = globalTransform.UnscaledTransform() * modulith::float4(dirX, dirY, dirZ, 0.0f);

                return modulith::Ray(globalTransform.Position(), globalDirection);
            }
        };

        /**
         * Contains the data for a point light.
         * When this component is attached alongside a GlobalTransformComponent,
         * a light source at that location with the given parameters is created.
         */
        struct RENDERER_API PointLightData {

            PointLightData(const modulith::float3& color, float rangeMeters) : Color(color), Range(rangeMeters) {}

            /**
             * The light color emitted
             */
            modulith::float3 Color{1.0f, 1.0f, 1.0f};

            /**
             * The range of the light and the strength at the center
             */
            float Range = 10.0f;
        };

        /**
         * Contains the data for a directional light.
         * The light shines into the -z direction.
         * When this component is attached alongside a GlobalTransformComponent,
         * a light source with the transform's orientation and the given parameters is created.
         */
        struct RENDERER_API DirectionalLightData {

            DirectionalLightData() = default;

            DirectionalLightData(const modulith::float3& color, float ambientFactor) : Color(color),
                                                                                       AmbientFactor(ambientFactor) {}
            /**
             * The color of the light.
             * Light strength can be controlled by decreasing the HSV Value
             */
            modulith::float3 Color{1.0f, 1.0f, 1.0f};

            /**
             * The strength of the ambient light in the scene.
             * Should not be below 0.
             */
            float AmbientFactor = 0.15f;
        };
    }

    /**
     * Allows serialization of the renderer::CameraData
     */
    template<>
    struct Serializer<renderer::CameraData> {
        [[nodiscard]] static renderer::CameraData Construct() { return renderer::CameraData(65, int2(1280, 720), 0.1f, 1000.0f); }

        [[nodiscard]] static SerializedObject Serialize(const renderer::CameraData& cameraData) {
            return {"Root", {
                {"Field of View", glm::degrees(cameraData.FieldOfViewRadians)},
                {"Render Dimensions", cameraData.Dimension},
                {"Near Plane", cameraData.NearPlane},
                {"Far Plane", cameraData.FarPlane}
            }};
        }

        [[nodiscard]] static std::optional<renderer::CameraData> TryDeserialize(const SerializedObject& so) {
            auto fieldOfView = so.TryGet<float>("Field of View");
            auto renderDimensions = so.TryGet<int2>("Render Dimensions");
            auto nearPlane = so.TryGet<float>("Near Plane");
            auto farPlane = so.TryGet<float>("Far Plane");

            if (fieldOfView && renderDimensions && nearPlane && farPlane)
                return renderer::CameraData(
                    std::clamp(fieldOfView.value(), 0.1f, 179.9f) /* the constructor takes fov as degrees */,
                    renderDimensions.value(),
                    nearPlane.value(),
                    farPlane.value()
                );

            return std::nullopt;
        }
    };

    /**
     * Allows serialization of the renderer::PointLightData
     */
    template<>
    struct Serializer<renderer::PointLightData> {
        [[nodiscard]] static renderer::PointLightData Construct() { return renderer::PointLightData({1, 1, 1,}, 15); }

        [[nodiscard]] static SerializedObject Serialize(const renderer::PointLightData& pointLightData) {
            return {"Root", {
                {"Color", color3(pointLightData.Color)},
                {"Range", pointLightData.Range}
            }};
        }

        [[nodiscard]] static std::optional<renderer::PointLightData> TryDeserialize(const SerializedObject& so) {
            auto color = so.TryGet<color3>("Color");
            auto range = so.TryGet<float>("Range");

            if (color && range)
                return renderer::PointLightData(color->Value, std::max(0.0f, range.value()));

            return std::nullopt;
        }
    };

    /**
     * Allows serialization of the renderer::DirectionalLightData
     */
    template<>
    struct Serializer<renderer::DirectionalLightData> {
        [[nodiscard]] static renderer::DirectionalLightData Construct() { return renderer::DirectionalLightData(); }

        [[nodiscard]] static SerializedObject Serialize(const renderer::DirectionalLightData& pointLightData) {
            return {"Root", {
                {"Color", color3(pointLightData.Color)},
                {"Ambient Factor", pointLightData.AmbientFactor}
            }};
        }

        [[nodiscard]] static std::optional<renderer::DirectionalLightData> TryDeserialize(const SerializedObject& so) {
            auto color = so.TryGet<color3>("Color");
            auto ambientFactor = so.TryGet<float>("Ambient Factor");

            if (color && ambientFactor)
                return renderer::DirectionalLightData(color->Value, std::max(0.0f, ambientFactor.value()));

            return std::nullopt;
        }
    };
}

