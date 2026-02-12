#include "rendering/resolution_manager.h"
#include <algorithm>
#include <iostream>

namespace atlas {

ResolutionManager::ResolutionManager()
    : m_targetWidth(1920)
    , m_targetHeight(1080)
    , m_targetAspect(16.0f / 9.0f)
    , m_windowWidth(1920)
    , m_windowHeight(1080)
    , m_windowAspect(16.0f / 9.0f)
    , m_viewport(0, 0, 1920, 1080)
    , m_uiScale(1.0f, 1.0f)
    , m_windowMode(WindowMode::WINDOWED)
    , m_aspectRatioMode(AspectRatioMode::LETTERBOX)
{
}

ResolutionManager::~ResolutionManager() {
}

void ResolutionManager::initialize(int targetWidth, int targetHeight) {
    m_targetWidth = targetWidth;
    m_targetHeight = targetHeight;
    m_targetAspect = static_cast<float>(targetWidth) / static_cast<float>(targetHeight);
    
    std::cout << "ResolutionManager initialized: " 
              << m_targetWidth << "x" << m_targetHeight 
              << " (aspect: " << m_targetAspect << ")" << std::endl;
    
    // Initial viewport matches target
    m_windowWidth = targetWidth;
    m_windowHeight = targetHeight;
    m_windowAspect = m_targetAspect;
    
    calculateViewport();
    calculateUIScale();
}

void ResolutionManager::onWindowResize(int windowWidth, int windowHeight) {
    if (windowWidth <= 0 || windowHeight <= 0) {
        std::cerr << "Invalid window size: " << windowWidth << "x" << windowHeight << std::endl;
        return;
    }
    
    std::cout << "Window resized to: " << windowWidth << "x" << windowHeight << std::endl;
    
    m_windowWidth = windowWidth;
    m_windowHeight = windowHeight;
    m_windowAspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    
    calculateViewport();
    calculateUIScale();
    
    std::cout << "Viewport: " << m_viewport.x << ", " << m_viewport.y << ", " 
              << m_viewport.z << ", " << m_viewport.w << std::endl;
    std::cout << "UI Scale: " << m_uiScale.x << ", " << m_uiScale.y << std::endl;
}

void ResolutionManager::setWindowMode(WindowMode mode) {
    m_windowMode = mode;
    std::cout << "Window mode changed to: " << static_cast<int>(mode) << std::endl;
}

void ResolutionManager::setAspectRatioMode(AspectRatioMode mode) {
    m_aspectRatioMode = mode;
    calculateViewport();
    calculateUIScale();
}

void ResolutionManager::calculateViewport() {
    if (m_aspectRatioMode == AspectRatioMode::LETTERBOX) {
        // Maintain target aspect ratio with letterboxing/pillarboxing
        float windowAspect = m_windowAspect;
        float targetAspect = m_targetAspect;
        
        int viewportWidth, viewportHeight;
        int viewportX, viewportY;
        
        if (windowAspect > targetAspect) {
            // Window is wider - add pillarbox (black bars on sides)
            viewportHeight = m_windowHeight;
            viewportWidth = static_cast<int>(viewportHeight * targetAspect);
            viewportX = (m_windowWidth - viewportWidth) / 2;
            viewportY = 0;
        } else {
            // Window is taller - add letterbox (black bars on top/bottom)
            viewportWidth = m_windowWidth;
            viewportHeight = static_cast<int>(viewportWidth / targetAspect);
            viewportX = 0;
            viewportY = (m_windowHeight - viewportHeight) / 2;
        }
        
        m_viewport = glm::ivec4(viewportX, viewportY, viewportWidth, viewportHeight);
        
    } else if (m_aspectRatioMode == AspectRatioMode::EXPAND) {
        // Use full window, expand viewport for wider screens
        m_viewport = glm::ivec4(0, 0, m_windowWidth, m_windowHeight);
        
    } else if (m_aspectRatioMode == AspectRatioMode::STRETCH) {
        // Stretch to fill (not recommended, causes distortion)
        m_viewport = glm::ivec4(0, 0, m_windowWidth, m_windowHeight);
    }
}

void ResolutionManager::calculateUIScale() {
    if (m_aspectRatioMode == AspectRatioMode::LETTERBOX) {
        // UI scales uniformly based on viewport size vs target size
        float scaleX = static_cast<float>(m_viewport.z) / static_cast<float>(m_targetWidth);
        float scaleY = static_cast<float>(m_viewport.w) / static_cast<float>(m_targetHeight);
        
        // Use uniform scale (smaller of the two to maintain aspect ratio)
        float uniformScale = std::min(scaleX, scaleY);
        m_uiScale = glm::vec2(uniformScale, uniformScale);
        
    } else if (m_aspectRatioMode == AspectRatioMode::EXPAND) {
        // UI scales based on height to maintain readable text
        // Width expands for wider screens
        float scaleY = static_cast<float>(m_windowHeight) / static_cast<float>(m_targetHeight);
        m_uiScale = glm::vec2(scaleY, scaleY);
        
    } else if (m_aspectRatioMode == AspectRatioMode::STRETCH) {
        // Scale independently (causes UI distortion)
        float scaleX = static_cast<float>(m_windowWidth) / static_cast<float>(m_targetWidth);
        float scaleY = static_cast<float>(m_windowHeight) / static_cast<float>(m_targetHeight);
        m_uiScale = glm::vec2(scaleX, scaleY);
    }
}

glm::vec2 ResolutionManager::screenToVirtual(const glm::vec2& screenPos) const {
    // Convert screen coordinates to viewport coordinates
    float viewportX = screenPos.x - m_viewport.x;
    float viewportY = screenPos.y - m_viewport.y;
    
    // Scale to virtual coordinates
    float virtualX = (viewportX / m_viewport.z) * m_targetWidth;
    float virtualY = (viewportY / m_viewport.w) * m_targetHeight;
    
    return glm::vec2(virtualX, virtualY);
}

glm::vec2 ResolutionManager::virtualToScreen(const glm::vec2& virtualPos) const {
    // Scale virtual coordinates to viewport
    float viewportX = (virtualPos.x / m_targetWidth) * m_viewport.z;
    float viewportY = (virtualPos.y / m_targetHeight) * m_viewport.w;
    
    // Add viewport offset
    float screenX = viewportX + m_viewport.x;
    float screenY = viewportY + m_viewport.y;
    
    return glm::vec2(screenX, screenY);
}

} // namespace atlas
