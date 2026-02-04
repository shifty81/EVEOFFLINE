#include "rendering/texture.h"
#include <glad/glad.h>
#include <iostream>

namespace eve {

Texture::Texture()
    : m_textureID(0)
    , m_width(0)
    , m_height(0)
    , m_channels(0)
{
}

Texture::~Texture() {
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
    }
}

bool Texture::loadFromFile(const std::string& path) {
    std::cout << "Texture loading not yet implemented: " << path << std::endl;
    return false;
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

} // namespace eve
