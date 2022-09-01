/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <CoreModule.h>
#include <renderer/primitives/Shader.h>
#include <window/SubWindow.h>

#include <utility>
#include "Subcontext.h"
#include "Renderer.h"

namespace modulith {

    class CORE_API RenderContext : public Subcontext {
    public:
        RenderContext();

        /**
         * @return Returns the window that is currently rendered in
         * @see Window
         */
        ref<Window> GetWindow() { return _currentWindow; }

        /**
         * @return Returns the input state of the current frame
         * @see InputState
         */
        [[nodiscard]] const InputState& Input() const { return _currentWindow->GetCurrentInputState(); }

        /**
         * @return Returns the application's shader library
         * @see ShaderLibrary
         */
        ref<ShaderLibrary> Shaders() { return ref(&_shaderLibrary); }

        /**
         * @return Returns the application's renderer
         */
        ref<Renderer> GetRenderer() { return ref(&_renderer); }

        shared<RendererAPI> RendererAPI() {return _renderer->GetAPI(); }

        void OnInitialize() override;

        void OnPreUpdate() override;

        void OnBeforeUpdate() override;

        void OnAfterUpdate() override;

        void OnBeforeImGui(bool renderingToImguiWindow) override;

        void OnAfterImGui(bool renderingToImguiWindow) override;

        void OnUpdate(float deltaTime) override;

        void OnImGui(float deltaTime, bool renderingToImguiWindow) override;

        void OnPostUpdate() override;

        void OnShutdown() override;


        void OnBeforeUnloadModules(const std::vector<Module>& modules) override;

    private:

        owned<Renderer> _renderer;
        owned<ShaderLibrary> _shaderLibrary;

        ref<Window> _currentWindow;
        owned<Window> _mainWindow;
        owned<SubWindow> _imguiWindow;
    };

}