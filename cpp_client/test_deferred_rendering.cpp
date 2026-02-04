/**
 * Test Deferred Rendering Pipeline
 * Tests G-Buffer and two-pass rendering
 */

#include <iostream>
#include <memory>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "rendering/window.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "rendering/gbuffer.h"
#include "rendering/lighting.h"
#include "rendering/mesh.h"
#include "rendering/model.h"
#include "ui/input_handler.h"

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
    inputHandler.handleMouseMove(xpos, ypos);
    
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        camera.rotate(xoffset * 0.5f, yoffset * 0.5f);
    }
    
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        camera.pan(xoffset * 2.0f, yoffset * 2.0f);
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.zoom(-yoffset * 50.0f);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    inputHandler.handleKeyPress(key, action);
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

// Create a simple cube mesh
std::shared_ptr<Mesh> createCube() {
    std::vector<float> vertices = {
        // Position           Normal            TexCoord
        // Front face
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f,
        // Back face
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        // Top face
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
        // Bottom face
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
        // Right face
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        // Left face
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
    };
    
    std::vector<unsigned int> indices = {
        0,  1,  2,   2,  3,  0,  // Front
        4,  5,  6,   6,  7,  4,  // Back
        8,  9, 10,  10, 11,  8,  // Top
        12, 13, 14,  14, 15, 12,  // Bottom
        16, 17, 18,  18, 19, 16,  // Right
        20, 21, 22,  22, 23, 20   // Left
    };
    
    return std::make_shared<Mesh>(vertices, indices);
}

// Create a fullscreen quad for lighting pass
std::shared_ptr<Mesh> createQuad() {
    std::vector<float> vertices = {
        // Position        TexCoord
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
    };
    
    std::vector<unsigned int> indices = {
        0, 1, 2,  2, 3, 0
    };
    
    return std::make_shared<Mesh>(vertices, indices);
}

int main() {
    std::cout << "=== Deferred Rendering Pipeline Test ===" << std::endl;
    
    // Create window
    Window window;
    if (!window.initialize(WINDOW_WIDTH, WINDOW_HEIGHT, "Deferred Rendering Test")) {
        std::cerr << "Failed to initialize window" << std::endl;
        return -1;
    }
    
    // Set callbacks
    glfwSetCursorPosCallback(window.getHandle(), mouseCallback);
    glfwSetScrollCallback(window.getHandle(), scrollCallback);
    glfwSetKeyCallback(window.getHandle(), keyCallback);
    
    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Create G-Buffer
    GBuffer gbuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!gbuffer.initialize()) {
        std::cerr << "Failed to initialize G-Buffer" << std::endl;
        return -1;
    }
    
    // Load shaders
    Shader geometryShader("cpp_client/shaders/gbuffer_geometry.vert", 
                          "cpp_client/shaders/gbuffer_geometry.frag");
    if (!geometryShader.isValid()) {
        std::cerr << "Failed to load geometry shader" << std::endl;
        return -1;
    }
    
    Shader lightingShader("cpp_client/shaders/gbuffer_lighting.vert",
                          "cpp_client/shaders/gbuffer_lighting.frag");
    if (!lightingShader.isValid()) {
        std::cerr << "Failed to load lighting shader" << std::endl;
        return -1;
    }
    
    // Create meshes
    auto cube = createCube();
    auto quad = createQuad();
    
    // Setup camera
    camera.setPosition(glm::vec3(0.0f, 0.0f, 10.0f));
    camera.setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Setup lighting
    LightManager lightManager;
    
    // Add main directional light (sun)
    Light sun;
    sun.type = 0; // Directional
    sun.direction = glm::vec3(0.5f, -1.0f, 0.3f);
    sun.color = glm::vec3(1.0f, 1.0f, 0.95f);
    sun.intensity = 1.0f;
    lightManager.addLight(sun);
    
    // Add a few point lights
    Light pointLight1;
    pointLight1.type = 1; // Point
    pointLight1.position = glm::vec3(5.0f, 3.0f, 5.0f);
    pointLight1.color = glm::vec3(1.0f, 0.3f, 0.3f);
    pointLight1.intensity = 1.0f;
    pointLight1.constant = 1.0f;
    pointLight1.linear = 0.09f;
    pointLight1.quadratic = 0.032f;
    lightManager.addLight(pointLight1);
    
    Light pointLight2;
    pointLight2.type = 1; // Point
    pointLight2.position = glm::vec3(-5.0f, 3.0f, 5.0f);
    pointLight2.color = glm::vec3(0.3f, 0.3f, 1.0f);
    pointLight2.intensity = 1.0f;
    pointLight2.constant = 1.0f;
    pointLight2.linear = 0.09f;
    pointLight2.quadratic = 0.032f;
    lightManager.addLight(pointLight2);
    
    std::cout << "Setup complete! Controls:" << std::endl;
    std::cout << "  - Right Mouse: Rotate camera" << std::endl;
    std::cout << "  - Middle Mouse: Pan camera" << std::endl;
    std::cout << "  - Mouse Wheel: Zoom" << std::endl;
    std::cout << "  - ESC: Exit" << std::endl;
    
    // Main loop
    float lastFrame = 0.0f;
    while (!window.shouldClose()) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Process input
        window.processInput();
        
        // Update camera
        camera.update(deltaTime);
        
        // ========== GEOMETRY PASS ==========
        gbuffer.bindForGeometryPass();
        
        geometryShader.use();
        geometryShader.setMat4("view", camera.getViewMatrix());
        geometryShader.setMat4("projection", camera.getProjectionMatrix());
        
        // Render multiple cubes
        for (int x = -3; x <= 3; x++) {
            for (int z = -3; z <= 3; z++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(x * 3.0f, 0.0f, z * 3.0f));
                model = glm::rotate(model, currentFrame * glm::radians(20.0f * (x + z + 1)), 
                                    glm::vec3(0.3f, 1.0f, 0.1f));
                model = glm::scale(model, glm::vec3(1.0f));
                
                geometryShader.setMat4("model", model);
                
                // Set material properties
                glm::vec3 color = glm::vec3(
                    0.5f + 0.5f * sin(x * 0.5f),
                    0.5f + 0.5f * cos(z * 0.5f),
                    0.5f + 0.5f * sin((x+z) * 0.3f)
                );
                geometryShader.setVec3("material_albedo", color);
                geometryShader.setFloat("material_specular", 0.5f);
                geometryShader.setFloat("material_metallic", 0.3f);
                geometryShader.setFloat("material_roughness", 0.5f);
                
                cube->draw();
            }
        }
        
        gbuffer.unbind();
        
        // ========== LIGHTING PASS ==========
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        lightingShader.use();
        
        // Bind G-Buffer textures
        gbuffer.bindForLightingPass(0, 1, 2);
        lightingShader.setInt("gPosition", 0);
        lightingShader.setInt("gNormal", 1);
        lightingShader.setInt("gAlbedoSpec", 2);
        
        // Set camera position
        lightingShader.setVec3("viewPos", camera.getPosition());
        
        // Set ambient lighting
        lightingShader.setVec3("ambientColor", glm::vec3(0.1f, 0.1f, 0.15f));
        lightingShader.setFloat("ambientIntensity", 0.3f);
        
        // Upload lights
        lightManager.uploadToShader(lightingShader);
        
        // Render fullscreen quad
        glDisable(GL_DEPTH_TEST);
        quad->draw();
        glEnable(GL_DEPTH_TEST);
        
        // Swap buffers
        window.swapBuffers();
        window.pollEvents();
    }
    
    std::cout << "Exiting..." << std::endl;
    return 0;
}
