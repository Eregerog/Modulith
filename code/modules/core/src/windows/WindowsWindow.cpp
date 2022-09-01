//
// Created by Daniel on 19/10/2019.
//

#include <GLFW/glfw3.h>

#include <opengl/OpenGLContext.h>
#include <imgui.h>
#include <examples/imgui_impl_glfw.h>
#include "WindowsWindow.h"

namespace modulith{

#define GetWindowData(x) *(WindowData*)glfwGetWindowUserPointer(x);

    std::unique_ptr<Window> Window::Create(WindowConfig config) {
        return std::make_unique<WindowsWindow>(std::move(config));
    }

    static bool GLFWInitialized = false;

    WindowsWindow::WindowsWindow(WindowConfig config) : _currentInputState(InputState()) {
        _data.Width = config.Width;
        _data.Height = config.Height;
        _data.Type = config.Type;
    }

    WindowsWindow::~WindowsWindow() = default;

    void WindowsWindow::OnInitialize() {
        if (!GLFWInitialized) {
            int success = glfwInit();
            CoreAssert(success, "GLFW failed to initialize!");

            GLFWInitialized = true;
        }

        _window = glfwCreateWindow((int) _data.Width, (int) _data.Height, _data.Title.c_str(), nullptr, nullptr);

        SetWindowType(_data.Type);

        // TODO hardcoded
        _context = std::make_unique<OpenGLContext>(_window);
        _context->Init();

        // TODO also extract to init function
        glfwSetInputMode(_window, GLFW_STICKY_KEYS, GLFW_TRUE);

        glfwSetWindowUserPointer(_window, &_data);

        // TODO extract to set vsync function
        glfwSwapInterval(0);

        glfwSetKeyCallback(
            _window, [](auto window, int key, int scanCode, int action, int mods) {
                auto& data = GetWindowData(window);

                switch (action) {
                    case GLFW_PRESS:
                        data.NextKeysDown.set(key);
                        break;
                    case GLFW_RELEASE:
                        data.NextKeysDown.reset(key);
                        break;
                }
            }
        );

        glfwSetMouseButtonCallback(
            _window, [](auto window, int key, int action, int mods) {
                auto& data = GetWindowData(window);

                switch (action) {
                    case GLFW_PRESS:
                        data.NextMouseKeysDown.set(key);
                        break;
                    case GLFW_RELEASE:
                        data.NextMouseKeysDown.reset(key);
                        break;
                }
            }
        );

        glfwSetScrollCallback(
            _window, [](auto window, double scrollX, double scrollY) {
                auto& data = GetWindowData(window);
                data.ScrollDelta += float2{scrollX, scrollY};
            }
        );

        glfwSetWindowSizeCallback(
            _window, [](auto window, int width, int height) {
                auto& data = GetWindowData(window);
                CoreLogInfo("Resizing window to {} {}", width, height)
                data.Width = width;
                data.Height = height;
                data.SizeHasChanged = true;
            }
        );

        glfwSetWindowCloseCallback(
            _window, [](auto window) {
                auto& data = GetWindowData(window);

                data.Running = false;
            }
        );
    }

    void WindowsWindow::OnPreUpdate() {
        auto& data = GetWindowData(_window);
        double xPos, yPos;

        glfwGetCursorPos(_window, &xPos, &yPos);
        _currentInputState = InputState(
            _currentInputState,
            {xPos, yPos}, // mouse Pos
            {data.ScrollDelta.x, data.ScrollDelta.y},
            data.NextKeysDown,
            data.NextMouseKeysDown,
            xPos >= 0 && xPos <= GetWidth() && yPos >= 0 && yPos <= GetHeight() // isMouseInsideWindow
        );
        data.ScrollDelta = {0.0f, 0.0f}; // reset the scroll delta, but not the key states
        _windowSizeHasChanged = data.SizeHasChanged;
        if(data.SizeHasChanged){
            _context->ResizeViewport(data.Width, data.Height);
            data.SizeHasChanged = false;
        }
    }

    void WindowsWindow::OnPostUpdate() {
        glfwPollEvents();
        _context->SwapBuffers();

    }

    void WindowsWindow::OnShutdown() {
        // TODO extract to global glfw handler
        glfwTerminate();
        GLFWInitialized = false;
    }

    InputState& WindowsWindow::GetCurrentInputState() {
        return _currentInputState;
    }

    void WindowsWindow::InitImGui() {
        ImGui_ImplGlfw_InitForOpenGL(_window, true);
    }

    void WindowsWindow::ToggleCursorVisibility(bool enabled) {
        _cursorDisabled = !enabled;
        glfwSetInputMode(_window, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }

    bool WindowsWindow::HasWindowSizeChanged() {
        return _windowSizeHasChanged;
    }

    int WindowsWindow::GetWidth() const {
        return _data.Width;
    }

    int WindowsWindow::GetHeight() const {
        return _data.Height;
    }

    void WindowsWindow::SetCursorPosition(int2 position) {
        glfwSetCursorPos(_window, position.x, position.y);
    }

    bool WindowsWindow::IsCursorVisible() {
        return !_cursorDisabled;
    }

    WindowType WindowsWindow::GetWindowType() const {
        return _data.Type;
    }

    void WindowsWindow::SetWindowType(WindowType newType) {
        switch(newType){
            case WindowType::Default:
                glfwSetWindowMonitor(_window, nullptr, 300, 300, _data.Width, _data.Height, GLFW_DONT_CARE);
                glfwRestoreWindow(_window);
                break;
            case WindowType::Maximized:
                glfwSetWindowMonitor(_window, nullptr, 0, 0, _data.Width, _data.Height, GLFW_DONT_CARE);
                glfwMaximizeWindow(_window);
                break;
            case WindowType::Fullscreen:
                glfwMaximizeWindow(_window);
                glfwSetWindowMonitor(_window, glfwGetPrimaryMonitor(), 0, 0, _data.Width, _data.Height, GLFW_DONT_CARE);
                break;
        }
        _data.Type = newType;
    }

    void WindowsWindow::Quit() {
        _data.Running = false;
    }
}
