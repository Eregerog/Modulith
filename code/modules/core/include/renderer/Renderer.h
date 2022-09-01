/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include "RendererAPI.h"
#include "renderer/primitives/Shader.h"
#include "Mesh.h"
#include "Material.h"

namespace modulith{

    class RenderContext;

    /**
     * The renderer collects information about the current scene and renders it
     */
    class CORE_API Renderer {

        friend class RenderContext;

    public:

        explicit Renderer(const shared<RendererAPI>& rendererAPI) : _api(rendererAPI){}

        /**
        * The data representation of a directional light
        */
        struct DirectionalLight{

            DirectionalLight() = default;

            DirectionalLight(const float3& direction, const float3& color, float ambientFactor)
                : Direction(direction), Color(color), AmbientFactor(ambientFactor) {}


            float3 Direction{};
            float3 Color{0.0f, 0.0f, 0.0f};

            float AmbientFactor = 0.0f;
        };

        /**
         * The data representation of a point light
         */
        struct PointLight{
            PointLight() = default;
            PointLight(float3 position, float3 color, float range) : Position(position), Color(color), Range(range){}

            float3 Position{};
            float3 Color{};

            float Range{};
        };

        /**
         * Statistics about a drawn scene
         */
        struct SceneStats{

            /**
             * How often SubmitImmediately was called
             */
            uint32_t ImmediateSubmits = 0;
            /**
             * How often SubmitDeferred was called
             */
            uint32_t DeferredSubmits = 0;

            /**
             * How many material batches were used when drawing deferred meshes.
             * This is equal to the amount of unique materials submitted via DeferredSubmit
             */
            uint32_t MaterialBatches = 0;
            /**
             * How many vertex array batches were used when drawing deferred meshes.
             * This is equal to the amount of meshes that were unique that also were submitted with the same material
             */
            uint32_t VertexArrayBatches = 0;
            /**
             * The amount of draw calls made during batching.
             */
            uint32_t BatchedDrawCalls = 0;

            /**
             * The total amount of vertices rendered
             */
            uint64_t Vertices = 0;
            /**
             * The total amount of triangles rendered
             */
            uint64_t Triangles = 0;

            [[nodiscard]] SceneStats CombineWith(const SceneStats& other) const{
                return SceneStats {
                    ImmediateSubmits + other.ImmediateSubmits,
                    DeferredSubmits + other.DeferredSubmits,

                    MaterialBatches + other.MaterialBatches,
                    VertexArrayBatches + other.VertexArrayBatches,
                    BatchedDrawCalls + other.BatchedDrawCalls,

                    Vertices + other.Vertices,
                    Triangles + other.Triangles,
                };
            }
        };

        /**
         * Begins a scene rendered by a camera
         * @param projectionMatrix The projection matrix of the camera
         * @param viewMatrix The view matrix of the camera
         * @param cameraPosition The position of the camera in worldspace
         * @param directionalLight The directional light to be rendered
         * @param pointLights The point lights to be rendered
         */
        void BeginScene(glm::mat4 projectionMatrix, float4x4 viewMatrix, float3 cameraPosition, std::optional<DirectionalLight> directionalLight, std::vector<PointLight> pointLights);

        /**
         * Submits an object to be rendered in the current scene immediately.
         * This will not make use of any batching optimizations!
         * @param material The material of the object
         * @param mesh The mesh of the object
         * @param transform The worldspace to object space transform matrix of the object
         */
        void SubmitImmediately(
            const shared<Material>& material, const shared<Mesh>& mesh, glm::mat4 transform);

        /**
         * Submits an object to be rendered in the current scene once EndScene is called.
         * This call is more performant as it may make use of batching
         * @param material The material of the object
         * @param mesh The mesh of the object
         * @param transform The worldspace to object space transform matrix of the object
         */
        void SubmitDeferred(const shared<Material>& material, const shared<Mesh>& mesh, glm::mat4 transform);

        /**
         * Ends the current scene, resetting all registered lights.
         */
        SceneStats EndScene();

        shared<RendererAPI> GetAPI() { return _api; }

    private:

        void bindMaterial(const shared<Material>& shader);
        void bindVertexArray(const shared<VertexArray>& vertexArray);
        void drawVertexArray(const shared<Shader>& activeShader, const shared<VertexArray>& vertexArray, float4x4 matrix);

        void initialize();
        void shutdown();
        void beginFrame();
        void endFrame();

        struct VertexArrayBatch{
            std::vector<float4x4> _matrices;
        };

        struct MaterialBatch{
            std::unordered_map<shared<VertexArray>, VertexArrayBatch> _vertexArrayBatch{};
        };

        struct SceneData {
            SceneStats Stats{};

            float4x4 ProjectionMatrix;
            float4x4 ViewMatrix;

            std::optional<DirectionalLight> CurrentDirectionalLight{};
            std::vector<PointLight> Lights{};
            PointLight CurrentCameraLights[4];
            int CurrentLightCount = 0;

            std::unordered_map<shared<Material>, MaterialBatch> _batches{};
        };

        owned<SceneData> _sceneData;


        void initTex(int2 initialTextureSize);
        void resize(int2 newTextureSize);
        void destroyTex();

        uint32_t _frameBuffer;
        uint32_t _colorTex;
        uint32_t _depthTex;

        int2 _currentImGuiFramebufferSize;

        shared<RendererAPI> _api;
    };

}
