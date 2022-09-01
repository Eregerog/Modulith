/**
 * \brief
 * \author Daniel Götz
 */


#pragma once

#include "CoreModule.h"
#include <renderer/primitives/Texture.h>

namespace modulith{

    class OpenGLFrameBuffer;

    class OpenGLTexture2D : public Texture2D {
        friend class OpenGLFrameBuffer;
    public:
        explicit OpenGLTexture2D(const fs::path& filePath);
        OpenGLTexture2D(int width, int height);

        ~OpenGLTexture2D() override;

        [[nodiscard]] uint32_t GetWidth() const override { return _width; }

        [[nodiscard]] uint32_t GetHeight() const override { return _height; }

        void Bind(uint32_t slot) const override;

    private:
        uint32_t _width;
        uint32_t _height;
        fs::path _path;
        uint32_t _rendererId{};
    };


}
