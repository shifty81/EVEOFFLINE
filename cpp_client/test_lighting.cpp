/**
 * Test Dynamic Lighting System
 * Tests multiple light types and configurations
 */

#include <iostream>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "rendering/window.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
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

int main() {
    std::cout << "=== Dynamic Lighting System Test ===" << std::endl;
    
    // Create window
    Window window;
    if (!window.initialize(WINDOW_WIDTH, WINDOW_HEIGHT, "Lighting Test")) {
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
    
    // Load shaders with multi-light support
    Shader shader("cpp_client/shaders/basic.vert", "cpp_client/shaders/multi_light.frag");
    if (!shader.isValid()) {
        std::cerr << "Failed to load shaders" << std::endl;
        return -1;
    }
    
    // Initialize camera
    camera.setDistance(500.0f);
    camera.setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    camera.updateViewMatrix();
    
    // Create light manager
    auto lightManager = std::make_unique<Lighting::LightManager>();
    
    // Test 1: EVE-style lighting (3 directional lights)
    std::cout << "\n=== Test 1: EVE-Style Lighting ===" << std::endl;
    lightManager->setupEVEStyleLighting();
    
    // Create some test objects (spheres)
    std::vector<std::unique_ptr<Model>> testObjects;
    std::vector<glm::vec3> positions = {
        {0.0f, 0.0f, 0.0f},
        {-200.0f, 0.0f, 0.0f},
        {200.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, -200.0f},
        {0.0f, 0.0f, 200.0f},
    };
    
    for (const auto& pos : positions) {
        testObjects.push_back(Model::createSphere(50.0f, 32, 16));
    }
    
    std::cout << "\nControls:" << std::endl;
    std::cout << "  Right Mouse: Rotate camera" << std::endl;
    std::cout << "  Middle Mouse: Pan camera" << std::endl;
    std::cout << "  Mouse Wheel: Zoom in/out" << std::endl;
    std::cout << "  1: EVE-style lighting (3 directional)" << std::endl;
    std::cout << "  2: Single directional light" << std::endl;
    std::cout << "  3: Point lights demo" << std::endl;
    std::cout << "  4: Spot lights demo" << std::endl;
    std::cout << "  5: Mixed lighting" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    
    // Render loop
    float lastFrameTime = glfwGetTime();
    int currentTest = 1;
    
    while (!window.shouldClose()) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;
        
        // Check for test changes
        if (glfwGetKey(window.getHandle(), GLFW_KEY_1) == GLFW_PRESS && currentTest != 1) {
            std::cout << "\n=== Test 1: EVE-Style Lighting ===" << std::endl;
            lightManager->setupEVEStyleLighting();
            currentTest = 1;
        }
        else if (glfwGetKey(window.getHandle(), GLFW_KEY_2) == GLFW_PRESS && currentTest != 2) {
            std::cout << "\n=== Test 2: Single Directional Light ===" << std::endl;
            lightManager->clearLights();
            lightManager->setAmbientLight(glm::vec3(0.1f, 0.1f, 0.15f), 1.0f);
            auto light = Lighting::LightManager::createDirectionalLight(
                glm::vec3(0.5f, -0.5f, -0.5f),
                glm::vec3(1.0f, 1.0f, 1.0f),
                1.5f
            );
            lightManager->addLight(light);
            currentTest = 2;
        }
        else if (glfwGetKey(window.getHandle(), GLFW_KEY_3) == GLFW_PRESS && currentTest != 3) {
            std::cout << "\n=== Test 3: Point Lights ===" << std::endl;
            lightManager->clearLights();
            lightManager->setAmbientLight(glm::vec3(0.05f, 0.05f, 0.1f), 1.0f);
            
            // Add colored point lights
            auto redLight = Lighting::LightManager::createPointLight(
                glm::vec3(-200.0f, 50.0f, 0.0f),
                glm::vec3(1.0f, 0.2f, 0.2f),
                2.0f,
                300.0f
            );
            lightManager->addLight(redLight);
            
            auto greenLight = Lighting::LightManager::createPointLight(
                glm::vec3(200.0f, 50.0f, 0.0f),
                glm::vec3(0.2f, 1.0f, 0.2f),
                2.0f,
                300.0f
            );
            lightManager->addLight(greenLight);
            
            auto blueLight = Lighting::LightManager::createPointLight(
                glm::vec3(0.0f, 50.0f, 200.0f),
                glm::vec3(0.2f, 0.2f, 1.0f),
                2.0f,
                300.0f
            );
            lightManager->addLight(blueLight);
            
            currentTest = 3;
        }
        else if (glfwGetKey(window.getHandle(), GLFW_KEY_4) == GLFW_PRESS && currentTest != 4) {
            std::cout << "\n=== Test 4: Spot Lights ===" << std::endl;
            lightManager->clearLights();
            lightManager->setAmbientLight(glm::vec3(0.05f, 0.05f, 0.1f), 1.0f);
            
            // Add spot lights
            auto spotlight1 = Lighting::LightManager::createSpotLight(
                glm::vec3(0.0f, 300.0f, 0.0f),
                glm::vec3(0.0f, -1.0f, 0.0f),
                glm::vec3(1.0f, 1.0f, 0.8f),
                3.0f,
                500.0f,
                15.0f,
                25.0f
            );
            lightManager->addLight(spotlight1);
            
            auto spotlight2 = Lighting::LightManager::createSpotLight(
                glm::vec3(-300.0f, 100.0f, 0.0f),
                glm::vec3(1.0f, -0.3f, 0.0f),
                glm::vec3(0.8f, 0.4f, 1.0f),
                2.0f,
                400.0f,
                20.0f,
                30.0f
            );
            lightManager->addLight(spotlight2);
            
            currentTest = 4;
        }
        else if (glfwGetKey(window.getHandle(), GLFW_KEY_5) == GLFW_PRESS && currentTest != 5) {
            std::cout << "\n=== Test 5: Mixed Lighting ===" << std::endl;
            lightManager->clearLights();
            lightManager->setAmbientLight(glm::vec3(0.1f, 0.1f, 0.15f), 1.0f);
            
            // Add directional (sun)
            auto sun = Lighting::LightManager::createDirectionalLight(
                glm::vec3(0.5f, -0.3f, -0.2f),
                glm::vec3(1.0f, 0.95f, 0.9f),
                1.0f
            );
            lightManager->addLight(sun);
            
            // Add point light
            auto point = Lighting::LightManager::createPointLight(
                glm::vec3(0.0f, 100.0f, 0.0f),
                glm::vec3(1.0f, 0.5f, 0.2f),
                2.0f,
                400.0f
            );
            lightManager->addLight(point);
            
            // Add spot
            auto spot = Lighting::LightManager::createSpotLight(
                glm::vec3(200.0f, 200.0f, 200.0f),
                glm::vec3(-1.0f, -1.0f, -1.0f),
                glm::vec3(0.5f, 0.8f, 1.0f),
                2.0f,
                600.0f,
                20.0f,
                30.0f
            );
            lightManager->addLight(spot);
            
            currentTest = 5;
        }
        
        // Process input
        glfwPollEvents();
        
        // Update camera
        camera.updateViewMatrix();
        
        // Clear screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Render objects
        shader.use();
        
        // Set matrices
        glm::mat4 projection = glm::perspective(
            glm::radians(60.0f),
            (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT,
            0.1f,
            10000.0f
        );
        shader.setMat4("projection", projection);
        shader.setMat4("view", camera.getViewMatrix());
        shader.setVec3("viewPos", camera.getPosition());
        
        // Upload lighting
        lightManager->uploadToShader(&shader);
        
        // Render test objects
        for (size_t i = 0; i < testObjects.size(); i++) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), positions[i]);
            shader.setMat4("model", model);
            testObjects[i]->draw();
        }
        
        // Swap buffers
        window.swapBuffers();
    }
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    
    return 0;
}
