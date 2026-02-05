#pragma once

#include "core/entity_manager.h"
#include <nlohmann/json.hpp>
#include <string>

namespace eve {

/**
 * Helper class to parse entity-related network messages
 * Bridges between JSON protocol and EntityManager
 */
class EntityMessageParser {
public:
    /**
     * Parse SPAWN_ENTITY message
     * @param dataJson JSON string containing entity data
     * @param entityManager EntityManager to spawn entity in
     * @return true if parsed successfully
     */
    static bool parseSpawnEntity(const std::string& dataJson, EntityManager& entityManager);

    /**
     * Parse DESTROY_ENTITY message
     * @param dataJson JSON string containing entity_id
     * @param entityManager EntityManager to destroy entity in
     * @return true if parsed successfully
     */
    static bool parseDestroyEntity(const std::string& dataJson, EntityManager& entityManager);

    /**
     * Parse STATE_UPDATE message
     * @param dataJson JSON string containing entities array
     * @param entityManager EntityManager to update entities in
     * @return true if parsed successfully
     */
    static bool parseStateUpdate(const std::string& dataJson, EntityManager& entityManager);

private:
    // Helper to parse position from JSON
    static glm::vec3 parsePosition(const nlohmann::json& posJson);
    
    // Helper to parse health from JSON
    static Health parseHealth(const nlohmann::json& healthJson);
};

} // namespace eve
