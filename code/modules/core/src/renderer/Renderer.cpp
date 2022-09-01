/**
 * \brief
 * \author Daniel Götz
 */

#include <renderer/Renderer.h>
#include "Context.h"
#include <glad/glad.h>
#include "renderer/RenderContext.h"

#include <utility>

namespace modulith{

// TODO temp for performance
    const std::string _uniformP = "u_P";
    const std::string _uniformV = "u_V";
    const std::string _uniformM = "u_M";
    const std::string _uniformN = "u_N";

    const std::string _uniformCameraPosition = "u_CameraPosition";
    const std::string _uniformLightCount = "u_LightCount";

    std::string _uniformLightField(int index, const std::string& fieldName) {
        return "u_Lights["+ std::to_string(index) + "]." + fieldName;
    }

    void Renderer::initialize() {
        _api->Init();

        auto window = Context::GetInstance<RenderContext>()->GetWindow();
        initTex(int2(window->GetWidth(), window->GetHeight()));
    }

    void Renderer::shutdown() {
        destroyTex();
    }


    void Renderer::bindMaterial(const shared<Material>& material){
        const auto& shader = material->GetShader();
        material->Bind();

        shader->UploadUniformMat4(_uniformP, _sceneData->ProjectionMatrix);
        shader->UploadUniformMat4(_uniformV, _sceneData->ViewMatrix);

        // Camera is always at origin, since lighting is calculated in view space
        shader->UploadUniformFloat3(_uniformCameraPosition, float3(0.0f));

        shader->UploadUniformBool("u_DirectionalLight.Exists", _sceneData->CurrentDirectionalLight.has_value());
        if(_sceneData->CurrentDirectionalLight.has_value()){
            shader->UploadUniformFloat3("u_DirectionalLight.Direction", _sceneData->ViewMatrix * float4(_sceneData->CurrentDirectionalLight->Direction, 0.0f));
            shader->UploadUniformFloat3("u_DirectionalLight.Color", _sceneData->CurrentDirectionalLight->Color);
            shader->UploadUniformFloat1("u_DirectionalLight.AmbientFactor", _sceneData->CurrentDirectionalLight->AmbientFactor);
        }


        shader->UploadUniformInt1(_uniformLightCount, _sceneData->CurrentLightCount);
        for(int i = 0; i < _sceneData->CurrentLightCount; ++i){
            auto light = _sceneData->CurrentCameraLights[i];
            shader->UploadUniformFloat3(_uniformLightField(i, "Position"), light.Position);
            shader->UploadUniformFloat3(_uniformLightField(i, "Color"), light.Color);

            // Quadratic falloff, so the light has 10% of its power at its range
            shader->UploadUniformFloat1(_uniformLightField(i, "Constant"), 1.0f);
            shader->UploadUniformFloat1(_uniformLightField(i, "Linear"), 0.0f);
            auto quadratic = 10.0f / std::pow(light.Range, 2);
            shader->UploadUniformFloat1(_uniformLightField(i, "Quadratic"), quadratic);
            shader->UploadUniformFloat1(_uniformLightField(i, "AmbientFactor"), 0.0f);
        }

    }

    void Renderer::bindVertexArray(const shared<VertexArray>& vertexArray){
        vertexArray->Bind();
    }

    void Renderer::drawVertexArray(const shared<Shader>& activeShader, const shared<VertexArray>& vertexArray, float4x4 matrix){
        activeShader->UploadUniformMat4(_uniformM, matrix);
        auto normalMatrix = glm::transpose(glm::inverse(_sceneData->ViewMatrix * matrix));
        activeShader->UploadUniformMat4(_uniformN, normalMatrix);

        _api->DrawIndexed(vertexArray);
    }

    void Renderer::BeginScene(glm::mat4 projectionMatrix, float4x4 viewMatrix, float3 cameraPosition, std::optional<DirectionalLight> directionalLight, std::vector<PointLight> pointLights) {
        _sceneData = std::make_unique<SceneData>();

        _sceneData->ProjectionMatrix = projectionMatrix;
        _sceneData->ViewMatrix = viewMatrix;

        _sceneData->CurrentDirectionalLight = directionalLight;
        _sceneData->Lights = std::move(pointLights);

        std::sort(_sceneData->Lights.begin(), _sceneData->Lights.end(), [cameraPosition](const PointLight& first, const PointLight& second){
            return glm::distance(first.Position, cameraPosition) < glm::distance(second.Position, cameraPosition);
        });

        _sceneData->CurrentLightCount = std::min<int>(4, _sceneData->Lights.size());
        for(int i = 0; i < _sceneData->CurrentLightCount; ++i){
            auto light = _sceneData->Lights[i];
            light.Position = _sceneData->ViewMatrix * float4(light.Position, 1.0f);
            _sceneData->CurrentCameraLights[i] = light;
        }
    }


    Renderer::SceneStats Renderer::EndScene() {
        // Copy is needed because scene data is destroyed!
        Renderer::SceneStats stats = _sceneData->Stats;

        for(auto& materialKvp : _sceneData->_batches){
            bindMaterial(materialKvp.first);
            stats.MaterialBatches += 1;
            for(auto& vertexArrayKvp : materialKvp.second._vertexArrayBatch){
                bindVertexArray(vertexArrayKvp.first);
                stats.VertexArrayBatches += 1;
                for(auto& drawTransform : vertexArrayKvp.second._matrices){
                    drawVertexArray(materialKvp.first->GetShader(), vertexArrayKvp.first, drawTransform);
                    stats.BatchedDrawCalls += 1;
                }
            }
        }

        _sceneData.reset();
        return stats;
    }

    void Renderer::SubmitImmediately(const shared<Material>& material, const shared<Mesh>& mesh, glm::mat4 transform) {
        bindMaterial(material);
        bindVertexArray(mesh->_vertexArray);
        drawVertexArray(material->GetShader(), mesh->_vertexArray, transform);

        auto& stats = _sceneData->Stats;
        stats.ImmediateSubmits += 1;
        stats.Vertices += mesh->VertexCount();
        stats.Triangles += mesh->IndexCount() / 3;
    }

    void Renderer::SubmitDeferred(const shared<Material> &material, const shared<Mesh> &mesh, glm::mat4 transform) {
        auto& materialBatch = _sceneData->_batches[material];
        auto& vertexArrayBatch = materialBatch._vertexArrayBatch[mesh->_vertexArray];
        vertexArrayBatch._matrices.push_back(transform);

        auto& stats = _sceneData->Stats;
        stats.DeferredSubmits += 1;
        stats.Vertices += mesh->VertexCount();
        stats.Triangles += mesh->IndexCount() / 3;
    }


    void Renderer::beginFrame() {
        auto& ctx = Context::Instance();
        if(ctx.IsImGuiEnabled()){
            auto window = ctx.Get<RenderContext>()->GetWindow();
            auto renderSize = int2(window->GetWidth(), window->GetHeight());

            if(renderSize != _currentImGuiFramebufferSize){
                resize(renderSize);
                _currentImGuiFramebufferSize = renderSize;
            }

            glBindTexture(GL_TEXTURE_2D, _colorTex);
            glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);
            glViewport(0, 0, renderSize.x, renderSize.y);
        }

        _api->SetClearColor({0.0f, 0.0f, 0.0f, 1.0f});
        _api->Clear();
    }

    void Renderer::endFrame() {
        auto& ctx = Context::Instance();
        if(ctx.IsImGuiEnabled())
        {
            auto window = ctx.Get<RenderContext>()->GetWindow();
            glViewport(0, 0, window->GetWidth(), window->GetHeight());

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            _api->SetClearColor({0.0f, 0.0f, 0.0f, 1.0f});
            _api->Clear();
        }
    }


    void Renderer::initTex(int2 initialTextureSize) {

        glGenFramebuffers(1, &_frameBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, _frameBuffer);

        glGenTextures(1, &_colorTex);
        glBindTexture(GL_TEXTURE_2D, _colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, initialTextureSize.x, initialTextureSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

// attach it to currently bound framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorTex, 0);

        glGenTextures(1, &_depthTex);
        glBindTexture(GL_TEXTURE_2D, _depthTex);
        glTexImage2D(GL_TEXTURE_2D, 0,  GL_DEPTH_COMPONENT, initialTextureSize.x, initialTextureSize.y, 0,   GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);


        glFramebufferTexture2D(GL_FRAMEBUFFER, 	GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _depthTex, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    }

    void Renderer::resize(int2 newTextureSize){
        glBindTexture(GL_TEXTURE_2D, _colorTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, newTextureSize.x, newTextureSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

        glBindTexture(GL_TEXTURE_2D, _depthTex);
        glTexImage2D(GL_TEXTURE_2D, 0,  GL_DEPTH_COMPONENT, newTextureSize.x, newTextureSize.y, 0,   GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Renderer::destroyTex() {
        glDeleteTextures(1, &_colorTex);
        glDeleteTextures(1, &_depthTex);
        glDeleteFramebuffers(1, &_frameBuffer);
    }

}
