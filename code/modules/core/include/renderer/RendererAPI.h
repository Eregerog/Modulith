/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include <renderer/primitives/Shader.h>
#include <renderer/primitives/Texture.h>
#include "renderer/primitives/VertexArray.h"
#include <filesystem>
#include <assets/AssetContext.h>

namespace fs = std::filesystem;

namespace modulith{

    /**
     * Implementations of this allow a certain rendering api to be used in the engine
     */
    class CORE_API RendererAPI {
    public:
        /**
         * This function is called to initialize the renderer API
         */
        virtual void Init() = 0;

        /**
         * This function should change the color used for clearing the screen (e.g. the color a new buffer is initialized with)
         * @param color The color the clear command
         */
        virtual void SetClearColor(const glm::vec4& color) = 0;

        /**
         * When called the renderer should clear the appropriate render buffers
         */
        virtual void Clear() = 0;

        /**
         * When called the currently bound vertex array should be rendered by the renderer API
         * @param vertexArray The vertexArray that is already bound which should be rendered
         */
        virtual void DrawIndexed(const shared<VertexArray>& vertexArray) = 0;

        /**
        * Creates a vertex buffer from the given vertices
        * @param vertices A pointer to an array of vertices
        * @param size The size (in bytes) of the vertices pointer array
        * @return The created vertex buffer
        */
        virtual shared<VertexBuffer> CreateVertexBuffer(float* vertices, uint32_t size) = 0;

        /**
         * Creates a vertex buffer from the given vertex data, allowing
         * the user to upload any data to the vertex array.
         *
         * SetLayout should be used afterwards to specify the data
         * and data types for each vertex.
         *
         * @param vertices A pointer to the vertex data
         * @param size The size (in bytes) of the vertex data pointer array
         * @return The created vertex buffer
         */
        virtual shared<VertexBuffer> CreateVertexBuffer(void* vertexData, uint32_t size) = 0;

        /**
         * Creates an index buffer from a reference to an array of indices
         * and the amount of indices
         * @param indices A pointer to a contiguous storage of indices(like an array)
         * @param count How many indices are at the pointer
         */
        virtual shared<IndexBuffer> CreateIndexBuffer(uint32_t* indices, uint32_t count) = 0;

        /**
         * Creates a vertex array based on the current rendering API
         */
        virtual shared<VertexArray> CreateVertexArray() = 0;

        /**
        * Creates a shader by loading and parsing the shader file found at the file path
        * This file is vertex and fragment shader in one,
        * the former denoted by a "#type vertex" before its source
        * and the latter denoted by a "#type fragment" before its source
        */
        virtual shared<Shader> CreateShader(const Address& address) = 0;

        /**
         * Creates a shader of the given name from the given vertex and fragment source code
         */
        virtual shared<Shader> CreateShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource) = 0;


        /**
         * Creates a 2D texture by loading the texture file at the given file path
         */
        virtual shared<Texture2D> CreateTexture2D(const Address& address) = 0;
    };

}
