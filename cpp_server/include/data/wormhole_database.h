#ifndef EVE_DATA_WORMHOLE_DATABASE_H
#define EVE_DATA_WORMHOLE_DATABASE_H

#include <string>
#include <unordered_map>
#include <vector>

namespace eve {
namespace data {

/**
 * @brief Sleeper NPC spawn definition within a wormhole class
 */
struct SleeperSpawn {
    std::string id;
    std::string name;
    std::string type;     // "frigate", "cruiser", "battleship"
    int count_min = 1;
    int count_max = 2;
};

/**
 * @brief Template for a wormhole class (C1-C6)
 *
 * Loaded from data/wormholes/wormhole_classes.json.
 */
struct WormholeClassTemplate {
    std::string id;                     // e.g. "c1"
    std::string name;                   // e.g. "Class 1 Wormhole"
    int wormhole_class = 1;             // 1-6
    std::string difficulty;             // "easy" .. "extreme"
    std::string description;
    std::string max_ship_class;         // e.g. "Battlecruiser", "Battleship", "Capital"
    double max_ship_mass = 20000000.0;
    std::vector<std::string> static_connections;
    double max_wormhole_stability = 500000000.0;
    float max_wormhole_lifetime_hours = 24.0f;
    std::vector<SleeperSpawn> sleeper_spawns;
    float salvage_value_multiplier = 1.0f;
    double blue_loot_isk = 150000.0;
};

/**
 * @brief System-wide wormhole effect modifier set
 */
struct WormholeEffect {
    std::string id;                     // e.g. "magnetar"
    std::string name;
    std::string description;
    std::unordered_map<std::string, float> modifiers;  // stat -> multiplier
};

/**
 * @brief Loads wormhole class templates and effects from JSON data files
 */
class WormholeDatabase {
public:
    WormholeDatabase() = default;

    /**
     * @brief Load wormhole class templates from data directory
     * @param data_dir Path to the data/ directory (e.g. "../data")
     * @return Number of wormhole classes loaded
     */
    int loadFromDirectory(const std::string& data_dir);

    /**
     * @brief Get a wormhole class template by id
     * @param class_id e.g. "c1", "c3", "c6"
     * @return Pointer to template, or nullptr if not found
     */
    const WormholeClassTemplate* getWormholeClass(const std::string& class_id) const;

    /**
     * @brief Get a wormhole effect by id
     * @param effect_id e.g. "magnetar", "pulsar"
     * @return Pointer to effect, or nullptr if not found
     */
    const WormholeEffect* getEffect(const std::string& effect_id) const;

    /**
     * @brief Get all loaded wormhole class ids
     */
    std::vector<std::string> getClassIds() const;

    /**
     * @brief Get all loaded effect ids
     */
    std::vector<std::string> getEffectIds() const;

    /**
     * @brief Get total number of loaded wormhole classes
     */
    size_t getClassCount() const { return classes_.size(); }

    /**
     * @brief Get total number of loaded effects
     */
    size_t getEffectCount() const { return effects_.size(); }

private:
    std::unordered_map<std::string, WormholeClassTemplate> classes_;
    std::unordered_map<std::string, WormholeEffect> effects_;

    int loadClasses(const std::string& filepath);
    int loadEffects(const std::string& filepath);

    // Lightweight JSON helpers (reused from ShipDatabase pattern)
    static std::string extractString(const std::string& json, const std::string& key);
    static float extractFloat(const std::string& json, const std::string& key, float fallback = 0.0f);
    static int extractInt(const std::string& json, const std::string& key, int fallback = 0);
    static std::string extractBlock(const std::string& json, const std::string& key);
    static std::string extractArray(const std::string& json, const std::string& key);
    static std::vector<std::string> parseStringArray(const std::string& arr);
};

} // namespace data
} // namespace eve

#endif // EVE_DATA_WORMHOLE_DATABASE_H
