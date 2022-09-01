/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <CoreModule.h>
#include "renderer/primitives/Buffers.h"

namespace modulith{

    class IndexBuffer;

    class VertexBuffer;

    /**
     * A rendering API-independent implemention of a Vertex array,
     * which stores vertex buffers with specific layouts as well as an index buffer.
     */
    class CORE_API VertexArray {
    public:
        virtual ~VertexArray() = default;

        /**
         * This method binds the current vertex array that then can be drawn
         */
        virtual void Bind() const = 0;

        /**
         * This methods unbinds the vertex array
         */
        virtual void Unbind() const = 0;

        /**
         * Adds a vertex buffer to this array
         * @param vertexBuffer The vertex buffer to add. It must have its buffer layout set
         */
        virtual void AddVertexBuffer(const shared<VertexBuffer>& vertexBuffer) = 0;

        /**
         * Sets the index buffer of this array
         * @param indexBuffer The index buffer to set for this vertex array
         */
        virtual void SetIndexBuffer(const shared<IndexBuffer>& indexBuffer) = 0;

        /**
         * @return Returns all vertex buffers attached to this vertex array
         */
        [[nodiscard]] virtual const std::vector<shared<VertexBuffer>>& GetVertexBuffers() const = 0;

        /**
         * @return Returns the index buffer attached to this vertex array
         */
        [[nodiscard]] virtual const shared<IndexBuffer>& GetIndexBuffer() const = 0;
    };

}
