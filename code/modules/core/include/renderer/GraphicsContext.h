/**
 * \brief
 * \author Daniel Götz
 */


#pragma once
#include <CoreModule.h>

namespace modulith{

    /**
     * A rendering-API independent implementation for a graphic context that contains functions used by the Window for rendering
     */
    class CORE_API GraphicsContext {
    public:

        virtual ~GraphicsContext() = default;

        /**
         * Initializes the graphics context
         */
        virtual void Init() = 0;

        /**
         * Swaps the buffers of the graphics context
         */
        virtual void SwapBuffers() = 0;

        /**
         * Resizes the viewport to the given size
         */
        virtual void ResizeViewport(uint32_t width, uint32_t height) = 0;
    };

}
