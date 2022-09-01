/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include <CoreModule.h>
#include <renderer/primitives/Shader.h>
#include <renderer/primitives/BufferLayout.h>

namespace modulith{

    class Renderer;

    /**
     * A variant that can hold either of the data of a shader's uniform variable
     */
    using UniformData = std::variant<int1, int2, int3, int4, float1, float2, float3, float4, float3x3, float4x4, bool>;

    /**
     * Data for an instance of a shader's uniform variable
     */
    struct ShaderUniform {
        /**
         * The name of the uniform variable, by which it can be accessed
         */
        std::string UniformName;
        /**
         * The type of the uniform variable
         */
        ShaderDataType Type;
        /**
         * The data of that uniform variable
         */
        UniformData Data;
    };

    /**
     * A rendering-api independent implementation of a material, which is an instance of a Shader
     */
    class CORE_API Material {
        friend Renderer;

    public:
        /**
         * Creates a material from the given shader
         */
        explicit Material(shared<Shader> shader) : _shader(std::move(shader)) {}

        /**
         * Registers a uniform value with data to be assigned when the material is bound
         * @tparam TData The type of data and the uniform variable. Only support a subset of all types
         * @param propertyName The name of the uniform property
         * @param data The data to assign to the property
         */
        template<class TData>
        void AddUniformValue(const std::string& propertyName, TData data);

        /**
         * Binds the material's shader and sets all its uniform values
         */
        void Bind();

        /**
         * Only sets all the uniform values without binding the shader.
         * The shader should already be bound!
         *
         * Can be overwritten if the subclass needs to upload additional data
         */
        virtual void UploadUniforms();

        shared<Shader> GetShader() { return _shader; }

    private:
        std::vector<ShaderUniform> _data{};
    protected:
        shared<Shader> _shader;
    };

// Int Specializations

    template<>
    void Material::AddUniformValue<int1>(const std::string& propertyName, int1 data);

    template<>
    void Material::AddUniformValue<int2>(const std::string& propertyName, int2 data);

    template<>
    void Material::AddUniformValue<int3>(const std::string& propertyName, int3 data);

    template<>
    void Material::AddUniformValue<int4>(const std::string& propertyName, int4 data);


// Float Specializations

    template<>
    void Material::AddUniformValue<float1>(const std::string& propertyName, float1 data);

    template<>
    void Material::AddUniformValue<float2>(const std::string& propertyName, float2 data);

    template<>
    void Material::AddUniformValue<float3>(const std::string& propertyName, float3 data);

    template<>
    void Material::AddUniformValue<float4>(const std::string& propertyName, float4 data);

// Matrix Specializations

    template<>
    void Material::AddUniformValue<float3x3>(const std::string& propertyName, float3x3 data);

    template<>
    void Material::AddUniformValue<float4x4>(const std::string& propertyName, float4x4 data);

// Bool Specialization

    template<>
    void Material::AddUniformValue<bool>(const std::string& propertyName, bool data);
}
