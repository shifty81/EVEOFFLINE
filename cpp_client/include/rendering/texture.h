#pragma once

#include <string>
#include <unordered_map>
#include <memory>

namespace atlas {

/**
 * Texture class for 2D textures
 */
class Texture {
public:
    Texture();
    ~Texture();
    
    // Prevent copying
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    /**
     * Load texture from file
     * Supports: PNG, JPG, TGA, BMP, PSD, GIF, HDR, PIC, PNM
     */
    bool loadFromFile(const std::string& path);
    
    /**
     * Create a solid color texture
     * @param color Color in RGBA format (0-255)
     * @param width Texture width
     * @param height Texture height
     */
    bool createSolidColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255, 
                         int width = 1, int height = 1);

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
    
    /**
     * Get texture dimensions
     */
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getChannels() const { return m_channels; }
    
    /**
     * Check if texture is valid
     */
    bool isValid() const { return m_textureID != 0; }

private:
    unsigned int m_textureID;
    int m_width;
    int m_height;
    int m_channels;
};

/**
 * Texture cache for managing loaded textures
 * Prevents duplicate loading and manages texture lifetime
 */
class TextureCache {
public:
    TextureCache() = default;
    ~TextureCache() = default;
    
    /**
     * Get or load a texture
     * @param path Path to texture file
     * @return Shared pointer to texture, or nullptr if failed
     */
    std::shared_ptr<Texture> get(const std::string& path);
    
    /**
     * Check if texture is cached
     */
    bool has(const std::string& path) const;
    
    /**
     * Remove texture from cache
     */
    void remove(const std::string& path);
    
    /**
     * Clear entire cache
     */
    void clear();
    
    /**
     * Get cache statistics
     */
    size_t getCacheSize() const { return m_cache.size(); }

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_cache;
};

} // namespace atlas
