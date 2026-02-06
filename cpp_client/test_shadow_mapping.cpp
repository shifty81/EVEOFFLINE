#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <memory>

#include "rendering/window.h"
#include "rendering/shader.h"
#include "rendering/camera.h"
#include "rendering/mesh.h"
#include "rendering/shadow_map.h"
#include "rendering/lighting.h"
#include "ui/input_handler.h"

using namespace eve;

// Window dimensions
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera
Camera camera;
InputHandler inputHandler;

// Mouse state
bool firstMouse = true;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Forward declarations
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

/**
 * Create a simple cube mesh
 */
std::unique_ptr<Mesh> createCube(const glm::vec3& color) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Cube vertices (8 corners)
    glm::vec3 positions[] = {
        // Front face
        {-1.0f, -1.0f,  1.0f}, { 1.0f, -1.0f,  1.0f}, { 1.0f,  1.0f,  1.0f}, {-1.0f,  1.0f,  1.0f},
        // Back face
        {-1.0f, -1.0f, -1.0f}, { 1.0f, -1.0f, -1.0f}, { 1.0f,  1.0f, -1.0f}, {-1.0f,  1.0f, -1.0f},
    };
    
    // Cube faces (6 faces, 2 triangles each)
    unsigned int cubeIndices[] = {
        // Front face
        0, 1, 2,  2, 3, 0,
        // Right face
        1, 5, 6,  6, 2, 1,
        // Back face
        5, 4, 7,  7, 6, 5,
        // Left face
        4, 0, 3,  3, 7, 4,
        // Top face
        3, 2, 6,  6, 7, 3,
        // Bottom face
        4, 5, 1,  1, 0, 4
    };
    
    // Create vertices with normals
    for (unsigned int i = 0; i < 36; i++) {
        Vertex vertex;
        vertex.position = positions[cubeIndices[i]];
        
        // Calculate normal based on face
        int face = i / 6;
        switch(face) {
            case 0: vertex.normal = glm::vec3( 0.0f,  0.0f,  1.0f); break;  // Front
            case 1: vertex.normal = glm::vec3( 1.0f,  0.0f,  0.0f); break;  // Right
            case 2: vertex.normal = glm::vec3( 0.0f,  0.0f, -1.0f); break;  // Back
            case 3: vertex.normal = glm::vec3(-1.0f,  0.0f,  0.0f); break;  // Left
            case 4: vertex.normal = glm::vec3( 0.0f,  1.0f,  0.0f); break;  // Top
            case 5: vertex.normal = glm::vec3( 0.0f, -1.0f,  0.0f); break;  // Bottom
        }
        
        vertex.texCoords = glm::vec2(0.0f, 0.0f);
        vertex.color = color;
        
        vertices.push_back(vertex);
        indices.push_back(i);
    }
    
    return std::make_unique<Mesh>(vertices, indices);
}

/**
 * Create a ground plane
 */
std::unique_ptr<Mesh> createGroundPlane(float size, const glm::vec3& color) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    
    // Four corners of the plane
    Vertex v0, v1, v2, v3;
    v0.position = glm::vec3(-size, 0.0f, -size);
    v1.position = glm::vec3( size, 0.0f, -size);
    v2.position = glm::vec3( size, 0.0f,  size);
    v3.position = glm::vec3(-size, 0.0f,  size);
    
    // All normals point up
    glm::vec3 normal(0.0f, 1.0f, 0.0f);
    v0.normal = v1.normal = v2.normal = v3.normal = normal;
    
    // Set color
    v0.color = v1.color = v2.color = v3.color = color;
    
    // Texture coordinates
    v0.texCoords = glm::vec2(0.0f, 0.0f);
    v1.texCoords = glm::vec2(1.0f, 0.0f);
    v2.texCoords = glm::vec2(1.0f, 1.0f);
    v3.texCoords = glm::vec2(0.0f, 1.0f);
    
    vertices.push_back(v0);
    vertices.push_back(v1);
    vertices.push_back(v2);
    vertices.push_back(v3);
    
    // Two triangles
    indices = {0, 1, 2,  2, 3, 0};
    
    return std::make_unique<Mesh>(vertices, indices);
}

int main() {
    // Initialize window
    Window window("EVE OFFLINE - Shadow Mapping Test", SCR_WIDTH, SCR_HEIGHT);
    
    GLFWwindow* glfwWindow = window.getHandle();
    glfwSetCursorPosCallback(glfwWindow, mouse_callback);
    glfwSetMouseButtonCallback(glfwWindow, mouse_button_callback);
    glfwSetScrollCallback(glfwWindow, scroll_callback);
    glfwSetKeyCallback(glfwWindow, key_callback);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Load shaders
    Shader lightingShader;
    if (!lightingShader.loadFromFiles("cpp_client/shaders/multi_light_shadow.vert", "cpp_client/shaders/multi_light_shadow.frag")) {
        std::cerr << "Failed to load lighting shaders" << std::endl;
        return -1;
    }
    
    Shader shadowShader;
    if (!shadowShader.loadFromFiles("cpp_client/shaders/shadow_map.vert", "cpp_client/shaders/shadow_map.frag")) {
        std::cerr << "Failed to load shadow shaders" << std::endl;
        return -1;
    }
    
    // Create meshes
    auto cubeMesh = createCube(glm::vec3(0.8f, 0.3f, 0.2f));
    auto groundMesh = createGroundPlane(100.0f, glm::vec3(0.3f, 0.4f, 0.3f));
    
    // Create shadow map
    Rendering::ShadowMap shadowMap(2048, 2048);
    
    // Setup lighting
    Lighting::LightManager lightManager;
    
    // Create directional light (sun)
    Lighting::Light sun = Lighting::LightManager::createDirectionalLight(
        glm::vec3(0.5f, -1.0f, -0.3f),
        glm::vec3(1.0f, 0.95f, 0.9f),
        1.0f
    );
    sun.castsShadows = true;
    lightManager.addLight(sun);
    
    // Set ambient light
    lightManager.setAmbientLight(glm::vec3(0.15f, 0.15f, 0.2f), 1.0f);
    
    // Setup camera
    camera.setTarget(glm::vec3(0.0f, 10.0f, 0.0f));
    camera.setDistance(150.0f);
    camera.setAspectRatio((float)SCR_WIDTH / (float)SCR_HEIGHT);
    
    std::cout << "=== Shadow Mapping Test ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  Right Mouse: Rotate camera" << std::endl;
    std::cout << "  Middle Mouse: Pan camera" << std::endl;
    std::cout << "  Mouse Wheel: Zoom in/out" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    std::cout << "==========================" << std::endl;
    
    // Render loop
    while (!window.shouldClose()) {
        // Per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        // Input
        processInput(glfwWindow);
        
        // Update camera
        camera.update(deltaTime);
        
        // Get light direction
        Lighting::Light* sunLight = lightManager.getLight(0);
        glm::vec3 lightDir = sunLight->direction;
        
        // Calculate light space matrix
        glm::mat4 lightSpaceMatrix = shadowMap.getLightSpaceMatrix(
            lightDir,
            glm::vec3(0.0f, 0.0f, 0.0f),
            150.0f
        );
        
        // === SHADOW PASS ===
        shadowShader.use();
        shadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        
        shadowMap.beginShadowPass();
        
        // Render ground
        glm::mat4 groundModel = glm::mat4(1.0f);
        shadowShader.setMat4("model", groundModel);
        groundMesh->draw();
        
        // Render cubes
        std::vector<glm::vec3> cubePositions = {
            glm::vec3( 0.0f, 10.0f,  0.0f),
            glm::vec3(20.0f, 15.0f, -10.0f),
            glm::vec3(-15.0f, 20.0f, 15.0f),
            glm::vec3(-20.0f, 8.0f, -20.0f),
            glm::vec3( 25.0f, 12.0f, 20.0f)
        };
        
        for (const auto& pos : cubePositions) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pos);
            model = glm::scale(model, glm::vec3(5.0f));
            shadowShader.setMat4("model", model);
            cubeMesh->draw();
        }
        
        shadowMap.endShadowPass();
        
        // === LIGHTING PASS ===
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        lightingShader.use();
        
        // Set view/projection matrices
        glm::mat4 projection = camera.getProjectionMatrix();
        glm::mat4 view = camera.getViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        lightingShader.setVec3("viewPos", camera.getPosition());
        
        // Set light space matrix for shadows
        lightingShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        
        // Upload lighting
        lightManager.uploadToShader(&lightingShader);
        
        // Set shadow uniforms
        lightingShader.setBool("useShadows", true);
        lightingShader.setFloat("shadowBias", 0.005f);
        
        // Bind shadow map
        shadowMap.bindShadowTexture(0);
        lightingShader.setInt("shadowMap", 0);
        
        // Render ground
        lightingShader.setMat4("model", groundModel);
        groundMesh->draw();
        
        // Render cubes
        for (const auto& pos : cubePositions) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pos);
            model = glm::scale(model, glm::vec3(5.0f));
            lightingShader.setMat4("model", model);
            cubeMesh->draw();
        }
        
        // Swap buffers and poll events
        glfwSwapBuffers(glfwWindow);
        glfwPollEvents();
    }
    
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        camera.pan(xoffset * 2.0f, yoffset * 2.0f);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // Mouse button handling is done in mouse_callback
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.zoom(-yoffset * 50.0f);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    inputHandler.handleKey(key, action, mods);
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}
