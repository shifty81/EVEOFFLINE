/**
 * Test Dynamic Lighting System
 * Tests multiple light types and configurations
 */

#include <iostream>
#include <memory>
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
#include "ui/ui_manager.h"

// Window dimensions
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// Camera settings
using namespace eve;
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
    inputHandler.handleKey(key, action, mods);
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main() {
    std::cout << "=== Dynamic Lighting System Test ===" << std::endl;
    
    // Create window
    Window window("Lighting Test", WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Set callbacks
    glfwSetCursorPosCallback(window.getHandle(), mouseCallback);
    glfwSetScrollCallback(window.getHandle(), scrollCallback);
    glfwSetKeyCallback(window.getHandle(), keyCallback);
    
    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Load shaders with multi-light support
    Shader shader;
    if (!shader.loadFromFiles("shaders/basic.vert", "shaders/multi_light.frag")) {
        std::cerr << "Failed to load shaders" << std::endl;
        return -1;
    }
    
    // Initialize camera
    camera.setDistance(500.0f);
    camera.setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
    
    // Create light manager
    auto lightManager = std::make_unique<Lighting::LightManager>();
    
    // Initialize UI Manager
    auto uiManager = std::make_unique<UI::UIManager>();
    if (!uiManager->Initialize(window.getHandle())) {
        std::cerr << "Failed to initialize UI Manager" << std::endl;
        return -1;
    }
    
    // Setup initial ship status with demo values
    UI::ShipStatus shipStatus;
    shipStatus.shields = 85.0f;
    shipStatus.shields_max = 100.0f;
    shipStatus.armor = 65.0f;
    shipStatus.armor_max = 100.0f;
    shipStatus.hull = 95.0f;
    shipStatus.hull_max = 100.0f;
    shipStatus.capacitor = 70.0f;
    shipStatus.capacitor_max = 100.0f;
    shipStatus.velocity = 45.5f;
    shipStatus.max_velocity = 120.0f;
    uiManager->SetShipStatus(shipStatus);
    
    // Setup target info
    UI::TargetInfo targetInfo;
    targetInfo.name = "Hostile Frigate";
    targetInfo.shields = 30.0f;
    targetInfo.shields_max = 100.0f;
    targetInfo.armor = 50.0f;
    targetInfo.armor_max = 100.0f;
    targetInfo.hull = 80.0f;
    targetInfo.hull_max = 100.0f;
    targetInfo.distance = 2450.0f;
    targetInfo.is_hostile = true;
    targetInfo.is_locked = true;
    uiManager->SetTargetInfo(targetInfo);
    
    // Add some combat log messages
    uiManager->AddCombatLogMessage("[12:34:56] Locked target: Hostile Frigate");
    uiManager->AddCombatLogMessage("[12:34:58] Activated weapons");
    uiManager->AddCombatLogMessage("[12:35:00] Hit! 250 damage dealt");
    uiManager->AddCombatLogMessage("[12:35:02] Target shields depleted");
    
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
        testObjects.push_back(Model::createShipModel("frigate", "caldari"));
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
        camera.update(deltaTime);
        
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
        
        // Render UI on top
        uiManager->BeginFrame();
        uiManager->Render();
        uiManager->EndFrame();
        
        // Animate ship values for demo (slowly change shields)
        shipStatus.shields = 50.0f + 50.0f * std::sin(currentTime * 0.5f);
        shipStatus.velocity = 45.5f + 20.0f * std::sin(currentTime * 0.3f);
        uiManager->SetShipStatus(shipStatus);
        
        // Animate target health
        targetInfo.shields = 30.0f * std::max(0.0f, std::sin(currentTime * 0.4f));
        uiManager->SetTargetInfo(targetInfo);
        
        // Swap buffers
        window.update();
    }
    
    // Cleanup UI
    uiManager->Shutdown();
    
    std::cout << "\n=== Test Complete ===" << std::endl;
    
    return 0;
}
