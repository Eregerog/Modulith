/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include <renderer/primitives/Shader.h>
#include <filesystem>

namespace modulith{

    using GLenum = unsigned int;
    using ShaderSources = std::unordered_map<GLenum, std::string>;

    class OpenGLShader : public Shader {
    public:
        explicit OpenGLShader(const std::filesystem::path& filePath);

        OpenGLShader(std::string  name, const std::string& vertexSource, const std::string& fragmentSource);

        ~OpenGLShader() override;

        void Bind() const override;

        void Unbind() const override;

        [[nodiscard]] const std::string& GetName() const override { return _name; }

        void UploadUniformInt1(const std::string& name, const int value) override;

        void UploadUniformInt2(const std::string& name, const glm::vec<2, int> value) override;

        void UploadUniformInt3(const std::string& name, const glm::vec<3, int> value) override;

        void UploadUniformInt4(const std::string& name, const glm::vec<4, int> value) override;

        void UploadUniformFloat1(const std::string& name, const float value) override;

        void UploadUniformFloat2(const std::string& name, const glm::vec2& value) override;

        void UploadUniformFloat3(const std::string& name, const glm::vec3& value) override;

        void UploadUniformFloat4(const std::string& name, const glm::vec4& value) override;

        void UploadUniformMat3(const std::string& name, const glm::mat3& matrix) override;

        void UploadUniformMat4(const std::string& name, const glm::mat4& matrix) override;

        void UploadUniformBool(const std::string& name, bool value) override;

    private:
        std::string ReadFile(const std::string& filePath);

        ShaderSources PreProcess(const std::string& sourceCode);

        void Compile(const ShaderSources& shaderSources);

        uint32_t _rendererId;
        std::string _name;
    };

}
