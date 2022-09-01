/**
 * \brief
 * \author Daniel Götz
 */


#include "OpenGLVertexArray.h"
#include "glad/glad.h"

namespace modulith{

    static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type) {
        switch (type) {
            case ShaderDataType::Float:
            case ShaderDataType::Float2:
            case ShaderDataType::Float3:
            case ShaderDataType::Float4:
            case ShaderDataType::Mat3:
            case ShaderDataType::Mat4:
                return GL_FLOAT;
            case ShaderDataType::Int:
            case ShaderDataType::Int2:
            case ShaderDataType::Int3:
            case ShaderDataType::Int4:
                return GL_INT;
            case ShaderDataType::Bool:
                return GL_BOOL;
            default:
            CoreAssert(false, "Unknown ShaderDataType {0}", type);
                return 0;
        }


    }

    OpenGLVertexArray::OpenGLVertexArray() {
        glCreateVertexArrays(1, &_rendererId);
    }

    OpenGLVertexArray::~OpenGLVertexArray() {
        glDeleteVertexArrays(1, &_rendererId);
    }

    void OpenGLVertexArray::Bind() const {
        glBindVertexArray(_rendererId);
    }

    void OpenGLVertexArray::Unbind() const {
        glBindVertexArray(0);
    }

    void OpenGLVertexArray::AddVertexBuffer(const shared<VertexBuffer>& vertexBuffer) {
        glBindVertexArray(_rendererId);
        vertexBuffer->Bind();

        CoreAssert(
            !vertexBuffer->GetLayout().GetElements().empty(),
            "The vertex buffer does not have a layout! AddVertexBuffer should be called after setting the layout"
        );


        uint32_t index = 0;
        const auto& layout = vertexBuffer->GetLayout();
        for (const auto& element : layout.GetElements()) {
            glEnableVertexAttribArray(index);
            glVertexAttribPointer(
                index,
                element.GetComponentCount(),
                ShaderDataTypeToOpenGLBaseType(element.Type),
                element.ShouldBeNormalized ? GL_TRUE : GL_FALSE,
                layout.GetStride(),
                (const void*) element.Offset
            );
            index++;
        }

        _vertexBuffers.push_back(vertexBuffer);
    }

    void OpenGLVertexArray::SetIndexBuffer(const shared<IndexBuffer>& indexBuffer) {
        glBindVertexArray(_rendererId);
        indexBuffer->Bind();

        _indexBuffer = indexBuffer;
    }

    const std::vector<shared<VertexBuffer>>& OpenGLVertexArray::GetVertexBuffers() const {
        return _vertexBuffers;
    }

    const shared<IndexBuffer>& OpenGLVertexArray::GetIndexBuffer() const {
        return _indexBuffer;
    }


}
