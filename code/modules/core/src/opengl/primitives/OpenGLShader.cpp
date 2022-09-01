/**
 * \brief
 * \author Daniel Götz
 */

#include "OpenGLShader.h"
#include <glad/glad.h>
#include <fstream>
#include <array>
#include <glm/ext.hpp>
#include <utility>

namespace modulith{

    static GLenum ShaderTypeFromString(const std::string& type) {
        if (type == "vertex") return GL_VERTEX_SHADER;
        if (type == "fragment" || type == "pixel") return GL_FRAGMENT_SHADER;

        CoreAssert(false, "Unsupported shader type {0}", type);
        return 0;
    }

    OpenGLShader::OpenGLShader(const std::filesystem::path& filePath) {
        auto filePathAsString = filePath.generic_string();
        auto lastSlash = filePathAsString.find_last_of("/\\");
        lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
        auto lastDot = filePathAsString.rfind('.');
        auto count = lastDot == std::string::npos ? filePathAsString.size() - lastSlash : lastDot - lastSlash;
        _name = filePathAsString.substr(lastSlash, count);

        std::string shaderSource = ReadFile(filePathAsString);
        auto shaderSources = PreProcess(shaderSource);
        Compile(shaderSources);
    }

    OpenGLShader::OpenGLShader(std::string  name, const std::string& vertexSource, const std::string& fragmentSource)
        : _name(std::move(name)) {
        ShaderSources shaderSources;
        shaderSources[GL_VERTEX_SHADER] = vertexSource;
        shaderSources[GL_FRAGMENT_SHADER] = fragmentSource;
        Compile(shaderSources);
    }

    OpenGLShader::~OpenGLShader() {
        glDeleteProgram(_rendererId);
    }

    std::string OpenGLShader::ReadFile(const std::string& filePath) {
        std::string result;
        std::ifstream in(filePath, std::ios::in | std::ios::binary);
        if (in) {
            in.seekg(0, std::ios::end);
            result.resize(in.tellg());
            in.seekg(0, std::ios::beg);
            in.read(&result[0], result.size());
            in.close();
        } else {
            CoreLogError("Could not load shader {0}", filePath);
        }
        return result;
    }


    ShaderSources OpenGLShader::PreProcess(const std::string& sourceCode) {
        std::unordered_map<GLenum, std::string> result;

        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = sourceCode.find(typeToken, 0);

        while (pos != std::string::npos) {
            size_t endOfLine = sourceCode.find_first_of("\r\n", pos);
            CoreAssert(endOfLine != std::string::npos, "Syntax Error: The file ended prematurely");
            size_t begin = pos + typeTokenLength + 1;
            std::string type = sourceCode.substr(begin, endOfLine - begin);
            GLenum shaderType = ShaderTypeFromString(type);
            CoreAssert(shaderType != 0, "Invalid shader type!");

            size_t nextLinePos = sourceCode.find_first_not_of("\r\n", endOfLine);
            pos = sourceCode.find(typeToken, nextLinePos);
            result[shaderType] = sourceCode.substr(
                nextLinePos,
                pos - (nextLinePos == std::string::npos ? sourceCode.size() - 1 : nextLinePos)
            );
        }

        return result;
    }

    void OpenGLShader::Compile(const ShaderSources& shaderSources) {

        auto program = glCreateProgram();

        CoreAssert(shaderSources.size() <= 2, "Only up to 2 shaders supported!");
        std::array<GLenum, 2> glShaderIds{};
        int glShaderIdIndex = 0;

        for (auto& kvp : shaderSources) {
            GLenum shaderType = kvp.first;
            const std::string& source = kvp.second;

            GLuint shader = glCreateShader(shaderType);

            const GLchar* sourceCode = source.c_str();
            glShaderSource(shader, 1, &sourceCode, 0);

            glCompileShader(shader);

            GLint isCompiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE) {
                GLint maxLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

                glDeleteShader(shader);

                CoreAssert(false, "Shader compilation failed! {0}", infoLog.data());

                break;
            }
            glAttachShader(program, shader);
            glShaderIds[glShaderIdIndex++] = shader;
        }

        glLinkProgram(program);

        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int*) &isLinked);
        if (isLinked == GL_FALSE) {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            glDeleteProgram(program);

            for (auto id : glShaderIds) {
                glDeleteShader(id);
            }

            CoreAssert(false, "Shader linking failed! {0}", infoLog.data());

            return;
        }

        for (auto shaderId : glShaderIds) {
            glDetachShader(_rendererId, shaderId);
        }

        _rendererId = program;
    }

    void OpenGLShader::Bind() const {
        glUseProgram(_rendererId);
    }

    void OpenGLShader::Unbind() const {
        glUseProgram(0);
    }

// --- INT ---


    void OpenGLShader::UploadUniformInt1(const std::string& name, const int value) {
        GLint location = glGetUniformLocation(_rendererId, name.c_str());
        glUniform1i(location, value);
    }

    void OpenGLShader::UploadUniformInt2(const std::string& name, const glm::vec<2, int> value) {
        GLint location = glGetUniformLocation(_rendererId, name.c_str());
        glUniform2i(location, value.x, value.y);
    }

    void OpenGLShader::UploadUniformInt3(const std::string& name, const glm::vec<3, int> value) {
        GLint location = glGetUniformLocation(_rendererId, name.c_str());
        glUniform3i(location, value.x, value.y, value.z);
    }

    void OpenGLShader::UploadUniformInt4(const std::string& name, const glm::vec<4, int> value) {
        GLint location = glGetUniformLocation(_rendererId, name.c_str());
        glUniform4i(location, value.x, value.y, value.z, value.w);
    }

// --- FLOAT ---

    void OpenGLShader::UploadUniformFloat1(const std::string& name, const float value) {
        GLint location = glGetUniformLocation(_rendererId, name.c_str());
        glUniform1f(location, value);
    }

    void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value) {
        GLint location = glGetUniformLocation(_rendererId, name.c_str());
        glUniform2f(location, value.x, value.y);
    }

    void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value) {
        GLint location = glGetUniformLocation(_rendererId, name.c_str());
        glUniform3f(location, value.x, value.y, value.z);
    }

    void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value) {
        GLint location = glGetUniformLocation(_rendererId, name.c_str());
        glUniform4f(location, value.x, value.y, value.z, value.w);
    }

// --- MAT ---

    void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix) {
        GLint location = glGetUniformLocation(_rendererId, name.c_str());
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix) {
        GLint location = glGetUniformLocation(_rendererId, name.c_str());
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
    }

// --- BOOL ---

    void OpenGLShader::UploadUniformBool(const std::string& name, bool value) {
        GLint location = glGetUniformLocation(_rendererId, name.c_str());
        glUniform1i(location, value);
    }

}
