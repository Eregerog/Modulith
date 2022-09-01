//
// Created by Daniel on 19/10/2019.
//

#pragma once

#include "CoreModule.h"
#include <window/Window.h>
#include <renderer/GraphicsContext.h>
#include <Input/InputState.h>

namespace modulith{

    class WindowsWindow : public Window {
    public:
        explicit WindowsWindow(WindowConfig config);

        ~WindowsWindow() override;

        void OnInitialize() override;

        void OnPreUpdate() override;
        void OnPostUpdate() override;

        void OnShutdown() override;

        InputState& GetCurrentInputState() override;
        void ToggleCursorVisibility(bool enabled) override;

        bool IsCursorVisible() override;

        void InitImGui() override;

        bool IsRunning() override {return _data.Running; }

        void Quit() override;

        bool HasWindowSizeChanged() override;

        [[nodiscard]] int GetWidth() const override;

        [[nodiscard]] int GetHeight() const override;

        void SetCursorPosition(int2 position) override;

        WindowType GetWindowType() const override;

        void SetWindowType(WindowType newType) override;

    private:
        struct WindowData {
            std::string Title;
            int Width, Height;
            WindowType Type;

            bool SizeHasChanged = false;
            bool DoVsync = false;
            bool Running = true;

            float2 ScrollDelta{};
            std::bitset<350> NextKeysDown = std::bitset<350>();
            std::bitset<8> NextMouseKeysDown = std::bitset<8>();
        };

        bool _windowSizeHasChanged = false;
        bool _cursorDisabled = false;

        InputState _currentInputState;
        WindowData _data;

        GLFWwindow* _window{};
        owned<GraphicsContext> _context;
    };



}
