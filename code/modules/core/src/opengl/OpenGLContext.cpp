/**
 * \brief
 * \author Daniel Götz
 */

#include "OpenGLContext.h"
#include "glad/glad.h"

namespace modulith{

    OpenGLContext::OpenGLContext(GLFWwindow* windowHandle) : _windowHandle(windowHandle) {
        CoreAssert(windowHandle, "Window Handle mustn't be null!")
    }

    void OpenGLContext::Init() {
        glfwMakeContextCurrent(_windowHandle);
        int status = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
        CoreAssert(status, "Failed to initialize Glad!")

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        CoreLogInfo("OpenGL Renderer")
        CoreLogInfo("   Vendor: {}", glGetString(GL_VENDOR))
        CoreLogInfo("   Renderer: {}", glGetString(GL_RENDERER))
        CoreLogInfo("   Version: {}", glGetString(GL_VERSION))
    }

    void OpenGLContext::SwapBuffers() {
        glfwSwapBuffers(_windowHandle);
    }

    void OpenGLContext::ResizeViewport(uint32_t width, uint32_t height) {
        glViewport(0,0,width,height);
    }
}
