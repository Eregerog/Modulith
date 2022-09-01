/**
 * \brief
 * \author Daniel Götz
 */

#include "renderer/Mesh.h"
#include <renderer/ModelLoaderUtils.h>
#include "Context.h"
#include "renderer/RenderContext.h"

namespace modulith{

    shared<Mesh> Mesh::_standardCube = nullptr;


    Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) {
        _vertices = vertices;
        _indices = indices;

        generateVertexArray();
    }

    Mesh::Mesh(const std::vector<float3>& positions, const std::vector<glm::vec3>& normals, std::vector<uint32_t> indices) {
        for (int i = 0; i < positions.size(); ++i) {
            if (normals.size() > i) {
                _vertices.push_back(Vertex{positions[i], normals[i]});
            } else {
                _vertices.push_back(Vertex{positions[i]});
            }
        }
        _indices = std::move(indices);

        generateVertexArray();
    }

    Mesh::Mesh(const shared<Mesh>& other) {
        for (auto vertex : other->_vertices)
            _vertices.push_back(vertex);
        for (auto index : other->_indices)
            _indices.push_back(index);

        generateVertexArray();
    }

    Mesh::~Mesh() = default;

    void Mesh::generateVertexArray() {
        auto renderingAPI = Context::GetInstance<RenderContext>()->RendererAPI();

        _vertexArray = renderingAPI->CreateVertexArray();

        auto vertexBuffer = renderingAPI->CreateVertexBuffer(&_vertices[0], _vertices.size() * sizeof(Vertex));
        vertexBuffer->SetLayout(
            {
                {ShaderDataType::Float3, "a_Position"},
                {ShaderDataType::Float3, "a_Normal"},
                {ShaderDataType::Float2, "a_TexCoord"}
            }
        );

        _vertexArray->AddVertexBuffer(vertexBuffer);

        auto indexBuffer = renderingAPI->CreateIndexBuffer(&_indices[0], _indices.size());
        _vertexArray->SetIndexBuffer(indexBuffer);

        _vertexArray->Unbind();
    }


    shared<Mesh> Mesh::CreateCube() {
        if (_standardCube == nullptr)
            _standardCube = ModelLoaderUtils::LoadSingleFromFile(Address() / "standard" / "meshes" / "Cube.obj");

        return _standardCube;
    }

    shared<Mesh> Mesh::CreateQuad() {
        std::vector<float3> vertices = {{-.5f, 0.0f, -.5f},
                                        {-.5f, 0.0f, .5f},
                                        {.5f,  0.0f, .5f},
                                        {.5f,  0.0f, -.5f}};

        std::vector<float3> normals = {{0.0f, 1.0f, 0.0f},
                                       {0.0f, 1.0f, 0.0f},
                                       {0.0f, 1.0f, 0.0f},
                                       {0.0f, 1.0f, 0.0f}};

        std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};

        return std::make_shared<Mesh>(vertices, normals, indices);
    }
}
