#include "rendering/texture.h"
#include <GL/glew.h>
#include <iostream>
#include <vector>

// STB_IMAGE implementation
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
    // If texture already loaded, delete it
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
    
    // Load image data using stb_image
    stbi_set_flip_vertically_on_load(true); // OpenGL expects bottom-left origin
    
    unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, 0);
    
    if (!data) {
        std::cerr << "[Texture] Failed to load texture: " << path << std::endl;
        std::cerr << "[Texture] STB Error: " << stbi_failure_reason() << std::endl;
        return false;
    }
    
    // Determine format based on channels
    GLenum format = GL_RGB;
    GLenum internalFormat = GL_RGB;
    
    switch (m_channels) {
        case 1:
            format = GL_RED;
            internalFormat = GL_R8;
            break;
        case 2:
            format = GL_RG;
            internalFormat = GL_RG8;
            break;
        case 3:
            format = GL_RGB;
            internalFormat = GL_RGB8;
            break;
        case 4:
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
            break;
        default:
            std::cerr << "[Texture] Unsupported channel count: " << m_channels << std::endl;
            stbi_image_free(data);
            return false;
    }
    
    // Generate and configure texture
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
    
    // Generate mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Enable anisotropic filtering if available
    float maxAnisotropy = 1.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
    if (maxAnisotropy > 1.0f) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Free image data
    stbi_image_free(data);
    
    std::cout << "[Texture] Loaded: " << path << " (" << m_width << "x" << m_height 
              << ", " << m_channels << " channels)" << std::endl;
    
    return true;
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::createSolidColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a,
                               int width, int height) {
    // If texture already loaded, delete it
    if (m_textureID != 0) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
    
    m_width = width;
    m_height = height;
    m_channels = 4;
    
    // Create pixel data
    std::vector<unsigned char> pixels(width * height * 4);
    for (int i = 0; i < width * height; i++) {
        pixels[i * 4 + 0] = r;
        pixels[i * 4 + 1] = g;
        pixels[i * 4 + 2] = b;
        pixels[i * 4 + 3] = a;
    }
    
    // Generate and configure texture
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    
    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    
    // Set texture parameters (no mipmaps for solid color)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    std::cout << "[Texture] Created solid color: (" << (int)r << "," << (int)g << "," 
              << (int)b << "," << (int)a << ")" << std::endl;
    
    return true;
}

// ============================================================================
// TextureCache Implementation
// ============================================================================

std::shared_ptr<Texture> TextureCache::get(const std::string& path) {
    // Check if already cached
    auto it = m_cache.find(path);
    if (it != m_cache.end()) {
        return it->second;
    }
    
    // Load new texture
    auto texture = std::make_shared<Texture>();
    if (!texture->loadFromFile(path)) {
        std::cerr << "[TextureCache] Failed to load texture: " << path << std::endl;
        return nullptr;
    }
    
    // Cache and return
    m_cache[path] = texture;
    return texture;
}

bool TextureCache::has(const std::string& path) const {
    return m_cache.find(path) != m_cache.end();
}

void TextureCache::remove(const std::string& path) {
    m_cache.erase(path);
}

void TextureCache::clear() {
    m_cache.clear();
}

} // namespace eve
