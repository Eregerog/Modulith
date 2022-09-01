/**
 * \brief
 * \author Daniel Götz
 */

#include <glad/glad.h>
#include <opengl/primitives/OpenGLBuffers.h>
#include <opengl/primitives/OpenGLVertexArray.h>
#include <opengl/primitives/OpenGLShader.h>
#include <opengl/primitives/OpenGLTexture.h>
#include "OpenGLRendererAPI.h"

namespace modulith{

    void OpenGLRendererAPI::Init() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void OpenGLRendererAPI::SetClearColor(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void OpenGLRendererAPI::Clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLRendererAPI::DrawIndexed(const shared<VertexArray>& vertexArray) {
        glDrawElements(GL_TRIANGLES, vertexArray->GetIndexBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr);
    }

    shared <VertexBuffer> OpenGLRendererAPI::CreateVertexBuffer(float* vertices, uint32_t size) {
        return std::make_shared<OpenGLVertexBuffer>(vertices, size);
    }

    shared <VertexBuffer> OpenGLRendererAPI::CreateVertexBuffer(void* vertexData, uint32_t size) {
        return std::make_shared<OpenGLVertexBuffer>(vertexData, size);
    }

    shared <IndexBuffer> OpenGLRendererAPI::CreateIndexBuffer(uint32_t* indices, uint32_t count) {
        return std::make_shared<OpenGLIndexBuffer>(indices, count);
    }

    shared <VertexArray> OpenGLRendererAPI::CreateVertexArray() {
        return std::make_shared<OpenGLVertexArray>();
    }

    shared <Shader> OpenGLRendererAPI::CreateShader(const Address& address) {
        return std::make_shared<OpenGLShader>(Context::GetInstance<AssetContext>()->ResolveAddressOrThrow(address, "OpenGLRenderAPI.CreateShader"));
    }

    shared <Shader> OpenGLRendererAPI::CreateShader(
        const std::string& name, const std::string& vertexSource, const std::string& fragmentSource
    ) {
        return std::make_shared<OpenGLShader>(name, vertexSource, fragmentSource);
    }

    shared <Texture2D> OpenGLRendererAPI::CreateTexture2D(const Address& address) {
        return std::make_shared<OpenGLTexture2D>(Context::GetInstance<AssetContext>()->ResolveAddressOrThrow(address, "OpenGLRenderAPI.CreateTexture2D"));
    }

}
