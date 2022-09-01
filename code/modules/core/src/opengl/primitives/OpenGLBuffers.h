/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <renderer/primitives/BufferLayout.h>
#include <renderer/primitives/Buffers.h>

namespace modulith{

    class OpenGLVertexBuffer : public VertexBuffer {
    public:
        OpenGLVertexBuffer(float* vertices, uint32_t size);
        OpenGLVertexBuffer(void* vertexData, uint32_t size);

        ~OpenGLVertexBuffer() override;

        void Bind() const override;

        void Unbind() const override;

        [[nodiscard]] const BufferLayout& GetLayout() const override { return _layout; }

        void SetLayout(const BufferLayout& layout) override { _layout = layout; }

    private:
        uint32_t _rendererId;
        BufferLayout _layout;
    };

    class OpenGLIndexBuffer : public IndexBuffer {
    public:
        OpenGLIndexBuffer(unsigned int* indices, uint32_t size);

        ~OpenGLIndexBuffer() override;

        void Bind() const override;

        void Unbind() const override;

        [[nodiscard]] uint32_t GetCount() const override;

    private:
        uint32_t _rendererId;
        uint32_t _count;
    };

}
