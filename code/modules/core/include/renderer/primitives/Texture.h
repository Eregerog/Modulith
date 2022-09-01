/**
 * \brief
 * \author Daniel Götz
 */

#pragma once

#include <CoreModule.h>

namespace fs = std::filesystem;

namespace modulith{

    /**
     * A rendering API-independent implementation of a texture
     */
    class CORE_API Texture {
    public:
        virtual ~Texture() = default;

        /**
         * @return Returns the width of this texture
         */
        [[nodiscard]] virtual uint32_t GetWidth() const = 0;

        /**
         * @return Returns the height of this texture
         */
        [[nodiscard]] virtual uint32_t GetHeight() const = 0;

        /**
         * Binds the texture to the given texture slot,
         * where it can be used by shaders
         * @param slot
         */
        virtual void Bind(uint32_t slot) const = 0;
    };

    /**
     * A rendering API-independent implementation of a 2D texture
     */
    class Texture2D : public Texture {

    };
}
