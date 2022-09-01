/**
 * \brief
 * \author Daniel Götz
 */

#include <renderer/primitives/Shader.h>
#include <renderer/Renderer.h>
#include <opengl/primitives/OpenGLShader.h>
#include "renderer/RenderContext.h"

namespace modulith{

    void ShaderLibrary::Add(const shared<Shader>& shader) {
        auto& name = shader->GetName();
        Add(name, shader);
    }

    void ShaderLibrary::Add(const std::string& name, const shared<Shader>& shader) {
        _shaders[name] = shader;
    }

    shared<Shader> ShaderLibrary::Load(const Address& address) {
        auto shader = Context::GetInstance<RenderContext>()->RendererAPI()->CreateShader(address);
        Add(shader);
        return shader;
    }

    shared<Shader> ShaderLibrary::Load(const std::string& name, const Address& address) {
        auto shader = Context::GetInstance<RenderContext>()->RendererAPI()->CreateShader(address);
        Add(name, shader);
        return shader;
    }


    bool ShaderLibrary::Exists(const std::string& name) const {
        return _shaders.find(name) != _shaders.end();
    }

    shared<Shader> ShaderLibrary::Get(const std::string& name) {
        CoreAssert(_shaders.find(name) != _shaders.end(), "Shader not found!");
        return _shaders[name];
    }

    void ShaderLibrary::Reset() {
        _shaders.clear();
    }

}
