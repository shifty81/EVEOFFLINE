#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <memory>

namespace atlas {

/**
 * G-Buffer for deferred rendering
 * 
 * G-Buffer Layout:
 * - Texture 0 (RGB16F): Position (world space)
 * - Texture 1 (RGB16F): Normal (world space)
 * - Texture 2 (RGBA): Albedo (RGB) + Specular (A)
 * - Depth Buffer: Depth/Stencil
 * 
 * Deferred rendering separates geometry from lighting:
 * 1. Geometry Pass: Render scene to G-Buffer
 * 2. Lighting Pass: Use G-Buffer to calculate lighting
 * 
 * Benefits:
 * - Efficient with many lights (O(lights) instead of O(lights * objects))
 * - Easier to add post-processing effects
 * - Consistent lighting across all objects
 */
class GBuffer {
public:
    /**
     * Construct G-Buffer with specified resolution
     * @param width Width in pixels
     * @param height Height in pixels
     */
    GBuffer(unsigned int width, unsigned int height);
    
    /**
     * Destructor - cleans up OpenGL resources
     */
    ~GBuffer();
    
    // Prevent copying
    GBuffer(const GBuffer&) = delete;
    GBuffer& operator=(const GBuffer&) = delete;
    
    /**
     * Initialize G-Buffer resources
     * @return true if successful
     */
    bool initialize();
    
    /**
     * Bind G-Buffer for geometry pass (write)
     * All subsequent draw calls will write to G-Buffer
     */
    void bindForGeometryPass();
    
    /**
     * Bind G-Buffer textures for lighting pass (read)
     * Textures are bound to texture units 0-2
     * @param positionUnit Texture unit for position buffer
     * @param normalUnit Texture unit for normal buffer
     * @param albedoSpecUnit Texture unit for albedo+spec buffer
     */
    void bindForLightingPass(
        unsigned int positionUnit = 0,
        unsigned int normalUnit = 1,
        unsigned int albedoSpecUnit = 2
    );
    
    /**
     * Unbind framebuffer (return to default)
     */
    void unbind();
    
    /**
     * Resize G-Buffer
     * @param width New width in pixels
     * @param height New height in pixels
     */
    void resize(unsigned int width, unsigned int height);
    
    /**
     * Get G-Buffer width
     */
    unsigned int getWidth() const { return m_width; }
    
    /**
     * Get G-Buffer height
     */
    unsigned int getHeight() const { return m_height; }
    
    /**
     * Get framebuffer object ID
     */
    GLuint getFramebuffer() const { return m_fbo; }
    
    /**
     * Get position texture ID
     */
    GLuint getPositionTexture() const { return m_positionTexture; }
    
    /**
     * Get normal texture ID
     */
    GLuint getNormalTexture() const { return m_normalTexture; }
    
    /**
     * Get albedo+specular texture ID
     */
    GLuint getAlbedoSpecTexture() const { return m_albedoSpecTexture; }
    
    /**
     * Check if G-Buffer is valid
     */
    bool isValid() const { return m_initialized; }

private:
    /**
     * Create framebuffer and textures
     */
    bool createFramebuffer();
    
    /**
     * Cleanup OpenGL resources
     */
    void cleanup();
    
    unsigned int m_width;
    unsigned int m_height;
    
    GLuint m_fbo;                    // Framebuffer object
    GLuint m_positionTexture;        // World position (RGB16F)
    GLuint m_normalTexture;          // World normal (RGB16F)
    GLuint m_albedoSpecTexture;      // Albedo (RGB) + Specular (A)
    GLuint m_depthRenderbuffer;      // Depth + stencil buffer
    
    bool m_initialized;
};

} // namespace atlas
