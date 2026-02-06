/**
 * @file test_post_processing.cpp
 * @brief Test program for post-processing effects (bloom and HDR)
 * 
 * Demonstrates:
 * - HDR rendering pipeline
 * - Bloom effect with multiple mip levels
 * - Multiple tone mapping operators
 * - Interactive parameter adjustment
 */

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "rendering/window.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "rendering/mesh.h"
#include "rendering/post_processing.h"
#include "rendering/lighting.h"

#include <iostream>
#include <memory>
#include <vector>

// Window dimensions
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera
std::unique_ptr<eve::Camera> camera;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Settings
struct Settings {
    bool bloomEnabled = true;
    bool hdrEnabled = true;
    float exposure = 1.0f;
    float bloomThreshold = 1.0f;
    float bloomIntensity = 0.5f;
    float gamma = 2.2f;
    int toneMapMode = 1; // 0=Reinhard, 1=ACES, 2=Uncharted2
} settings;

// Function declarations
void processInput(GLFWwindow* window);
void renderCube();
void printControls();

// Cube VAO/VBO
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;

int main() {
    // Initialize window
    eve::Window window("Post-Processing Test (Bloom & HDR)", SCR_WIDTH, SCR_HEIGHT);
    
    // Create camera
    camera = std::make_unique<eve::Camera>(45.0f, (float)SCR_WIDTH / (float)SCR_HEIGHT);
    camera->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    camera->zoom(-20.0f);  // Move camera back
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(glewErr) << std::endl;
        return -1;
    }
    
    // OpenGL configuration
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    
    // Load shaders
    eve::Shader lightingShader;
    lightingShader.loadFromFiles("shaders/multi_light.vert", "shaders/multi_light.frag");
    
    // Setup post-processing
    auto postProcessing = std::make_unique<Rendering::PostProcessing>(SCR_WIDTH, SCR_HEIGHT);
    if (!postProcessing->initialize()) {
        std::cerr << "Failed to initialize post-processing" << std::endl;
        return -1;
    }
    
    // Setup lights
    Lighting::LightManager lightManager;
    
    // Sun light (directional)
    auto sun = Lighting::LightManager::createDirectionalLight(
        glm::vec3(0.3f, -1.0f, -0.5f),
        glm::vec3(1.0f, 0.95f, 0.9f),
        0.5f
    );
    lightManager.addLight(sun);
    
    // Bright point lights (for bloom demonstration)
    std::vector<glm::vec3> lightPositions = {
        glm::vec3(0.0f, 2.0f, 0.0f),
        glm::vec3(5.0f, 2.0f, 5.0f),
        glm::vec3(-5.0f, 2.0f, 5.0f),
        glm::vec3(5.0f, 2.0f, -5.0f),
        glm::vec3(-5.0f, 2.0f, -5.0f)
    };
    
    std::vector<glm::vec3> lightColors = {
        glm::vec3(5.0f, 5.0f, 5.0f),    // Bright white (for bloom)
        glm::vec3(10.0f, 2.0f, 2.0f),   // Bright red
        glm::vec3(2.0f, 10.0f, 2.0f),   // Bright green
        glm::vec3(2.0f, 2.0f, 10.0f),   // Bright blue
        glm::vec3(10.0f, 10.0f, 2.0f)   // Bright yellow
    };
    
    for (size_t i = 0; i < lightPositions.size(); i++) {
        auto light = Lighting::LightManager::createPointLight(
            lightPositions[i],
            lightColors[i],
            1.0f,
            1.0f,   // Constant
            0.09f,  // Linear
            0.032f  // Quadratic
        );
        lightManager.addLight(light);
    }
    
    // HDR framebuffer for scene rendering
    auto hdrBuffer = std::make_unique<Rendering::PostProcessingBuffer>(SCR_WIDTH, SCR_HEIGHT, true);
    if (!hdrBuffer->initialize()) {
        std::cerr << "Failed to create HDR buffer" << std::endl;
        return -1;
    }
    
    printControls();
    
    // Render loop
    while (!window.shouldClose()) {
        // Time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Input
        processInput(window.getHandle());
        
        // Update post-processing settings
        postProcessing->setBloomEnabled(settings.bloomEnabled);
        postProcessing->setHDREnabled(settings.hdrEnabled);
        postProcessing->setExposure(settings.exposure);
        postProcessing->setBloomThreshold(settings.bloomThreshold);
        postProcessing->setBloomIntensity(settings.bloomIntensity);
        postProcessing->setGamma(settings.gamma);
        
        // === SCENE RENDERING PASS (to HDR buffer) ===
        hdrBuffer->bind();
        
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        lightingShader.use();
        
        // Set view/projection
        glm::mat4 projection = camera->getProjectionMatrix();
        glm::mat4 view = camera->getViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setVec3("viewPos", camera->getPosition());
        
        // Upload lights
        lightManager.uploadToShader(&lightingShader);
        
        // Render cubes
        for (int i = 0; i < 5; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i] + glm::vec3(0.0f, -2.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.5f));
            lightingShader.setMat4("model", model);
            lightingShader.setVec3("material_albedo", glm::vec3(0.3f, 0.3f, 0.3f));
            lightingShader.setFloat("material_specular", 0.5f);
            renderCube();
        }
        
        // Render light sources (as bright cubes)
        for (size_t i = 0; i < lightPositions.size(); i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, lightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f));
            lightingShader.setMat4("model", model);
            lightingShader.setVec3("material_albedo", lightColors[i]);
            lightingShader.setFloat("material_specular", 0.0f);
            renderCube();
        }
        
        // Render floor
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
            model = glm::scale(model, glm::vec3(20.0f, 0.1f, 20.0f));
            lightingShader.setMat4("model", model);
            lightingShader.setVec3("material_albedo", glm::vec3(0.2f, 0.2f, 0.2f));
            lightingShader.setFloat("material_specular", 0.1f);
            renderCube();
        }
        
        hdrBuffer->unbind();
        
        // === POST-PROCESSING PASS ===
        postProcessing->process(hdrBuffer->getTexture(), 0);
        
        // Display info
        static float lastInfoTime = 0.0f;
        if (currentFrame - lastInfoTime > 1.0f) {
            std::cout << "FPS: " << (int)(1.0f / deltaTime) 
                      << " | Bloom: " << (settings.bloomEnabled ? "ON" : "OFF")
                      << " | Exposure: " << settings.exposure
                      << " | Threshold: " << settings.bloomThreshold
                      << std::endl;
            lastInfoTime = currentFrame;
        }
        
        // Swap and poll
        window.update();
    }
    
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Bloom toggle
    static bool bKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bKeyPressed) {
        settings.bloomEnabled = !settings.bloomEnabled;
        bKeyPressed = true;
        std::cout << "Bloom: " << (settings.bloomEnabled ? "ON" : "OFF") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
        bKeyPressed = false;
    }
    
    // Exposure adjustment
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        settings.exposure -= 0.5f * deltaTime;
        if (settings.exposure < 0.1f) settings.exposure = 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        settings.exposure += 0.5f * deltaTime;
        if (settings.exposure > 5.0f) settings.exposure = 5.0f;
    }
    
    // Bloom threshold
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        settings.bloomThreshold -= 0.5f * deltaTime;
        if (settings.bloomThreshold < 0.1f) settings.bloomThreshold = 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        settings.bloomThreshold += 0.5f * deltaTime;
        if (settings.bloomThreshold > 5.0f) settings.bloomThreshold = 5.0f;
    }
}

void renderCube() {
    if (cubeVAO == 0) {
        float vertices[] = {
            // positions          // normals
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
        };
        
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        
        glBindVertexArray(cubeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        
        glBindVertexArray(0);
    }
    
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void printControls() {
    std::cout << "\n=== Post-Processing Test Controls ===" << std::endl;
    std::cout << "B - Toggle bloom effect" << std::endl;
    std::cout << "Q/E - Decrease/Increase exposure" << std::endl;
    std::cout << "Z/X - Decrease/Increase bloom threshold" << std::endl;
    std::cout << "ESC - Exit\n" << std::endl;
}
