#ifndef EVE_DATA_WORLD_PERSISTENCE_H
#define EVE_DATA_WORLD_PERSISTENCE_H

#include "ecs/world.h"
#include <string>

namespace atlas {
namespace data {

/**
 * @brief Serializes and deserializes world state for persistent worlds
 *
 * Saves all entity data (position, velocity, health, capacitor, ship,
 * faction, AI, weapon, target, wormhole, fleet membership, station,
 * docked, wreck, captain personality, fleet morale, captain relationship,
 * emotional state, captain memory, fleet formation, fleet cargo pool,
 * rumor log, mineral deposit, system resources, market hub) to a JSON
 * file and restores it on load.
 */
class WorldPersistence {
public:
    WorldPersistence() = default;
    ~WorldPersistence() = default;

    /// Save the entire world state to a JSON file.
    /// @return true on success
    bool saveWorld(const ecs::World* world, const std::string& filepath);

    /// Load world state from a JSON file, creating entities and components.
    /// Existing entities are NOT cleared – call world->destroyEntity() first
    /// if a clean reload is desired.
    /// @return true on success, false on file-not-found or parse error
    bool loadWorld(ecs::World* world, const std::string& filepath);

    /// Serialize world state to a JSON string (useful for tests and network).
    std::string serializeWorld(const ecs::World* world) const;

    /// Deserialize a JSON string into the world.
    bool deserializeWorld(ecs::World* world, const std::string& json) const;

private:
    /// Serialize a single entity to a JSON object string.
    std::string serializeEntity(const ecs::Entity* entity) const;

    /// Deserialize a single entity JSON object and create it in the world.
    bool deserializeEntity(ecs::World* world, const std::string& json) const;

    // Lightweight JSON helpers
    static std::string extractString(const std::string& json, const std::string& key);
    static float extractFloat(const std::string& json, const std::string& key, float fallback = 0.0f);
    static int extractInt(const std::string& json, const std::string& key, int fallback = 0);
    static double extractDouble(const std::string& json, const std::string& key, double fallback = 0.0);
    static bool extractBool(const std::string& json, const std::string& key, bool fallback = false);
    static std::string extractObject(const std::string& json, const std::string& key);
};

} // namespace data
} // namespace atlas

#endif // EVE_DATA_WORLD_PERSISTENCE_H
