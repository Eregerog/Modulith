/**
 * \brief
 * \author Daniel Götz
 */


#pragma once

#include "CoreModule.h"
#include <renderer/primitives/Buffers.h>
#include <renderer/primitives/VertexArray.h>

namespace modulith{

    class OpenGLVertexArray : public VertexArray {
    public:
        OpenGLVertexArray();

        ~OpenGLVertexArray() override;

        void Bind() const override;

        void Unbind() const override;

        void AddVertexBuffer(const shared<VertexBuffer>& vertexBuffer) override;

        void SetIndexBuffer(const shared<IndexBuffer>& indexBuffer) override;

        [[nodiscard]] const std::vector<shared<VertexBuffer>>& GetVertexBuffers() const override;

        [[nodiscard]] const shared<IndexBuffer>& GetIndexBuffer() const override;

    private:
        uint32_t _rendererId;

        std::vector<shared<VertexBuffer>> _vertexBuffers;
        shared<IndexBuffer> _indexBuffer;
    };

}
