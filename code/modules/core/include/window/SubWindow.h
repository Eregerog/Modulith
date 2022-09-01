/*
 * \brief
 * \author Daniel Götz
 */

# pragma once

#include "CoreModule.h"
#include "Window.h"

namespace modulith {

    class RenderContext;

    /**
     * A implementation of a window that is nested inside another
     */
    class CORE_API SubWindow : public Window {
        friend class RenderContext;

    public:
        /**
         * Creates a new subwindow
         * @param containedIn The window the subwindow is contained in
         */
        explicit SubWindow(ref<Window> containedIn) :
            _windowSize(containedIn->GetSize()), _windowOffset(int2(0,0)), _containedIn(std::move(containedIn)) {}

        void OnInitialize() override { _containedIn->OnInitialize(); }

        void OnPreUpdate() override {
            _containedIn->OnPreUpdate();

            if(_isFocused){
                _currentInputState = _containedIn->GetCurrentInputState().ForSubWindow(
                    float2(_containedIn->GetWidth(), _containedIn->GetHeight()),
                    _windowOffset,
                    _windowSize
                );
            }else{
                _currentInputState = InputState();
            }
        }

        void OnPostUpdate() override {
            _containedIn->OnPostUpdate();
            _windowSizeChangedThisFrame = false;
        }

        void OnShutdown() override { _containedIn->OnShutdown(); }

        void ToggleCursorVisibility(bool enabled) override { _containedIn->ToggleCursorVisibility(enabled); }

        void InitImGui() override { _containedIn->InitImGui(); }

        bool IsRunning() override { return _containedIn->IsRunning(); }

        void Quit() override {
            _containedIn->Quit();
        }

        [[nodiscard]] int GetWidth() const override { return _windowSize.x; }

        [[nodiscard]] int GetHeight() const override { return _windowSize.y; }

        /**
         * @return Returns whether the subwindow is currently focused
         */
        [[nodiscard]] bool IsFocused() const { return _isFocused; }

        InputState& GetCurrentInputState() override { return _currentInputState; }

        bool HasWindowSizeChanged() override { return _windowSizeChangedThisFrame; }

        bool IsCursorVisible() override {
            return _containedIn->IsCursorVisible();
        }

        void SetCursorPosition(int2 position) override {
            _containedIn->SetCursorPosition(position);
        }

        [[nodiscard]] WindowType GetWindowType() const override {
            return _containedIn->GetWindowType();
        }

        void SetWindowType(WindowType newType) override {
            _containedIn->SetWindowType(newType);
        }

    private:
        void setSize(int2 size) {
            _windowSizeChangedThisFrame = true;
            _windowSize = size;
        }

        void setOffset(int2 offset) {
            _windowSizeChangedThisFrame = true;
            _windowOffset = offset;
        }

        void setIsFocused(bool focused) {
            _isFocused = focused;
        }

        bool _windowSizeChangedThisFrame = false;
        InputState _currentInputState{};

        bool _isFocused = false;
        int2 _windowSize;
        int2 _windowOffset;
        ref<Window> _containedIn;
    };
}
