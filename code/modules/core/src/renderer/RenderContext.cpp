/**
 * \brief
 * \author Daniel Götz
 */

#include "renderer/RenderContext.h"
#include "Context.h"

// Headers below cannot be in a "public" header, since this header is not public itself
#include <opengl/OpenGLRendererAPI.h>
#include <imgui/ImGuiWrapper.h>

namespace modulith{

    void RenderContext::OnInitialize() {
        _imguiWindow->OnInitialize();
        InitializeImGui(*_mainWindow);
        _renderer->initialize();
    }

    void RenderContext::OnPreUpdate() {

        _imguiWindow->OnPreUpdate();

        if(Input().IsKeyPressed(KEY_F11)){
            if(_currentWindow->GetWindowType() == WindowType::Fullscreen)
                _currentWindow->SetWindowType(WindowType::Default);
            else if(_currentWindow->GetWindowType() == WindowType::Default)
                _currentWindow->SetWindowType(WindowType::Maximized);
            else _currentWindow->SetWindowType(WindowType::Fullscreen);
        }
    }

    void RenderContext::OnBeforeUpdate() {
        auto& ctx = Context::Instance();
        if (Input().IsKeyPressed(KEY_F3)) {
            ctx.SetImGuiEnabled(!ctx.IsImGuiEnabled());
        }
        _currentWindow = ctx.IsImGuiEnabled() ? ref<Window>(&_imguiWindow) : ref(&_mainWindow);
        _renderer->beginFrame();
    }

    void RenderContext::OnUpdate(float deltaTime) {}

    void RenderContext::OnAfterUpdate() {
        _currentWindow = ref(&_mainWindow);
        _renderer->endFrame();
        if(!GetWindow()->IsRunning())
            Context::Instance().RequestShutdown();
    }

    void RenderContext::OnBeforeImGui(bool renderingToImguiWindow) {
        BeginImGuiRender(*_mainWindow, renderingToImguiWindow);
    }

    void RenderContext::OnImGui(float deltaTime, bool renderingToImguiWindow) {

        if (renderingToImguiWindow){

            ImGui::Begin("Game", nullptr, ImGuiWindowFlags_NoCollapse);
            {
                auto imGuiSize = ImGui::GetWindowSize();
                auto size = int2(imGuiSize.x  - 16, imGuiSize.y - 36);
                _imguiWindow->setSize(size);

                auto imguiCursorPos = ImGui::GetCursorScreenPos();
                _imguiWindow->setOffset(int2(imguiCursorPos.x, imguiCursorPos.y));

                _imguiWindow->setIsFocused(ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows));

                ImGui::GetWindowDrawList()->AddImage(
                    (void*) _renderer->_colorTex,
                    ImVec2(imguiCursorPos.x, imguiCursorPos.y),
                    ImVec2(imguiCursorPos.x + size.x, imguiCursorPos.y + size.y), ImVec2(0, 1),
                    ImVec2(1, 0));
            }
            ImGui::End();
        }
    }

    void RenderContext::OnAfterImGui(bool renderingToImguiWindow) {
        EndImGuiRender(*_mainWindow);
    }

    void RenderContext::OnPostUpdate() {
        _imguiWindow->OnPostUpdate();
    }

    void RenderContext::OnShutdown() {
        _imguiWindow->OnShutdown();
        _renderer->shutdown();
        _shaderLibrary.reset();
        ShutdownImGui(*_mainWindow);
    }

    void RenderContext::OnBeforeUnloadModules(const std::vector<Module>& modules) {
        _shaderLibrary.reset();
        _shaderLibrary = std::make_unique<ShaderLibrary>();
    }

    RenderContext::RenderContext() : Subcontext("Render Context"),
                                     _shaderLibrary(std::make_unique<ShaderLibrary>()),
                                     _renderer(std::make_unique<Renderer>(std::make_shared<OpenGLRendererAPI>())),
                                     _mainWindow(Window::Create({"Modulith", 1280, 720, WindowType::Default })),
                                     _currentWindow(ref(&_mainWindow)){

        _imguiWindow = std::make_unique<SubWindow>(_currentWindow);
    }


}