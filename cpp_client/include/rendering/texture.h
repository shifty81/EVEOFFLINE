#pragma once

#include <string>

namespace eve {

/**
 * Texture class for 2D textures
 */
class Texture {
public:
    Texture();
    ~Texture();

    /**
     * Load texture from file
     */
    bool loadFromFile(const std::string& path);

    /**
     * Bind texture to specified unit
     */
    void bind(unsigned int unit = 0) const;

    /**
     * Unbind texture
     */
    void unbind() const;

    /**
     * Get texture ID
     */
    unsigned int getID() const { return m_textureID; }

private:
    unsigned int m_textureID;
    int m_width;
    int m_height;
    int m_channels;
};

} // namespace eve
