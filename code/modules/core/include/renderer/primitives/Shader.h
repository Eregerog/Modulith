/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <CoreModule.h>
#include <assets/AssetContext.h>

namespace modulith{

    /**
     * A rendering API-independent implementation of a shader
     */
    class CORE_API Shader {
    public:
        virtual ~Shader() = default;

        /**
         * Binds the shader, allowing it to be used for rendering
         */
        virtual void Bind() const = 0;

        /**
         * Unbids the shader
         */
        virtual void Unbind() const = 0;

        /**
         * Uploads an int uniform value to the shader. The shader must be bounds when this method is called.
         * @param name The name of the value used by the shader
         * @param value The value uploaded
         */
        virtual void UploadUniformInt1(const std::string& name, int value) = 0;

        /**
         * Uploads an int2 uniform value to the shader. The shader must be bounds when this method is called.
         * @param name The name of the value used by the shader
         * @param value The value uploaded
         */
        virtual void UploadUniformInt2(const std::string& name, glm::vec<2, int> value) = 0;

        /**
         * Uploads an int3 uniform value to the shader. The shader must be bounds when this method is called.
         * @param name The name of the value used by the shader
         * @param value The value uploaded
         */
        virtual void UploadUniformInt3(const std::string& name, glm::vec<3, int> value) = 0;

        /**
         * Uploads an int4 uniform value to the shader. The shader must be bounds when this method is called.
         * @param name The name of the value used by the shader
         * @param value The value uploaded
         */
        virtual void UploadUniformInt4(const std::string& name, glm::vec<4, int> value) = 0;

        /**
         * Uploads an float uniform value to the shader. The shader must be bounds when this method is called.
         * @param name The name of the value used by the shader
         * @param value The value uploaded
         */
        virtual void UploadUniformFloat1(const std::string& name, float value) = 0;

        /**
         * Uploads an float2 uniform value to the shader. The shader must be bounds when this method is called.
         * @param name The name of the value used by the shader
         * @param value The value uploaded
         */
        virtual void UploadUniformFloat2(const std::string& name, const glm::vec2& value) = 0;

        /**
         * Uploads an float3 uniform value to the shader. The shader must be bounds when this method is called.
         * @param name The name of the value used by the shader
         * @param value The value uploaded
         */
        virtual void UploadUniformFloat3(const std::string& name, const glm::vec3& value) = 0;

        /**
         * Uploads an float4 uniform value to the shader. The shader must be bounds when this method is called.
         * @param name The name of the value used by the shader
         * @param value The value uploaded
         */
        virtual void UploadUniformFloat4(const std::string& name, const glm::vec4& value) = 0;

        /**
         * Uploads an float3x3 uniform value to the shader. The shader must be bounds when this method is called.
         * @param name The name of the value used by the shader
         * @param value The value uploaded
         */
        virtual void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) = 0;

        /**
         * Uploads an float4x4 uniform value to the shader. The shader must be bounds when this method is called.
         * @param name The name of the value used by the shader
         * @param value The value uploaded
         */
        virtual void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) = 0;

        /**
         * Uploads an bool uniform value to the shader. The shader must be bounds when this method is called.
         * @param name The name of the value used by the shader
         * @param value The value uploaded
         */
        virtual void UploadUniformBool(const std::string& name, bool value) = 0;

        /**
         * @return Returns the name of the shader
         */
        [[nodiscard]] virtual const std::string& GetName() const = 0;

    };

    class RenderContext;
    namespace fs = std::filesystem;

    /**
     * The shader library can be used to load and the store shaders by name,
     * avoiding to re-loading shaders from disk whenever one is needed
     */
    class CORE_API ShaderLibrary {
    public:
        /**
         * Adds the given shader to the library. The name of the shader will be used to address it.
         */
        void Add(const shared<Shader>& shader);

        /**
         * Adds the given shader to the library with a custom name
         */
        void Add(const std::string& name, const shared<Shader>& shader);

        /**
         * Loads the given shader and adds it to the library. The name of the shader will be used to address it.
         */
        shared<Shader> Load(const Address& address);

        /**
         * Loads the given shader and adds it to the library using a custom name to address it.
         */
        shared<Shader> Load(const std::string& name, const Address& address);

        /**
         * @return Returns whether a shader with the given address name is registered in the shader library
         */
        [[nodiscard]] bool Exists(const std::string& name) const;

        /**
         * Gets the shader with the given address name from the library.
         * A shader with that name must be contained in the library
         */
        shared<Shader> Get(const std::string& name);

        /**
         * Resets the shader library, removing all registered shaders
         */
        void Reset();

    private:
        std::unordered_map<std::string, shared<Shader>> _shaders;
    };

}
