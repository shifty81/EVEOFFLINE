#pragma once

#include <string>
#include <glm/glm.hpp>

namespace eve {

/**
 * Health structure for entities
 * Represents shield, armor, and hull hit points
 */
struct Health {
    int shield{0};
    int armor{0};
    int hull{0};
    int maxShield{0};
    int maxArmor{0};
    int maxHull{0};

    Health() = default;
    Health(int s, int a, int h) : shield(s), armor(a), hull(h), maxShield(s), maxArmor(a), maxHull(h) {}
};

/**
 * Client-side entity representation
 * Stores entity state with interpolation support for smooth rendering
 */
class Entity {
public:
    Entity(const std::string& id);
    ~Entity() = default;

    // Getters
    const std::string& getId() const { return m_id; }
    glm::vec3 getPosition() const { return m_position; }
    glm::vec3 getVelocity() const { return m_velocity; }
    float getRotation() const { return m_rotation; }
    const Health& getHealth() const { return m_health; }
    
    // Ship info
    const std::string& getShipType() const { return m_shipType; }
    const std::string& getShipName() const { return m_shipName; }
    const std::string& getFaction() const { return m_faction; }
    
    // State
    bool isAlive() const { return m_health.hull > 0; }
    bool needsUpdate() const { return m_needsUpdate; }
    void clearUpdateFlag() { m_needsUpdate = false; }

    /**
     * Update entity from server spawn message
     * Called when entity first appears
     */
    void updateFromSpawn(const glm::vec3& position, const Health& health,
                         const std::string& shipType = "",
                         const std::string& shipName = "",
                         const std::string& faction = "");

    /**
     * Update entity from server state update
     * Sets target state for interpolation
     */
    void updateFromState(const glm::vec3& position, const glm::vec3& velocity,
                         float rotation, const Health& health);

    /**
     * Interpolate position towards target
     * Called every frame for smooth movement
     * @param deltaTime Time since last frame in seconds
     * @param interpolationTime Time window for interpolation (default 0.1s)
     */
    void interpolate(float deltaTime, float interpolationTime = 0.1f);

private:
    // Entity ID
    std::string m_id;

    // Current interpolated state (what we render)
    glm::vec3 m_position{0.0f};
    glm::vec3 m_velocity{0.0f};
    float m_rotation{0.0f};
    Health m_health;

    // Previous state (for interpolation)
    glm::vec3 m_prevPosition{0.0f};
    
    // Target state (from server)
    glm::vec3 m_targetPosition{0.0f};
    glm::vec3 m_targetVelocity{0.0f};
    float m_targetRotation{0.0f};
    
    // Interpolation tracking
    float m_interpolationProgress{0.0f};  // 0.0 to 1.0
    bool m_needsUpdate{false};

    // Ship information
    std::string m_shipType;
    std::string m_shipName;
    std::string m_faction;
};

} // namespace eve
