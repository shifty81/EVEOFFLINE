/**
 * Test Asteroid Field Rendering
 * Tests the asteroid field renderer with various configurations
 */

#include <iostream>
#include <memory>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "rendering/window.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "rendering/asteroid_field_renderer.h"
#include "ui/input_handler.h"

using namespace atlas;

// Window dimensions
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// Camera settings
Camera camera;
InputHandler inputHandler;

// Mouse control
bool firstMouse = true;
float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;

// Callbacks
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    inputHandler.handleMouse(xpos, ypos);
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    
    // Right mouse button rotates camera
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        camera.rotate(xoffset * 0.5f, yoffset * 0.5f);
    }
    
    // Middle mouse button pans camera
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        camera.pan(xoffset * 2.0f, yoffset * 2.0f);
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.zoom(-yoffset * 500.0f);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    inputHandler.handleKey(key, action, mods);
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main() {
    std::cout << "=== Asteroid Field Rendering Test ===" << std::endl;
    
    // Create window
    Window window("Asteroid Field Test", WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Set callbacks
    glfwSetCursorPosCallback(window.getHandle(), mouseCallback);
    glfwSetScrollCallback(window.getHandle(), scrollCallback);
    glfwSetKeyCallback(window.getHandle(), keyCallback);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }
    
    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Load shaders
    Shader shader;
    if (!shader.loadFromFiles("cpp_client/shaders/basic.vert", "cpp_client/shaders/basic.frag")) {
        std::cerr << "Failed to load shaders" << std::endl;
        return -1;
    }
    
    // Initialize camera
    camera.setDistance(5000.0f);
    camera.setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Initialize asteroid field renderer
    auto asteroidRenderer = std::make_unique<AsteroidFieldRenderer>();
    if (!asteroidRenderer->initialize()) {
        std::cerr << "Failed to initialize asteroid renderer" << std::endl;
        return -1;
    }
    
    // Test 1: Small semicircle field
    std::cout << "\n=== Test 1: Small Semicircle Field ===" << std::endl;
    std::vector<int> counts1 = {50, 30, 15, 5}; // SMALL, MEDIUM, LARGE, HUGE
    asteroidRenderer->generateField(
        glm::vec3(0.0f, 0.0f, 0.0f),
        10000.0f,
        counts1,
        AsteroidFieldRenderer::BeltLayout::SEMICIRCLE,
        42
    );
    
    // Wait a bit for user to see
    std::cout << "\nPress any key to continue to next test..." << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Right Mouse: Rotate camera" << std::endl;
    std::cout << "  Middle Mouse: Pan camera" << std::endl;
    std::cout << "  Mouse Wheel: Zoom in/out" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    
    // Render loop
    float lastFrameTime = glfwGetTime();
    int frameCount = 0;
    int testPhase = 0;
    float phaseTime = 0.0f;
    
    while (!window.shouldClose()) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        
        phaseTime += deltaTime;
        frameCount++;
        
        // Change test phase every 10 seconds
        if (phaseTime > 10.0f && testPhase < 2) {
            testPhase++;
            phaseTime = 0.0f;
            
            if (testPhase == 1) {
                // Test 2: Large spherical field
                std::cout << "\n=== Test 2: Large Spherical Field ===" << std::endl;
                std::vector<int> counts2 = {100, 60, 30, 10};
                asteroidRenderer->generateField(
                    glm::vec3(0.0f, 0.0f, 0.0f),
                    20000.0f,
                    counts2,
                    AsteroidFieldRenderer::BeltLayout::SPHERICAL,
                    123
                );
            } else if (testPhase == 2) {
                // Test 3: Dense field
                std::cout << "\n=== Test 3: Dense Asteroid Field ===" << std::endl;
                std::vector<int> counts3 = {200, 100, 50, 20};
                asteroidRenderer->generateField(
                    glm::vec3(5000.0f, 0.0f, 0.0f),
                    15000.0f,
                    counts3,
                    AsteroidFieldRenderer::BeltLayout::SEMICIRCLE,
                    456
                );
            }
        }
        
        // Print FPS every second
        static float fpsTimer = 0.0f;
        static int fpsCount = 0;
        fpsTimer += deltaTime;
        fpsCount++;
        if (fpsTimer >= 1.0f) {
            std::cout << "FPS: " << fpsCount << " | Asteroids: " 
                      << asteroidRenderer->getAsteroidCount() << std::endl;
            fpsTimer = 0.0f;
            fpsCount = 0;
        }
        
        // Process input
        glfwPollEvents();
        
        // Update camera
        camera.update(deltaTime);
        
        // Clear screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Render asteroids
        shader.use();
        
        // Set matrices
        glm::mat4 projection = glm::perspective(
            glm::radians(60.0f),
            (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
            1.0f,
            100000.0f
        );
        shader.setMat4("projection", projection);
        shader.setMat4("view", camera.getViewMatrix());
        
        // Set lighting
        shader.setVec3("viewPos", camera.getPosition());
        shader.setVec3("lightDir", glm::normalize(glm::vec3(0.5f, -0.3f, -0.2f)));
        shader.setVec3("lightColor", glm::vec3(1.0f, 0.95f, 0.9f));
        shader.setVec3("ambientColor", glm::vec3(0.15f, 0.15f, 0.2f));
        
        // Render asteroid field
        asteroidRenderer->render(&shader, camera);
        
        // Swap buffers
        window.update();
    }
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    std::cout << "Total frames rendered: " << frameCount << std::endl;
    
    return 0;
}
