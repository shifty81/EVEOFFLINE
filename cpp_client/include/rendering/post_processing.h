#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Rendering {

/**
 * @brief Framebuffer wrapper for post-processing effects
 * 
 * Manages a framebuffer with color and depth attachments.
 * Supports HDR formats and ping-pong rendering for multi-pass effects.
 */
class PostProcessingBuffer {
public:
    PostProcessingBuffer(unsigned int width, unsigned int height, bool useHDR = true);
    ~PostProcessingBuffer();
    
    // Delete copy operations
    PostProcessingBuffer(const PostProcessingBuffer&) = delete;
    PostProcessingBuffer& operator=(const PostProcessingBuffer&) = delete;
    
    /**
     * @brief Initialize framebuffer and attachments
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Bind framebuffer for rendering
     */
    void bind();
    
    /**
     * @brief Unbind framebuffer (return to default)
     */
    void unbind();
    
    /**
     * @brief Bind color texture to a texture unit
     * @param unit Texture unit (0-31)
     */
    void bindTexture(unsigned int unit);
    
    /**
     * @brief Resize framebuffer
     * @param width New width
     * @param height New height
     */
    void resize(unsigned int width, unsigned int height);
    
    /**
     * @brief Get color texture ID
     */
    GLuint getTexture() const { return m_colorTexture; }
    
    /**
     * @brief Get framebuffer dimensions
     */
    glm::uvec2 getSize() const { return glm::uvec2(m_width, m_height); }

private:
    unsigned int m_width, m_height;
    bool m_useHDR;
    
    GLuint m_fbo;
    GLuint m_colorTexture;
    GLuint m_rbo;  // Renderbuffer for depth
    
    void cleanup();
};

/**
 * @brief Post-processing effects manager
 * 
 * Manages a chain of post-processing effects including:
 * - HDR tone mapping
 * - Bloom
 * - Gamma correction
 */
class PostProcessing {
public:
    PostProcessing(unsigned int width, unsigned int height);
    ~PostProcessing();
    
    // Delete copy operations
    PostProcessing(const PostProcessing&) = delete;
    PostProcessing& operator=(const PostProcessing&) = delete;
    
    /**
     * @brief Initialize post-processing system
     * @return true if initialization successful
     */
    bool initialize();
    
    /**
     * @brief Apply post-processing effects to input texture
     * @param inputTexture Source texture (HDR)
     * @param outputFBO Target framebuffer (0 for screen)
     */
    void process(GLuint inputTexture, GLuint outputFBO = 0);
    
    /**
     * @brief Resize all buffers
     * @param width New width
     * @param height New height
     */
    void resize(unsigned int width, unsigned int height);
    
    // Effect enable/disable
    void setBloomEnabled(bool enabled) { m_bloomEnabled = enabled; }
    void setHDREnabled(bool enabled) { m_hdrEnabled = enabled; }
    
    // Effect parameters
    void setExposure(float exposure) { m_exposure = exposure; }
    void setBloomThreshold(float threshold) { m_bloomThreshold = threshold; }
    void setBloomIntensity(float intensity) { m_bloomIntensity = intensity; }
    void setGamma(float gamma) { m_gamma = gamma; }
    
    // Getters
    bool isBloomEnabled() const { return m_bloomEnabled; }
    bool isHDREnabled() const { return m_hdrEnabled; }
    float getExposure() const { return m_exposure; }
    float getBloomThreshold() const { return m_bloomThreshold; }
    float getBloomIntensity() const { return m_bloomIntensity; }
    float getGamma() const { return m_gamma; }

private:
    unsigned int m_width, m_height;
    
    // Effect toggles
    bool m_bloomEnabled;
    bool m_hdrEnabled;
    
    // Effect parameters
    float m_exposure;
    float m_bloomThreshold;
    float m_bloomIntensity;
    float m_gamma;
    
    // Buffers for multi-pass rendering
    std::unique_ptr<PostProcessingBuffer> m_brightPassBuffer;
    std::vector<std::unique_ptr<PostProcessingBuffer>> m_blurBuffers;  // Ping-pong buffers
    std::vector<std::unique_ptr<PostProcessingBuffer>> m_mipBuffers;   // Downsampled buffers for bloom
    
    // Fullscreen quad
    GLuint m_quadVAO;
    GLuint m_quadVBO;
    
    /**
     * @brief Setup fullscreen quad for post-processing
     */
    void setupQuad();
    
    /**
     * @brief Render fullscreen quad
     */
    void renderQuad();
    
    /**
     * @brief Extract bright pixels for bloom
     * @param inputTexture Source texture
     * @return Bright pass texture
     */
    GLuint brightPass(GLuint inputTexture);
    
    /**
     * @brief Apply Gaussian blur
     * @param inputTexture Source texture
     * @param horizontal True for horizontal pass, false for vertical
     * @return Blurred texture
     */
    GLuint gaussianBlur(GLuint inputTexture, bool horizontal);
    
    /**
     * @brief Downsample texture
     * @param inputTexture Source texture
     * @param targetBuffer Target buffer
     */
    void downsample(GLuint inputTexture, PostProcessingBuffer* targetBuffer);
    
    /**
     * @brief Apply bloom effect
     * @param inputTexture Source texture
     * @return Bloomed texture
     */
    GLuint applyBloom(GLuint inputTexture);
    
    /**
     * @brief Apply HDR tone mapping
     * @param inputTexture HDR texture
     * @param bloomTexture Bloom texture (can be 0)
     * @param outputFBO Target framebuffer
     */
    void toneMap(GLuint inputTexture, GLuint bloomTexture, GLuint outputFBO);
};

} // namespace Rendering
