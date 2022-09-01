/*
 * \brief
 * \author Daniel Götz
 */

#include "renderer/Material.h"

#define SetShaderUniform(type, uploadFunction)  Assert(std::holds_alternative<type>(uniformData.Data),\
        "The uniform data of {0} was not set up properly: "\
        "A {1} was expected, but none found!", uniformData.UniformName, #type);\
        _shader->uploadFunction(uniformData.UniformName, std::get<type>(uniformData.Data));

namespace modulith{

    void Material::Bind() {
        _shader->Bind();
        UploadUniforms();
    }

    void Material::UploadUniforms() {
        for (const ShaderUniform& uniformData : _data) {
            switch (uniformData.Type) {
                case ShaderDataType::None:
                Assert(false, "Material properties of type none are not supported!");
                    break;
                case ShaderDataType::Float:
                SetShaderUniform(float, UploadUniformFloat1);
                    break;
                case ShaderDataType::Float2:
                SetShaderUniform(float2, UploadUniformFloat2);
                    break;
                case ShaderDataType::Float3:
                SetShaderUniform(float3, UploadUniformFloat3);
                    break;
                case ShaderDataType::Float4:
                SetShaderUniform(float4, UploadUniformFloat4);
                    break;
                case ShaderDataType::Mat3:
                SetShaderUniform(float3x3, UploadUniformMat3);
                    break;
                case ShaderDataType::Mat4:
                SetShaderUniform(float4x4, UploadUniformMat4);
                    break;
                case ShaderDataType::Int:
                SetShaderUniform(int1, UploadUniformInt1);
                    break;
                case ShaderDataType::Int2:
                SetShaderUniform(int2, UploadUniformInt2);
                    break;
                case ShaderDataType::Int3:
                SetShaderUniform(int3, UploadUniformInt3);
                    break;
                case ShaderDataType::Int4:
                SetShaderUniform(int4, UploadUniformInt4);
                    break;
                case ShaderDataType::Bool:
                SetShaderUniform(bool, UploadUniformBool);
                    Assert(false, "Material properties of type bool are not supported yet!");
                    break;
            }
        }
    }

// Int Specializations

    template<>
    void Material::AddUniformValue<int1>(const std::string& propertyName, int1 data) {
        _data.push_back(ShaderUniform{propertyName, ShaderDataType::Int, data});
    }

    template<>
    void Material::AddUniformValue<int2>(const std::string& propertyName, int2 data) {
        _data.push_back(ShaderUniform{propertyName, ShaderDataType::Int2, data});
    }

    template<>
    void Material::AddUniformValue<int3>(const std::string& propertyName, int3 data) {
        _data.push_back(ShaderUniform{propertyName, ShaderDataType::Int3, data});
    }

    template<>
    void Material::AddUniformValue<int4>(const std::string& propertyName, int4 data) {
        _data.push_back(ShaderUniform{propertyName, ShaderDataType::Int4, data});
    }

// Float Specializations

    template<>
    void Material::AddUniformValue<float1>(const std::string& propertyName, float1 data) {
        _data.push_back(ShaderUniform{propertyName, ShaderDataType::Float, data});
    }

    template<>
    void Material::AddUniformValue<float2>(const std::string& propertyName, float2 data) {
        _data.push_back(ShaderUniform{propertyName, ShaderDataType::Float2, data});
    }

    template<>
    void Material::AddUniformValue<float3>(const std::string& propertyName, float3 data) {
        _data.push_back(ShaderUniform{propertyName, ShaderDataType::Float3, data});
    }

    template<>
    void Material::AddUniformValue<float4>(const std::string& propertyName, float4 data) {
        _data.push_back(ShaderUniform{propertyName, ShaderDataType::Float4, data});
    }

// Matrix Specializations

    template<>
    void Material::AddUniformValue<float3x3>(const std::string& propertyName, float3x3 data) {
        _data.push_back(ShaderUniform{propertyName, ShaderDataType::Mat3, data});
    }

    template<>
    void Material::AddUniformValue<float4x4>(const std::string& propertyName, float4x4 data) {
        _data.push_back(ShaderUniform{propertyName, ShaderDataType::Mat4, data});
    }

// Bool Specialization

    template<>
    void Material::AddUniformValue<bool>(const std::string& propertyName, bool data) {
        _data.push_back(ShaderUniform{propertyName, ShaderDataType::Bool, data});
    }
}
