#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace eve {

// Forward declarations
class Camera;
class Shader;

/**
 * 3D Interactive Star Map for EVE OFFLINE
 * 
 * Mimics EVE Online's F10 map interface with:
 * - Galaxy view showing all solar systems
 * - Solar system view showing celestials
 * - Interactive 3D navigation (zoom, pan, rotate)
 * - Route planning and waypoints
 * - Data filtering and visualization
 */
class StarMap {
public:
    enum class ViewMode {
        GALAXY,         // Show all star systems
        SOLAR_SYSTEM,   // Show single system with planets, stations, belts
        TACTICAL        // In-space tactical overlay
    };

    struct SystemNode {
        std::string id;
        std::string name;
        glm::vec3 position;
        float security;
        std::string faction;
        bool isCurrentSystem;
        bool isDestination;
        bool isWaypoint;
        std::vector<std::string> connectedSystems;
    };

    struct CelestialObject {
        std::string id;
        std::string name;
        glm::vec3 position;
        float radius;
        enum Type {
            STAR,
            PLANET,
            MOON,
            STATION,
            ASTEROID_BELT,
            STARGATE,
            WORMHOLE
        } type;
    };

    StarMap();
    ~StarMap();

    /**
     * Initialize star map with universe data
     */
    void initialize(const std::string& universeDataPath);

    /**
     * Update star map state
     */
    void update(float deltaTime);

    /**
     * Render star map to screen
     */
    void render();

    /**
     * Toggle star map visibility
     */
    void toggle();
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    /**
     * View mode control
     */
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const { return m_viewMode; }

    /**
     * System navigation
     */
    void setCurrentSystem(const std::string& systemId);
    void focusOnSystem(const std::string& systemId);
    void setDestination(const std::string& systemId);
    void addWaypoint(const std::string& systemId);
    void clearWaypoints();

    /**
     * Camera controls for map
     */
    void zoomMap(float delta);
    void rotateMap(float deltaYaw, float deltaPitch);
    void panMap(float deltaX, float deltaY);
    void resetCamera();

    /**
     * Mouse interaction
     */
    void handleMouseClick(int x, int y);
    void handleMouseRelease(int x, int y);
    void handleMouseMove(int x, int y);
    void handleMouseScroll(float delta);

    /**
     * Route planning
     */
    std::vector<std::string> getRouteToDestination() const;
    int getJumpsToDestination() const;

    /**
     * Data filtering
     */
    void setSecurityFilter(float minSec, float maxSec);
    void setFactionFilter(const std::string& faction);
    void clearFilters();

private:
    void loadUniverseData(const std::string& path);
    void loadSystemData(const std::string& systemId);
    void calculateRoute();
    void renderGalaxyView();
    void renderSystemView();
    void renderTacticalOverlay();
    void renderSystemNode(const SystemNode& node);
    void renderConnection(const SystemNode& from, const SystemNode& to);
    void renderCelestial(const CelestialObject& obj);
    void renderRangeCircles();
    void renderRouteLines();
    SystemNode* findSystemAtScreenPos(int x, int y);

    // State
    bool m_visible;
    ViewMode m_viewMode;
    std::string m_currentSystemId;
    std::string m_destinationSystemId;
    std::vector<std::string> m_waypoints;
    std::vector<std::string> m_route;

    // Data
    std::vector<SystemNode> m_systems;
    std::vector<CelestialObject> m_celestials;
    
    // Camera for map (separate from main game camera)
    std::unique_ptr<Camera> m_mapCamera;
    
    // Rendering
    std::unique_ptr<Shader> m_mapShader;
    unsigned int m_systemVAO;
    unsigned int m_systemVBO;
    unsigned int m_lineVAO;
    unsigned int m_lineVBO;
    
    // Filters
    float m_minSecurity;
    float m_maxSecurity;
    std::string m_factionFilter;

    // Interaction
    bool m_dragging;
    int m_lastMouseX;
    int m_lastMouseY;
    SystemNode* m_selectedSystem;

    // Visual settings
    float m_systemNodeSize;
    float m_connectionWidth;
    glm::vec4 m_highsecColor;
    glm::vec4 m_lowsecColor;
    glm::vec4 m_nullsecColor;
    glm::vec4 m_routeColor;
    glm::vec4 m_waypointColor;
};

} // namespace eve
