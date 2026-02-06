#include "ui/star_map.h"
#include "rendering/camera.h"
#include "rendering/shader.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <GL/glew.h>

using json = nlohmann::json;

namespace eve {

StarMap::StarMap()
    : m_visible(false)
    , m_viewMode(ViewMode::GALAXY)
    , m_systemVAO(0)
    , m_systemVBO(0)
    , m_lineVAO(0)
    , m_lineVBO(0)
    , m_minSecurity(0.0f)
    , m_maxSecurity(1.0f)
    , m_dragging(false)
    , m_lastMouseX(0)
    , m_lastMouseY(0)
    , m_selectedSystem(nullptr)
    , m_systemNodeSize(5.0f)
    , m_connectionWidth(1.0f)
    , m_highsecColor(0.2f, 1.0f, 0.2f, 1.0f)
    , m_lowsecColor(1.0f, 0.8f, 0.0f, 1.0f)
    , m_nullsecColor(1.0f, 0.2f, 0.2f, 1.0f)
    , m_routeColor(0.0f, 0.8f, 1.0f, 1.0f)
    , m_waypointColor(1.0f, 1.0f, 0.0f, 1.0f)
{
    // Create dedicated camera for star map with wider view
    m_mapCamera = std::make_unique<Camera>(60.0f, 16.0f / 9.0f, 0.1f, 100000.0f);
    m_mapCamera->setDistance(5000.0f);
}

StarMap::~StarMap() {
    if (m_systemVAO) {
        glDeleteVertexArrays(1, &m_systemVAO);
    }
    if (m_systemVBO) {
        glDeleteBuffers(1, &m_systemVBO);
    }
    if (m_lineVAO) {
        glDeleteVertexArrays(1, &m_lineVAO);
    }
    if (m_lineVBO) {
        glDeleteBuffers(1, &m_lineVBO);
    }
}

void StarMap::initialize(const std::string& universeDataPath) {
    loadUniverseData(universeDataPath);
    
    // Initialize rendering buffers
    glGenVertexArrays(1, &m_systemVAO);
    glGenBuffers(1, &m_systemVBO);
    
    glGenVertexArrays(1, &m_lineVAO);
    glGenBuffers(1, &m_lineVBO);
    
    // Setup vertex attributes for points (systems)
    glBindVertexArray(m_systemVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_systemVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Setup vertex attributes for lines (connections)
    glBindVertexArray(m_lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    std::cout << "[StarMap] Initialized with " << m_systems.size() << " systems" << std::endl;
}

void StarMap::loadUniverseData(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[StarMap] Failed to open universe data: " << path << std::endl;
        return;
    }
    
    json data;
    file >> data;
    
    if (data.contains("systems")) {
        for (const auto& systemData : data["systems"]) {
            SystemNode node;
            node.id = systemData["id"].get<std::string>();
            node.name = systemData["name"].get<std::string>();
            node.security = systemData["security"].get<float>();
            node.faction = systemData["faction"].get<std::string>();
            node.isCurrentSystem = false;
            node.isDestination = false;
            node.isWaypoint = false;
            
            // Parse coordinates
            if (systemData.contains("coordinates")) {
                auto coords = systemData["coordinates"];
                node.position = glm::vec3(
                    coords["x"].get<float>(),
                    coords["y"].get<float>(),
                    coords["z"].get<float>()
                );
            }
            
            // Parse gates (connections to other systems)
            if (systemData.contains("gates")) {
                for (const auto& gate : systemData["gates"]) {
                    node.connectedSystems.push_back(gate.get<std::string>());
                }
            }
            
            m_systems.push_back(node);
        }
    }
    
    // Set first system as current if none set
    if (!m_systems.empty() && m_currentSystemId.empty()) {
        m_currentSystemId = m_systems[0].id;
        m_systems[0].isCurrentSystem = true;
    }
}

void StarMap::loadSystemData(const std::string& systemId) {
    // Clear existing celestials
    m_celestials.clear();
    
    // Find the system
    auto it = std::find_if(m_systems.begin(), m_systems.end(),
                          [&systemId](const SystemNode& node) { return node.id == systemId; });
    
    if (it == m_systems.end()) {
        return;
    }
    
    // For now, generate some placeholder celestials
    // TODO: Load from actual system data files
    
    // Add a central star
    CelestialObject star;
    star.id = systemId + "_star";
    star.name = it->name + " (Star)";
    star.position = glm::vec3(0.0f, 0.0f, 0.0f);
    star.radius = 695000.0f; // Sun-like radius
    star.type = CelestialObject::STAR;
    m_celestials.push_back(star);
    
    // Add some planets in orbit
    for (int i = 0; i < 5; i++) {
        CelestialObject planet;
        planet.id = systemId + "_planet_" + std::to_string(i);
        planet.name = it->name + " " + std::to_string(i + 1);
        float angle = (float)i / 5.0f * 2.0f * 3.14159f;
        float distance = 5000000.0f * (i + 1);
        planet.position = glm::vec3(
            cos(angle) * distance,
            (float)(rand() % 1000000 - 500000),
            sin(angle) * distance
        );
        planet.radius = 6371.0f * (1.0f + (float)(rand() % 100) / 100.0f);
        planet.type = CelestialObject::PLANET;
        m_celestials.push_back(planet);
    }
}

void StarMap::update(float deltaTime) {
    if (!m_visible) {
        return;
    }
    
    m_mapCamera->update(deltaTime);
}

void StarMap::render() {
    if (!m_visible) {
        return;
    }
    
    // Save previous OpenGL state
    GLboolean depthTest;
    glGetBooleanv(GL_DEPTH_TEST, &depthTest);
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    switch (m_viewMode) {
        case ViewMode::GALAXY:
            renderGalaxyView();
            break;
        case ViewMode::SOLAR_SYSTEM:
            renderSystemView();
            break;
        case ViewMode::TACTICAL:
            renderTacticalOverlay();
            break;
    }
    
    // Restore state
    if (!depthTest) {
        glDisable(GL_DEPTH_TEST);
    }
}

void StarMap::renderGalaxyView() {
    std::vector<float> vertices;
    std::vector<float> lines;
    
    // Build vertex data for systems
    for (const auto& system : m_systems) {
        // Filter by security if needed
        if (system.security < m_minSecurity || system.security > m_maxSecurity) {
            continue;
        }
        
        if (!m_factionFilter.empty() && system.faction != m_factionFilter) {
            continue;
        }
        
        // Determine color based on security status
        glm::vec4 color;
        if (system.security >= 0.5f) {
            color = m_highsecColor;
        } else if (system.security >= 0.1f) {
            color = m_lowsecColor;
        } else {
            color = m_nullsecColor;
        }
        
        // Highlight current system
        if (system.isCurrentSystem) {
            color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
        
        // Highlight destination
        if (system.isDestination) {
            color = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f);
        }
        
        // Add vertex: position (3) + color (4)
        vertices.push_back(system.position.x);
        vertices.push_back(system.position.y);
        vertices.push_back(system.position.z);
        vertices.push_back(color.r);
        vertices.push_back(color.g);
        vertices.push_back(color.b);
        vertices.push_back(color.a);
    }
    
    // Build line data for connections
    for (const auto& system : m_systems) {
        for (const auto& connectedId : system.connectedSystems) {
            auto it = std::find_if(m_systems.begin(), m_systems.end(),
                                  [&connectedId](const SystemNode& node) { return node.id == connectedId; });
            
            if (it != m_systems.end()) {
                glm::vec4 lineColor(0.3f, 0.3f, 0.3f, 0.5f);
                
                // From point
                lines.push_back(system.position.x);
                lines.push_back(system.position.y);
                lines.push_back(system.position.z);
                lines.push_back(lineColor.r);
                lines.push_back(lineColor.g);
                lines.push_back(lineColor.b);
                lines.push_back(lineColor.a);
                
                // To point
                lines.push_back(it->position.x);
                lines.push_back(it->position.y);
                lines.push_back(it->position.z);
                lines.push_back(lineColor.r);
                lines.push_back(lineColor.g);
                lines.push_back(lineColor.b);
                lines.push_back(lineColor.a);
            }
        }
    }
    
    // TODO: Use actual shader program when available
    // For now, use fixed-function pipeline or basic shader
    
    // Render connections (lines)
    if (!lines.empty()) {
        glBindVertexArray(m_lineVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_lineVBO);
        glBufferData(GL_ARRAY_BUFFER, lines.size() * sizeof(float), lines.data(), GL_DYNAMIC_DRAW);
        glLineWidth(m_connectionWidth);
        glDrawArrays(GL_LINES, 0, lines.size() / 7);
    }
    
    // Render systems (points)
    if (!vertices.empty()) {
        glBindVertexArray(m_systemVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_systemVBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glPointSize(m_systemNodeSize);
        glDrawArrays(GL_POINTS, 0, vertices.size() / 7);
    }
    
    glBindVertexArray(0);
}

void StarMap::renderSystemView() {
    // Render celestial objects in the selected system
    // TODO: Implement detailed system view
    
    std::cout << "[StarMap] Rendering system view for: " << m_currentSystemId << std::endl;
}

void StarMap::renderTacticalOverlay() {
    // Render tactical overlay with range circles
    // TODO: Implement tactical overlay
    
    std::cout << "[StarMap] Rendering tactical overlay" << std::endl;
}

void StarMap::toggle() {
    m_visible = !m_visible;
}

void StarMap::setViewMode(ViewMode mode) {
    m_viewMode = mode;
    
    if (mode == ViewMode::SOLAR_SYSTEM && !m_currentSystemId.empty()) {
        loadSystemData(m_currentSystemId);
    }
}

void StarMap::setCurrentSystem(const std::string& systemId) {
    // Clear previous current system
    for (auto& system : m_systems) {
        if (system.isCurrentSystem) {
            system.isCurrentSystem = false;
        }
        if (system.id == systemId) {
            system.isCurrentSystem = true;
        }
    }
    
    m_currentSystemId = systemId;
}

void StarMap::focusOnSystem(const std::string& systemId) {
    auto it = std::find_if(m_systems.begin(), m_systems.end(),
                          [&systemId](const SystemNode& node) { return node.id == systemId; });
    
    if (it != m_systems.end()) {
        m_mapCamera->setTarget(it->position);
        m_selectedSystem = &(*it);
    }
}

void StarMap::setDestination(const std::string& systemId) {
    // Clear previous destination
    for (auto& system : m_systems) {
        if (system.isDestination) {
            system.isDestination = false;
        }
        if (system.id == systemId) {
            system.isDestination = true;
        }
    }
    
    m_destinationSystemId = systemId;
    calculateRoute();
}

void StarMap::addWaypoint(const std::string& systemId) {
    m_waypoints.push_back(systemId);
    
    auto it = std::find_if(m_systems.begin(), m_systems.end(),
                          [&systemId](const SystemNode& node) { return node.id == systemId; });
    
    if (it != m_systems.end()) {
        it->isWaypoint = true;
    }
}

void StarMap::clearWaypoints() {
    for (auto& system : m_systems) {
        system.isWaypoint = false;
    }
    m_waypoints.clear();
}

void StarMap::calculateRoute() {
    m_route.clear();
    
    if (m_currentSystemId.empty() || m_destinationSystemId.empty()) {
        return;
    }
    
    // Use BFS to find shortest path
    std::unordered_map<std::string, std::string> parent;
    std::unordered_set<std::string> visited;
    std::queue<std::string> queue;
    
    queue.push(m_currentSystemId);
    visited.insert(m_currentSystemId);
    
    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        
        if (current == m_destinationSystemId) {
            // Reconstruct path
            std::string node = m_destinationSystemId;
            while (node != m_currentSystemId) {
                m_route.push_back(node);
                node = parent[node];
            }
            std::reverse(m_route.begin(), m_route.end());
            break;
        }
        
        // Find system node
        auto it = std::find_if(m_systems.begin(), m_systems.end(),
                              [&current](const SystemNode& node) { return node.id == current; });
        
        if (it != m_systems.end()) {
            for (const auto& neighbor : it->connectedSystems) {
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    parent[neighbor] = current;
                    queue.push(neighbor);
                }
            }
        }
    }
}

std::vector<std::string> StarMap::getRouteToDestination() const {
    return m_route;
}

int StarMap::getJumpsToDestination() const {
    return m_route.size();
}

void StarMap::zoomMap(float delta) {
    m_mapCamera->zoom(delta);
}

void StarMap::rotateMap(float deltaYaw, float deltaPitch) {
    m_mapCamera->rotate(deltaYaw, deltaPitch);
}

void StarMap::panMap(float deltaX, float deltaY) {
    m_mapCamera->pan(deltaX, deltaY);
}

void StarMap::resetCamera() {
    m_mapCamera->setDistance(5000.0f);
    m_mapCamera->setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
}

void StarMap::handleMouseClick(int x, int y) {
    // TODO: Implement ray picking to select systems
    std::cout << "[StarMap] Mouse click at: " << x << ", " << y << std::endl;
}

void StarMap::handleMouseMove(int x, int y) {
    if (m_dragging) {
        float deltaX = x - m_lastMouseX;
        float deltaY = y - m_lastMouseY;
        
        rotateMap(deltaX * 0.5f, -deltaY * 0.5f);
    }
    
    m_lastMouseX = x;
    m_lastMouseY = y;
}

void StarMap::handleMouseScroll(float delta) {
    zoomMap(delta);
}

void StarMap::setSecurityFilter(float minSec, float maxSec) {
    m_minSecurity = minSec;
    m_maxSecurity = maxSec;
}

void StarMap::setFactionFilter(const std::string& faction) {
    m_factionFilter = faction;
}

void StarMap::clearFilters() {
    m_minSecurity = 0.0f;
    m_maxSecurity = 1.0f;
    m_factionFilter.clear();
}

} // namespace eve
