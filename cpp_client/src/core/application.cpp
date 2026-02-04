#include "core/application.h"
#include "rendering/window.h"
#include "rendering/renderer.h"
#include "rendering/camera.h"
#include "core/game_client.h"
#include "ui/input_handler.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace eve {

Application* Application::s_instance = nullptr;

Application::Application(const std::string& title, int width, int height)
    : m_running(false)
    , m_lastFrameTime(0.0f)
{
    if (s_instance != nullptr) {
        throw std::runtime_error("Application already exists");
    }
    s_instance = this;
    
    std::cout << "Creating application: " << title << std::endl;
    
    // Create window
    m_window = std::make_unique<Window>(title, width, height);
    
    // Create subsystems
    m_renderer = std::make_unique<Renderer>();
    m_gameClient = std::make_unique<GameClient>();
    m_inputHandler = std::make_unique<InputHandler>();
    
    // Initialize
    initialize();
}

Application::~Application() {
    cleanup();
    s_instance = nullptr;
    std::cout << "Application destroyed" << std::endl;
}

void Application::run() {
    std::cout << "Starting main loop..." << std::endl;
    m_running = true;
    m_lastFrameTime = static_cast<float>(glfwGetTime());
    
    // Main game loop
    while (m_running && !m_window->shouldClose()) {
        // Calculate delta time
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;
        
        // Update
        update(deltaTime);
        
        // Render
        render();
        
        // Swap buffers and poll events
        m_window->update();
    }
    
    std::cout << "Main loop ended" << std::endl;
}

void Application::shutdown() {
    std::cout << "Shutdown requested" << std::endl;
    m_running = false;
}

void Application::initialize() {
    std::cout << "Initializing application..." << std::endl;
    
    // Initialize renderer
    if (!m_renderer->initialize()) {
        throw std::runtime_error("Failed to initialize renderer");
    }
    
    // Set up input callbacks
    m_window->setKeyCallback([this](int key, int action) {
        // Handle ESC key to exit
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            shutdown();
        }
        
        // Forward to input handler
        m_inputHandler->handleKey(key, action);
    });
    
    m_window->setMouseCallback([this](double xpos, double ypos) {
        m_inputHandler->handleMouse(xpos, ypos);
    });
    
    m_window->setResizeCallback([this](int width, int height) {
        m_renderer->setViewport(0, 0, width, height);
    });
    
    // Set initial viewport
    m_renderer->setViewport(0, 0, m_window->getWidth(), m_window->getHeight());
    
    std::cout << "Application initialized successfully" << std::endl;
}

void Application::update(float deltaTime) {
    // Update game client
    m_gameClient->update(deltaTime);
    
    // Can add more update logic here
}

void Application::render() {
    // Clear screen
    m_renderer->clear(glm::vec4(0.01f, 0.01f, 0.05f, 1.0f));
    
    // Begin rendering
    m_renderer->beginFrame();
    
    // Create a camera for rendering
    static Camera camera(45.0f, m_window->getAspectRatio(), 0.1f, 10000.0f);
    camera.setAspectRatio(m_window->getAspectRatio());
    camera.update(0.016f);
    
    // Render scene
    m_renderer->renderScene(camera);
    
    // End rendering
    m_renderer->endFrame();
}

void Application::cleanup() {
    std::cout << "Cleaning up application..." << std::endl;
    
    // Disconnect from server if connected
    if (m_gameClient) {
        m_gameClient->disconnect();
    }
    
    std::cout << "Cleanup complete" << std::endl;
}

} // namespace eve
