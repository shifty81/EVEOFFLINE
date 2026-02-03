#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace eve {

// Forward declarations
class Shader;
class Camera;
class Mesh;

/**
 * Main renderer class
 * Handles all OpenGL rendering
 */
class Renderer {
public:
    Renderer();
    ~Renderer();

    /**
     * Initialize renderer
     */
    bool initialize();

    /**
     * Begin frame
     */
    void beginFrame();

    /**
     * End frame
     */
    void endFrame();

    /**
     * Render the scene
     */
    void renderScene(Camera& camera);

    /**
     * Clear screen with color
     */
    void clear(const glm::vec4& color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    /**
     * Set viewport
     */
    void setViewport(int x, int y, int width, int height);

private:
    void setupStarfield();
    void renderStarfield(Camera& camera);

    std::unique_ptr<Shader> m_basicShader;
    std::unique_ptr<Shader> m_starfieldShader;
    
    // Starfield data
    unsigned int m_starfieldVAO;
    unsigned int m_starfieldVBO;
    int m_starCount;

    bool m_initialized;
};

} // namespace eve
