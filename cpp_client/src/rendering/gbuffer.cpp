#include "rendering/gbuffer.h"
#include <iostream>

namespace atlas {

GBuffer::GBuffer(unsigned int width, unsigned int height)
    : m_width(width)
    , m_height(height)
    , m_fbo(0)
    , m_positionTexture(0)
    , m_normalTexture(0)
    , m_albedoSpecTexture(0)
    , m_depthRenderbuffer(0)
    , m_initialized(false)
{
}

GBuffer::~GBuffer() {
    cleanup();
}

bool GBuffer::initialize() {
    std::cout << "Initializing G-Buffer (" << m_width << "x" << m_height << ")..." << std::endl;
    
    if (!createFramebuffer()) {
        std::cerr << "Failed to create G-Buffer framebuffer" << std::endl;
        return false;
    }
    
    m_initialized = true;
    std::cout << "G-Buffer initialized successfully" << std::endl;
    return true;
}

bool GBuffer::createFramebuffer() {
    // Generate framebuffer
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    
    // Position buffer (RGB16F for world position)
    glGenTextures(1, &m_positionTexture);
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_positionTexture, 0);
    
    // Normal buffer (RGB16F for world normal)
    glGenTextures(1, &m_normalTexture);
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, m_width, m_height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_normalTexture, 0);
    
    // Albedo + Specular buffer (RGBA8 for color and specular intensity)
    glGenTextures(1, &m_albedoSpecTexture);
    glBindTexture(GL_TEXTURE_2D, m_albedoSpecTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_albedoSpecTexture, 0);
    
    // Tell OpenGL which color attachments we'll use for rendering
    GLenum attachments[3] = {
        GL_COLOR_ATTACHMENT0,
        GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2
    };
    glDrawBuffers(3, attachments);
    
    // Create depth buffer (renderbuffer for depth testing)
    glGenRenderbuffers(1, &m_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
    
    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "G-Buffer framebuffer is not complete!" << std::endl;
        cleanup();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    std::cout << "G-Buffer framebuffer created:" << std::endl;
    std::cout << "  - Position: " << m_positionTexture << " (RGB16F)" << std::endl;
    std::cout << "  - Normal: " << m_normalTexture << " (RGB16F)" << std::endl;
    std::cout << "  - Albedo+Spec: " << m_albedoSpecTexture << " (RGBA8)" << std::endl;
    std::cout << "  - Depth: " << m_depthRenderbuffer << " (DEPTH24_STENCIL8)" << std::endl;
    
    return true;
}

void GBuffer::bindForGeometryPass() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GBuffer::bindForLightingPass(
    unsigned int positionUnit,
    unsigned int normalUnit,
    unsigned int albedoSpecUnit
) {
    glActiveTexture(GL_TEXTURE0 + positionUnit);
    glBindTexture(GL_TEXTURE_2D, m_positionTexture);
    
    glActiveTexture(GL_TEXTURE0 + normalUnit);
    glBindTexture(GL_TEXTURE_2D, m_normalTexture);
    
    glActiveTexture(GL_TEXTURE0 + albedoSpecUnit);
    glBindTexture(GL_TEXTURE_2D, m_albedoSpecTexture);
}

void GBuffer::unbind() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GBuffer::resize(unsigned int width, unsigned int height) {
    if (width == m_width && height == m_height) {
        return; // No change
    }
    
    std::cout << "Resizing G-Buffer: " << m_width << "x" << m_height
              << " -> " << width << "x" << height << std::endl;
    
    m_width = width;
    m_height = height;
    
    // Recreate framebuffer with new size
    cleanup();
    createFramebuffer();
}

void GBuffer::cleanup() {
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    
    if (m_positionTexture != 0) {
        glDeleteTextures(1, &m_positionTexture);
        m_positionTexture = 0;
    }
    
    if (m_normalTexture != 0) {
        glDeleteTextures(1, &m_normalTexture);
        m_normalTexture = 0;
    }
    
    if (m_albedoSpecTexture != 0) {
        glDeleteTextures(1, &m_albedoSpecTexture);
        m_albedoSpecTexture = 0;
    }
    
    if (m_depthRenderbuffer != 0) {
        glDeleteRenderbuffers(1, &m_depthRenderbuffer);
        m_depthRenderbuffer = 0;
    }
    
    m_initialized = false;
}

} // namespace atlas
