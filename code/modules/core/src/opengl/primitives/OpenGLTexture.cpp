/**
 * \brief
 * \author Daniel Götz
 */


#include "OpenGLTexture.h"
#include "stb_image.h"
#include "glad/glad.h"

namespace modulith{

    OpenGLTexture2D::OpenGLTexture2D(const fs::path& filePath) : _path(filePath) {
        int width, height, channels;
        stbi_set_flip_vertically_on_load(1);
        auto* data = stbi_load(filePath.generic_string().c_str(), &width, &height, &channels, 0);
        CoreAssert(data, "Failed to load image!");
        _width = width;
        _height = height;

        GLenum internalFormat = 0, dataFormat = 0;

        switch (channels) {
            case 4:
                internalFormat = GL_SRGB8_ALPHA8;
                dataFormat = GL_RGBA;
                break;
            case 3:
                internalFormat = GL_SRGB8;
                dataFormat = GL_RGB;
                break;
            default: CoreAssert(false, "Unsupported image channel count!");
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &_rendererId);
        glTextureStorage2D(_rendererId, 1, internalFormat, _width, _height);

        glTextureParameteri(_rendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(_rendererId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTextureSubImage2D(_rendererId, 0, 0, 0, _width, _height, dataFormat, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }


    OpenGLTexture2D::OpenGLTexture2D(int width, int height) : _width(width), _height(height) {
        glGenTextures(1, &_rendererId);
        glBindTexture(GL_TEXTURE_2D, _rendererId);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    OpenGLTexture2D::~OpenGLTexture2D() {
        glDeleteTextures(1, &_rendererId);
    }

    void OpenGLTexture2D::Bind(uint32_t slot) const {
        glBindTextureUnit(slot, _rendererId);
    }


}
