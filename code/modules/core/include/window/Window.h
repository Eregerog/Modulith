//
// Created by Daniel on 19/10/2019.
//

#pragma once

#include "CoreModule.h"
#include "input/InputState.h"

namespace modulith {

    enum CORE_API WindowType {
        Default, // The window will neither be maximized or fullscreen
        Maximized, // The window will be maximized
        Fullscreen // The window will be in fullscreen mode
    };

    /**
     * This struct contains the information to create a window
     */
    struct CORE_API WindowConfig {
        /**
         * The title of the window
         */
        std::string Title;
        /**
         * The starting width of the window
         */
        int Width;
        /**
         * The starting height of the window
         */
        int Height;

        /**
         * The type of the window when it is created
         */
        WindowType Type;

        WindowConfig(const std::string& title, int width, int height, WindowType type) : Title(title), Width(width),
                                                                                         Height(height), Type(type) {}
    };

    /**
     * This class represents an application window in which this application can render graphics
     * and from which this application will receive input
     */
    class CORE_API Window {

    public:
        /**
         * Creates and returns a Window from the given config
         */
        static std::unique_ptr<Window> Create(WindowConfig config);

        Window() = default;

        virtual ~Window() = default;

        /**
         * This method is called after the window has been created and before the first frame in which it is used.
         * Should be used for initializations that were not done in the constructor
         */
        virtual void OnInitialize() = 0;

        /**
         * This method is called at the beginning of every frame
         */
        virtual void OnPreUpdate() = 0;

        /**
         * This method is called at the end of every frame
         */
        virtual void OnPostUpdate() = 0;

        /**
         * This method is called when the window is no longer used.
         * Should be used for shutdown logic that needs to happen immediately and cannot wait for the destructor.
         */
        virtual void OnShutdown() = 0;

        /**
         * @return An input state object for the current frame.
         * It is expected that the same input object is returned
         * when called multiple times during the same frame.
         */
        virtual InputState& GetCurrentInputState() = 0;

        /**
         * Toggles whether the cursor is visible, determined by the parameter
         */
        virtual void ToggleCursorVisibility(bool enabled) = 0;

        /**
         * @return Whether the cursor is currently visible
         */
        virtual bool IsCursorVisible() = 0;

        /**
         * Allows the cursor position to automatically be set to the given position
         * @param position The new cursor position, given in screen space coordinates
         */
        virtual void SetCursorPosition(int2 position) = 0;

        /**
         * @return Returns true if the window's size has changed during this frame
         */
        virtual bool HasWindowSizeChanged() = 0;

        /**
         * This method should initialize imgui and register it to draw in this window
         */
        virtual void InitImGui() = 0;

        /**
         * @return Returns the current width of the window
         */
        [[nodiscard]] virtual int GetWidth() const = 0;

        /**
         * @return Returns the current height of the window
         */
        [[nodiscard]] virtual int GetHeight() const = 0;

        /**
         * @return The current width and height of the window
         */
        [[nodiscard]] int2 GetSize() const { return int2(GetWidth(), GetHeight()); }

        /**
         * @return Returns true as long as the window has not been closed.
         * Should return false as soon as the window is closed by the user.
         */
        virtual bool IsRunning() = 0;

        virtual void Quit() = 0;

        [[nodiscard]] virtual WindowType GetWindowType() const = 0;

        virtual void SetWindowType(WindowType newType) = 0;
    };

};
