#include "core/entity_message_parser.h"
#include <iostream>

namespace eve {

glm::vec3 EntityMessageParser::parsePosition(const nlohmann::json& posJson) {
    float x = posJson.value("x", 0.0f);
    float y = posJson.value("y", 0.0f);
    float z = posJson.value("z", 0.0f);
    return glm::vec3(x, y, z);
}

Health EntityMessageParser::parseHealth(const nlohmann::json& healthJson) {
    Health health;
    
    // Handle both full field names and abbreviated versions
    health.shield = healthJson.value("shield", healthJson.value("s", 0));
    health.armor = healthJson.value("armor", healthJson.value("a", 0));
    health.hull = healthJson.value("hull", healthJson.value("h", 0));
    
    // Max values (might not be in update, use current as max if not specified)
    health.maxShield = healthJson.value("max_shield", health.shield);
    health.maxArmor = healthJson.value("max_armor", health.armor);
    health.maxHull = healthJson.value("max_hull", health.hull);
    
    return health;
}

Capacitor EntityMessageParser::parseCapacitor(const nlohmann::json& capacitorJson) {
    Capacitor capacitor;
    capacitor.current = capacitorJson.value("current", 0.0f);
    capacitor.max = capacitorJson.value("max", 0.0f);
    return capacitor;
}

bool EntityMessageParser::parseSpawnEntity(const std::string& dataJson, EntityManager& entityManager) {
    try {
        auto data = nlohmann::json::parse(dataJson);
        
        // Extract entity ID
        std::string entityId = data.value("entity_id", "");
        if (entityId.empty()) {
            std::cerr << "SPAWN_ENTITY missing entity_id" << std::endl;
            return false;
        }
        
        // Extract position
        glm::vec3 position{0.0f};
        if (data.contains("position")) {
            position = parsePosition(data["position"]);
        }
        
        // Extract health
        Health health;
        if (data.contains("health")) {
            health = parseHealth(data["health"]);
        }
        
        // Extract capacitor
        Capacitor capacitor;
        if (data.contains("capacitor")) {
            capacitor = parseCapacitor(data["capacitor"]);
        }
        
        // Extract optional ship info
        std::string shipType = data.value("ship_type", "");
        std::string shipName = data.value("ship_name", "");
        std::string faction = data.value("faction", "");
        
        // Spawn entity
        entityManager.spawnEntity(entityId, position, health, capacitor, shipType, shipName, faction);
        return true;
        
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Failed to parse SPAWN_ENTITY: " << e.what() << std::endl;
        return false;
    }
}

bool EntityMessageParser::parseDestroyEntity(const std::string& dataJson, EntityManager& entityManager) {
    try {
        auto data = nlohmann::json::parse(dataJson);
        
        // Extract entity ID
        std::string entityId = data.value("entity_id", "");
        if (entityId.empty()) {
            std::cerr << "DESTROY_ENTITY missing entity_id" << std::endl;
            return false;
        }
        
        // Destroy entity
        entityManager.destroyEntity(entityId);
        return true;
        
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Failed to parse DESTROY_ENTITY: " << e.what() << std::endl;
        return false;
    }
}

bool EntityMessageParser::parseStateUpdate(const std::string& dataJson, EntityManager& entityManager) {
    try {
        auto data = nlohmann::json::parse(dataJson);
        
        // Extract snapshot metadata (for future packet loss detection and timing)
        // TODO: Use these values for interpolation delay calculation and dropped packet detection
        uint64_t sequence = data.value("sequence", 0ULL);
        uint64_t timestamp = data.value("timestamp", 0ULL);
        (void)sequence;   // Suppress unused variable warning
        (void)timestamp;  // Suppress unused variable warning
        
        // Extract entities array
        if (!data.contains("entities") || !data["entities"].is_array()) {
            std::cerr << "STATE_UPDATE missing entities array" << std::endl;
            return false;
        }
        
        auto entitiesArray = data["entities"];
        std::vector<std::string> entityIds;
        
        // Process each entity
        for (const auto& entityData : entitiesArray) {
            // Extract entity ID
            std::string entityId = entityData.value("id", "");
            if (entityId.empty()) {
                continue;
            }
            
            entityIds.push_back(entityId);
            
            // Extract position
            glm::vec3 position{0.0f};
            if (entityData.contains("pos")) {
                position = parsePosition(entityData["pos"]);
            }
            
            // Extract velocity
            glm::vec3 velocity{0.0f};
            if (entityData.contains("vel")) {
                const auto& velJson = entityData["vel"];
                velocity.x = velJson.value("vx", 0.0f);
                velocity.y = velJson.value("vy", 0.0f);
                velocity.z = velJson.value("vz", 0.0f);
            }
            
            // Extract rotation
            float rotation = 0.0f;
            if (entityData.contains("pos") && entityData["pos"].contains("rot")) {
                rotation = entityData["pos"].value("rot", 0.0f);
            }
            
            // Extract health
            Health health;
            if (entityData.contains("health")) {
                health = parseHealth(entityData["health"]);
            }
            
            // Extract capacitor
            Capacitor capacitor;
            if (entityData.contains("capacitor")) {
                capacitor = parseCapacitor(entityData["capacitor"]);
            }
            
            // Extract ship info (needed for correct model selection)
            std::string shipType = entityData.value("ship_type", "");
            std::string shipName = entityData.value("ship_name", "");
            std::string faction = entityData.value("faction", "");
            
            // Update entity state
            entityManager.updateEntityState(entityId, position, velocity, rotation, health, capacitor,
                                            shipType, shipName, faction);
        }
        
        // Process state update (remove entities not in update)
        entityManager.processStateUpdate(entityIds);
        return true;
        
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Failed to parse STATE_UPDATE: " << e.what() << std::endl;
        return false;
    }
}

} // namespace eve
