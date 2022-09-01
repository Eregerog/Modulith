/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <CoreModule.h>
#include <renderer/primitives/VertexArray.h>

namespace modulith{

    class Renderer;
    class RenderContext;

    /**
     * The data of a mesh's single vertex
     */
    struct CORE_API Vertex{
        /**
         * The vertex position in object space
         */
        float3 Position;

        /**
         * The vertex normal in object space
         */
        float3 Normal;

        /**
         * The tex coords of the vertex
         */
        float2 TexCoords;
    };

    /**
     * The rendering-api independent implementation of a mesh
     */
    class CORE_API Mesh{
        friend Renderer;

    public:
        /**
         * Creates a mesh from the given vertex positions, normals and indices
         */
        Mesh(const std::vector<float3>& positions, const std::vector<float3>& normals, std::vector<uint32_t> indices);
        /**
         * Copies another mesh by deep copying all its vertices and indices
         */
        explicit Mesh(const shared<Mesh>& other);
        /**
         * Creates a mesh from the given vertices and indices
         */
        Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

        ~Mesh();

        /**
         * @return Returns a mesh of a cube with the size of 1 in all directions and its center in the middle
         */
        static shared<Mesh> CreateCube();

        /**
         * @return Returns a mesh of a quad with the size of 1 and facing upwards
         */
        static shared<Mesh> CreateQuad();

        /**
         * @return Returns the vertices in the mesh
         */
        [[nodiscard]] uint64_t VertexCount() const { return _vertices.size(); }

        /**
         * @return Returns the indices in the mesh. The triangle count is generally a third of this value.
         */
        [[nodiscard]] uint64_t IndexCount() const { return _indices.size(); }

    private:

        static shared<Mesh> _standardCube;

        void generateVertexArray();

        std::vector<Vertex> _vertices{};
        std::vector<uint32_t> _indices{};

        shared<VertexArray> _vertexArray = nullptr;
    };
}
