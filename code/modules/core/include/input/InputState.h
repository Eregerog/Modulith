/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <CoreModule.h>

namespace modulith {

    using KeyCode = uint32_t;

    class InputState;

    /**
     * This class represents the state of a number of keys in a particular moment
     * @tparam size The amount of keys represented
     */
    template<int size>
    class CORE_API KeyState {

    public:
        /**
         * Creates the default key state, where no keys are pressed
         */
        KeyState() = default;

        /**
         * Creates a key state from the previous key state and the keys that are currently active
         */
        KeyState(const KeyState<size>& previous, const std::bitset<size>& currentKeys) : _currentKeys(currentKeys) {
            auto changedKeys = previous._currentKeys ^_currentKeys;
            _pressedKeys = _currentKeys & changedKeys;
            _releasedKeys = (~_currentKeys) & changedKeys;
        }

        /**
         * @return Returns whether the key of the given keyCode is currently pressed
         */
        [[nodiscard]] bool IsKeyDown(KeyCode keyCode) const {
            CoreAssert(keyCode < size, "The key code {0} is invalid, since it exceeds the size {1} of this KeyState",
                keyCode, size)
            return _currentKeys.test(keyCode);
        }

        /**
         * @return Returns whether the key of the given keyCode has been started to be pressed in this moment
         */
        [[nodiscard]] bool IsKeyPressed(KeyCode keyCode) const {
            CoreAssert(keyCode < size, "The key code {0} is invalid, since it exceeds the size {1} of this KeyState",
                keyCode, size)
            return _pressedKeys.test(keyCode);
        }

        /**
         * @return Returns whether the key of the given keyCode has been released in this moment
         */
        [[nodiscard]] bool IsKeyReleased(KeyCode keyCode) const {
            CoreAssert(keyCode < size, "The key code {0} is invalid, since it exceeds the size {1} of this KeyState",
                keyCode, size)
            return _releasedKeys.test(keyCode);
        }

    private:

        std::bitset<size> _currentKeys{};
        std::bitset<size> _pressedKeys{};
        std::bitset<size> _releasedKeys{};

    };

    /**
     * This class represents the state of all user input in the current moment
     */
    class CORE_API InputState {
    public:
        static const InputState& Empty;

        /**
         * Creates an empty (default) input state
         */
        explicit InputState() = default;

        /**
         * Creates an input state from the previous input state and information about the current state
         * @param previous The previous input state
         * @param mousePosition The current mouse position
         * @param scrollDelta The amount that was scrolled since the last input state
         * @param keysDown The keyboard keys that are currently pressed
         * @param mouseKeysDown The mouse keys that are currently pressed
         * @param mouseInsideWindow If the mouse is currently inside the window
         */
        InputState(
            const InputState& previous, const float2& mousePosition, const float2& scrollDelta,
            const std::bitset<350>& keysDown, const std::bitset<8>& mouseKeysDown, const bool mouseInsideWindow
        )
            : _mousePosition(mousePosition), _scrollDelta(scrollDelta),
              _keyboardState(KeyState<350>(previous._keyboardState, keysDown)),
              _mouseState(KeyState<8>(previous._mouseState, mouseKeysDown)),
              _mouseInsideWindow(mouseInsideWindow) {
            _mouseDelta = _mousePosition - previous._mousePosition;
        }

    private:

        InputState(
            const float2& mousePosition, const float2& mouseDelta, const float2& scrollDelta,
            const KeyState<350>& keyboardState, const KeyState<8>& mouseState, const bool mouseInsideWindow
        ) : _mousePosition(mousePosition), _mouseDelta(mouseDelta), _scrollDelta(scrollDelta),
            _keyboardState(keyboardState), _mouseState(mouseState), _mouseInsideWindow(mouseInsideWindow) {}

    public:
        /**
         * @return Returns whether the keyboard key of the given code is currently pressed
         */
        [[nodiscard]] bool IsKeyDown(KeyCode keyCode) const {
            return _keyboardState.IsKeyDown(keyCode);
        }

        /**
         * @return Returns whether the keyboard key of the given code has been started to be pressed
         */
        [[nodiscard]] bool IsKeyPressed(KeyCode keyCode) const {
            return _keyboardState.IsKeyPressed(keyCode);
        }

        /**
         * @return Returns whether the keyboard key of the given code has been started to be released
         */
        [[nodiscard]] bool IsKeyReleased(KeyCode keyCode) const {
            return _keyboardState.IsKeyReleased(keyCode);
        }

        /**
         * @return Returns whether the mouse key of the given code is currently pressed
         */
        [[nodiscard]] bool IsMouseKeyDown(KeyCode keyCode) const {
            return _mouseState.IsKeyDown(keyCode);
        }

        /**
         * @return Returns whether the mouse key of the given code has been started to be pressed
         */
        [[nodiscard]] bool IsMouseKeyPressed(KeyCode keyCode) const {
            return _mouseState.IsKeyPressed(keyCode);
        }

        /**
         * @return Returns whether the mouse key of the given code has been started to be released
         */
        [[nodiscard]] bool IsMouseKeyReleased(KeyCode keyCode) const {
            return _mouseState.IsKeyReleased(keyCode);
        }

        /**
         * @return Returns whether the mouse position is currently inside the window
         */
        [[nodiscard]] bool IsMouseInsideWindow() const { return _mouseInsideWindow; }

        /**
         * @return Returns the current mouse position
         */
        [[nodiscard]] float2 GetMousePosition() const { return _mousePosition; }

        /**
         * @return Returns the amount the mouse position changed since the last input state
         */
        [[nodiscard]] float2 GetMouseDelta() const { return _mouseDelta; }

        /**
         * @return Returns the amount the scroll wheel changed since the last input state
         */
        [[nodiscard]] float2 GetScrollDelta() const { return _scrollDelta; }

        /**
         * Constructs a new input state by translating this input state's mouse position
         * into the viewport of a subwindow
         * @param mainWindowSize The main window's size
         * @param subWindowOffset The offset of the subwindow relative to the main window
         * @param subWindowSize The subwindow's size. This has to be smaller than the main window
         * @return A new input state suited for the subwindow
         */
        [[nodiscard]] InputState
        ForSubWindow(float2 mainWindowSize, float2 subWindowOffset, float2 subWindowSize) const {
            auto sizeDelta = subWindowSize / mainWindowSize;

            int2 newPosition = _mousePosition - subWindowOffset;
            auto isInsideWindow = newPosition.x >= 0 && newPosition.x <= subWindowSize.x && newPosition.y >= 0
                                  && newPosition.y <= subWindowSize.y;

            return InputState(
                newPosition, sizeDelta * _mouseDelta, _scrollDelta, _keyboardState, _mouseState, isInsideWindow
            );
        }

    private:

        bool _mouseInsideWindow = false;
        float2 _mousePosition{};
        float2 _mouseDelta{};

        float2 _scrollDelta{};

        KeyState<350> _keyboardState{};
        KeyState<8> _mouseState{};
    };

};
