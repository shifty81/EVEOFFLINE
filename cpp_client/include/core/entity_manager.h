#pragma once

#include "core/entity.h"
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <functional>

namespace eve {

/**
 * Client-side entity manager
 * Handles entity lifecycle (spawn, update, destroy) from server messages
 * Manages entity interpolation for smooth rendering
 */
class EntityManager {
public:
    // Callback types
    using EntityCallback = std::function<void(const std::shared_ptr<Entity>&)>;

    EntityManager();
    ~EntityManager() = default;

    /**
     * Spawn a new entity
     * Called when receiving SPAWN_ENTITY message from server
     */
    void spawnEntity(const std::string& id, const glm::vec3& position,
                     const Health& health, const Capacitor& capacitor = Capacitor(),
                     const std::string& shipType = "",
                     const std::string& shipName = "", const std::string& faction = "");

    /**
     * Destroy an entity
     * Called when receiving DESTROY_ENTITY message from server
     */
    void destroyEntity(const std::string& id);

    /**
     * Update entity state from server
     * Called when processing STATE_UPDATE message
     */
    void updateEntityState(const std::string& id, const glm::vec3& position,
                           const glm::vec3& velocity, float rotation, const Health& health,
                           const Capacitor& capacitor = Capacitor(),
                           const std::string& shipType = "",
                           const std::string& shipName = "",
                           const std::string& faction = "");

    /**
     * Process state update message
     * Updates all entities and removes those not in the update
     * @param entityIds Set of entity IDs in the update
     */
    void processStateUpdate(const std::vector<std::string>& entityIds);

    /**
     * Update all entities (interpolation)
     * Should be called every frame
     */
    void update(float deltaTime);

    /**
     * Get entity by ID
     */
    std::shared_ptr<Entity> getEntity(const std::string& id) const;

    /**
     * Get all entities
     */
    const std::unordered_map<std::string, std::shared_ptr<Entity>>& getAllEntities() const {
        return m_entities;
    }

    /**
     * Get entity count
     */
    size_t getEntityCount() const { return m_entities.size(); }

    /**
     * Clear all entities
     */
    void clear();

    /**
     * Register callbacks for entity events
     */
    void setOnEntitySpawned(EntityCallback callback) { m_onEntitySpawned = callback; }
    void setOnEntityDestroyed(EntityCallback callback) { m_onEntityDestroyed = callback; }
    void setOnEntityUpdated(EntityCallback callback) { m_onEntityUpdated = callback; }

private:
    // Entity storage
    std::unordered_map<std::string, std::shared_ptr<Entity>> m_entities;

    // Callbacks
    EntityCallback m_onEntitySpawned;
    EntityCallback m_onEntityDestroyed;
    EntityCallback m_onEntityUpdated;
};

} // namespace eve
