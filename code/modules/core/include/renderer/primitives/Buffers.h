/*
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <CoreModule.h>
#include "BufferLayout.h"

namespace modulith{

    /**
     * A rendering-api independent implementation of a vertex buffer
     */
    class CORE_API VertexBuffer {
    public:
        virtual ~VertexBuffer() = default;

        /**
         * Binds the vertex buffer
         */
        virtual void Bind() const = 0;

        /**
         * Unbinds the vertex buffer
         */
        virtual void Unbind() const = 0;

        /**
         * @return Returns the layout of the vertex buffer
         */
        [[nodiscard]] virtual const BufferLayout& GetLayout() const = 0;

        /**
         * Sets the layout of the vertex buffer
         */
        virtual void SetLayout(const BufferLayout& layout) = 0;

    };

    /**
     * A rendering-api independent implementation of an index buffer buffer
     */
    class CORE_API IndexBuffer {
    public:
        virtual ~IndexBuffer() = default;

        /**
         * Binds the index buffer
         */
        virtual void Bind() const = 0;

        /**
         * Unbinds the index buffer
         */
        virtual void Unbind() const = 0;

        /**
         * @return Returns the amount of indices in this index buffer
         */
        [[nodiscard]] virtual uint32_t GetCount() const = 0;

    };

}
