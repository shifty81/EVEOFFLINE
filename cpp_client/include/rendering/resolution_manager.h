#pragma once

#include <glm/glm.hpp>

namespace atlas {

/**
 * Resolution and viewport management
 * Handles resolution independence and window mode changes
 */
class ResolutionManager {
public:
    enum class WindowMode {
        WINDOWED,
        MAXIMIZED,
        BORDERLESS_FULLSCREEN
    };
    
    enum class AspectRatioMode {
        LETTERBOX,      // Add black bars to maintain aspect ratio
        EXPAND,         // Expand viewport for wider screens
        STRETCH         // Stretch to fill (not recommended)
    };
    
    ResolutionManager();
    ~ResolutionManager();
    
    /**
     * Initialize with target resolution
     * @param targetWidth Virtual/design resolution width
     * @param targetHeight Virtual/design resolution height
     */
    void initialize(int targetWidth, int targetHeight);
    
    /**
     * Update when window resizes
     * @param windowWidth Physical window width in pixels
     * @param windowHeight Physical window height in pixels
     */
    void onWindowResize(int windowWidth, int windowHeight);
    
    /**
     * Set window mode
     */
    void setWindowMode(WindowMode mode);
    WindowMode getWindowMode() const { return m_windowMode; }
    
    /**
     * Set aspect ratio handling mode
     */
    void setAspectRatioMode(AspectRatioMode mode);
    AspectRatioMode getAspectRatioMode() const { return m_aspectRatioMode; }
    
    /**
     * Get viewport for rendering
     * Returns x, y, width, height for glViewport
     */
    glm::ivec4 getViewport() const { return m_viewport; }
    
    /**
     * Get scale factors for UI
     */
    glm::vec2 getUIScale() const { return m_uiScale; }
    float getUIScaleX() const { return m_uiScale.x; }
    float getUIScaleY() const { return m_uiScale.y; }
    
    /**
     * Get target (virtual) resolution
     */
    glm::ivec2 getTargetResolution() const { 
        return glm::ivec2(m_targetWidth, m_targetHeight); 
    }
    
    /**
     * Get actual window resolution
     */
    glm::ivec2 getWindowResolution() const { 
        return glm::ivec2(m_windowWidth, m_windowHeight); 
    }
    
    /**
     * Get aspect ratios
     */
    float getTargetAspectRatio() const { return m_targetAspect; }
    float getWindowAspectRatio() const { return m_windowAspect; }
    
    /**
     * Convert screen coordinates to virtual coordinates
     */
    glm::vec2 screenToVirtual(const glm::vec2& screenPos) const;
    
    /**
     * Convert virtual coordinates to screen coordinates
     */
    glm::vec2 virtualToScreen(const glm::vec2& virtualPos) const;
    
private:
    void calculateViewport();
    void calculateUIScale();
    
    // Target (virtual) resolution
    int m_targetWidth;
    int m_targetHeight;
    float m_targetAspect;
    
    // Actual window resolution
    int m_windowWidth;
    int m_windowHeight;
    float m_windowAspect;
    
    // Viewport (for letterboxing)
    glm::ivec4 m_viewport;  // x, y, width, height
    
    // UI scale factors
    glm::vec2 m_uiScale;
    
    // Modes
    WindowMode m_windowMode;
    AspectRatioMode m_aspectRatioMode;
};

} // namespace atlas
