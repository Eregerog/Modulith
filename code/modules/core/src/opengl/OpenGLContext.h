/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include "CoreModule.h"
#include <GLFW/glfw3.h>
#include <renderer/GraphicsContext.h>

namespace modulith{

    class OpenGLContext : public GraphicsContext {
    public:
        static void InitForModule();

        explicit OpenGLContext(GLFWwindow* windowHandle);

        void Init() override;

        void SwapBuffers() override;

        void ResizeViewport(uint32_t width, uint32_t height) override;

    private:
        GLFWwindow* _windowHandle;
    };

}
