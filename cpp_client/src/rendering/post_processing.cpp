#include "rendering/post_processing.h"
#include "rendering/shader.h"
#include <iostream>
#include <memory>

namespace Rendering {

// Static shader storage
static std::unique_ptr<eve::Shader> g_brightPassShader;
static std::unique_ptr<eve::Shader> g_blurShader;
static std::unique_ptr<eve::Shader> g_downsampleShader;
static std::unique_ptr<eve::Shader> g_upsampleShader;
static std::unique_ptr<eve::Shader> g_toneMapShader;

// =============================================================================
// PostProcessingBuffer Implementation
// =============================================================================

PostProcessingBuffer::PostProcessingBuffer(unsigned int width, unsigned int height, bool useHDR)
    : m_width(width), m_height(height), m_useHDR(useHDR),
      m_fbo(0), m_colorTexture(0), m_rbo(0) {
}

PostProcessingBuffer::~PostProcessingBuffer() {
    cleanup();
}

bool PostProcessingBuffer::initialize() {
    // Generate framebuffer
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    
    // Create color texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    
    // Use HDR format if requested
    if (m_useHDR) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, nullptr);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);
    
    // Create renderbuffer for depth
    glGenRenderbuffers(1, &m_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
    
    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "PostProcessingBuffer: Framebuffer is not complete!" << std::endl;
        cleanup();
        return false;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return true;
}

void PostProcessingBuffer::bind() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PostProcessingBuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessingBuffer::bindTexture(unsigned int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
}

void PostProcessingBuffer::resize(unsigned int width, unsigned int height) {
    if (width == m_width && height == m_height) {
        return;
    }
    
    m_width = width;
    m_height = height;
    
    cleanup();
    initialize();
}

void PostProcessingBuffer::cleanup() {
    if (m_colorTexture) {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }
    if (m_rbo) {
        glDeleteRenderbuffers(1, &m_rbo);
        m_rbo = 0;
    }
    if (m_fbo) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
}

// =============================================================================
// PostProcessing Implementation
// =============================================================================

PostProcessing::PostProcessing(unsigned int width, unsigned int height)
    : m_width(width), m_height(height),
      m_bloomEnabled(true), m_hdrEnabled(true),
      m_exposure(1.0f), m_bloomThreshold(1.0f), m_bloomIntensity(0.5f), m_gamma(2.2f),
      m_quadVAO(0), m_quadVBO(0) {
}

PostProcessing::~PostProcessing() {
    if (m_quadVAO) glDeleteVertexArrays(1, &m_quadVAO);
    if (m_quadVBO) glDeleteBuffers(1, &m_quadVBO);
}

bool PostProcessing::initialize() {
    setupQuad();
    
    // Load shaders
    g_brightPassShader = std::make_unique<eve::Shader>();
    if (!g_brightPassShader->loadFromFiles("shaders/post_processing.vert", "shaders/bright_pass.frag")) {
        std::cerr << "Failed to load bright pass shader" << std::endl;
        return false;
    }
    
    g_blurShader = std::make_unique<eve::Shader>();
    if (!g_blurShader->loadFromFiles("shaders/post_processing.vert", "shaders/gaussian_blur.frag")) {
        std::cerr << "Failed to load blur shader" << std::endl;
        return false;
    }
    
    g_downsampleShader = std::make_unique<eve::Shader>();
    if (!g_downsampleShader->loadFromFiles("shaders/post_processing.vert", "shaders/downsample.frag")) {
        std::cerr << "Failed to load downsample shader" << std::endl;
        return false;
    }
    
    g_upsampleShader = std::make_unique<eve::Shader>();
    if (!g_upsampleShader->loadFromFiles("shaders/post_processing.vert", "shaders/upsample.frag")) {
        std::cerr << "Failed to load upsample shader" << std::endl;
        return false;
    }
    
    g_toneMapShader = std::make_unique<eve::Shader>();
    if (!g_toneMapShader->loadFromFiles("shaders/post_processing.vert", "shaders/tone_mapping.frag")) {
        std::cerr << "Failed to load tone mapping shader" << std::endl;
        return false;
    }
    
    // Create bright pass buffer
    m_brightPassBuffer = std::make_unique<PostProcessingBuffer>(m_width, m_height, true);
    if (!m_brightPassBuffer->initialize()) {
        std::cerr << "Failed to create bright pass buffer" << std::endl;
        return false;
    }
    
    // Create ping-pong buffers for blur
    m_blurBuffers.resize(2);
    for (int i = 0; i < 2; i++) {
        m_blurBuffers[i] = std::make_unique<PostProcessingBuffer>(m_width, m_height, true);
        if (!m_blurBuffers[i]->initialize()) {
            std::cerr << "Failed to create blur buffer " << i << std::endl;
            return false;
        }
    }
    
    // Create downsampled mip buffers for bloom (5 levels)
    m_mipBuffers.resize(5);
    unsigned int mipWidth = m_width / 2;
    unsigned int mipHeight = m_height / 2;
    for (int i = 0; i < 5; i++) {
        m_mipBuffers[i] = std::make_unique<PostProcessingBuffer>(mipWidth, mipHeight, true);
        if (!m_mipBuffers[i]->initialize()) {
            std::cerr << "Failed to create mip buffer " << i << std::endl;
            return false;
        }
        mipWidth /= 2;
        mipHeight /= 2;
        if (mipWidth < 1) mipWidth = 1;
        if (mipHeight < 1) mipHeight = 1;
    }
    
    return true;
}

void PostProcessing::setupQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };
    
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    
    glBindVertexArray(0);
}

void PostProcessing::renderQuad() {
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void PostProcessing::process(GLuint inputTexture, GLuint outputFBO) {
    GLuint bloomTexture = 0;
    
    // Apply bloom if enabled
    if (m_bloomEnabled) {
        bloomTexture = applyBloom(inputTexture);
    }
    
    // Apply HDR tone mapping and output to screen/target
    if (m_hdrEnabled) {
        toneMap(inputTexture, bloomTexture, outputFBO);
    } else {
        // Simple copy if HDR disabled
        glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // TODO: Use simple blit shader
        // For now, just indicate this path isn't implemented
        std::cerr << "Warning: Non-HDR path not fully implemented" << std::endl;
    }
}

GLuint PostProcessing::brightPass(GLuint inputTexture) {
    m_brightPassBuffer->bind();
    
    g_brightPassShader->use();
    g_brightPassShader->setInt("inputTexture", 0);
    g_brightPassShader->setFloat("threshold", m_bloomThreshold);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    
    renderQuad();
    
    m_brightPassBuffer->unbind();
    return m_brightPassBuffer->getTexture();
}

GLuint PostProcessing::gaussianBlur(GLuint inputTexture, bool horizontal) {
    // Use ping-pong buffers
    int targetBuffer = horizontal ? 0 : 1;
    m_blurBuffers[targetBuffer]->bind();
    
    g_blurShader->use();
    g_blurShader->setInt("inputTexture", 0);
    g_blurShader->setBool("horizontal", horizontal);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    
    renderQuad();
    
    m_blurBuffers[targetBuffer]->unbind();
    return m_blurBuffers[targetBuffer]->getTexture();
}

void PostProcessing::downsample(GLuint inputTexture, PostProcessingBuffer* targetBuffer) {
    targetBuffer->bind();
    
    g_downsampleShader->use();
    g_downsampleShader->setInt("inputTexture", 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    
    renderQuad();
    
    targetBuffer->unbind();
}

GLuint PostProcessing::applyBloom(GLuint inputTexture) {
    // 1. Extract bright pixels
    GLuint brightTexture = brightPass(inputTexture);
    
    // 2. Downsample to create mip chain
    GLuint currentTexture = brightTexture;
    for (size_t i = 0; i < m_mipBuffers.size(); i++) {
        downsample(currentTexture, m_mipBuffers[i].get());
        currentTexture = m_mipBuffers[i]->getTexture();
        
        // Blur this mip level
        GLuint blurredH = gaussianBlur(currentTexture, true);
        GLuint blurredV = gaussianBlur(blurredH, false);
        
        // Store back (blurredV is in blur buffer, we need to copy to mip buffer)
        // For simplicity, we'll use the blurred result directly in next steps
    }
    
    // 3. Upsample and combine with additive blending
    GLuint result = m_mipBuffers[m_mipBuffers.size() - 1]->getTexture();
    
    for (int i = m_mipBuffers.size() - 2; i >= 0; i--) {
        // Bind target buffer
        m_blurBuffers[0]->bind();
        
        // Enable additive blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);
        
        // Upsample current result
        g_upsampleShader->use();
        g_upsampleShader->setInt("inputTexture", 0);
        g_upsampleShader->setFloat("filterRadius", 1.0f);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, result);
        
        renderQuad();
        
        // Add the current mip level
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_mipBuffers[i]->getTexture());
        
        renderQuad();
        
        glDisable(GL_BLEND);
        
        m_blurBuffers[0]->unbind();
        result = m_blurBuffers[0]->getTexture();
    }
    
    return result;
}

void PostProcessing::toneMap(GLuint inputTexture, GLuint bloomTexture, GLuint outputFBO) {
    glBindFramebuffer(GL_FRAMEBUFFER, outputFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    g_toneMapShader->use();
    g_toneMapShader->setInt("hdrTexture", 0);
    g_toneMapShader->setInt("bloomTexture", 1);
    g_toneMapShader->setBool("useBloom", bloomTexture != 0);
    g_toneMapShader->setFloat("exposure", m_exposure);
    g_toneMapShader->setFloat("gamma", m_gamma);
    g_toneMapShader->setInt("toneMapMode", 1); // ACES by default
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, inputTexture);
    
    if (bloomTexture) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, bloomTexture);
    }
    
    renderQuad();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessing::resize(unsigned int width, unsigned int height) {
    if (width == m_width && height == m_height) {
        return;
    }
    
    m_width = width;
    m_height = height;
    
    // Resize all buffers
    m_brightPassBuffer->resize(width, height);
    
    for (auto& buffer : m_blurBuffers) {
        buffer->resize(width, height);
    }
    
    // Resize mip buffers
    unsigned int mipWidth = width / 2;
    unsigned int mipHeight = height / 2;
    for (auto& buffer : m_mipBuffers) {
        buffer->resize(mipWidth, mipHeight);
        mipWidth /= 2;
        mipHeight /= 2;
        if (mipWidth < 1) mipWidth = 1;
        if (mipHeight < 1) mipHeight = 1;
    }
}

} // namespace Rendering
