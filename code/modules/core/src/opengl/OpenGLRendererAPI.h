/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <renderer/RendererAPI.h>

namespace modulith{

    class OpenGLRendererAPI : public RendererAPI {
    public:

        void Init() override;


        void SetClearColor(const glm::vec4& color) override;
        void Clear() override;

        void DrawIndexed(const shared<VertexArray>& vertexArray) override;

        shared <VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size) override;

        shared <VertexBuffer> CreateVertexBuffer(void* vertexData, uint32_t size) override;

        shared <IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count) override;

        shared <VertexArray> CreateVertexArray() override;

        shared <Shader> CreateShader(const Address& address) override;

        shared <Shader> CreateShader(
            const std::string& name, const std::string& vertexSource, const std::string& fragmentSource
        ) override;

        shared <Texture2D> CreateTexture2D(const Address& address) override;
    };


}
