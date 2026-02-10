#ifndef EVE_DATA_NPC_DATABASE_H
#define EVE_DATA_NPC_DATABASE_H

#include <string>
#include <unordered_map>
#include <vector>

namespace eve {
namespace data {

/**
 * @brief NPC template loaded from JSON data files
 *
 * Represents the base stats for an NPC type, read from
 * data/npcs/pirates.json.  Used when spawning NPC entities
 * so that stats are data-driven rather than hard-coded.
 */
struct NpcTemplate {
    std::string id;
    std::string name;
    std::string type;          // "frigate", "cruiser", "battleship"
    std::string faction;

    // Health pools
    float hull_hp = 100.0f;
    float armor_hp = 100.0f;
    float shield_hp = 100.0f;

    // Navigation / AI
    float max_velocity = 300.0f;
    float orbit_distance = 1000.0f;
    float signature_radius = 35.0f;

    // Bounty
    double bounty = 0.0;

    // Behavior
    std::string behavior;      // "aggressive", "defensive"
    float awareness_range = 50000.0f;

    // Weapons
    struct WeaponData {
        std::string type;
        float damage = 0.0f;
        std::string damage_type;
        float optimal_range = 0.0f;
        float falloff_range = 0.0f;
        float rate_of_fire = 0.0f;
    };
    std::vector<WeaponData> weapons;

    // Resistances (stored as 0.0-1.0 fractions)
    struct Resistances {
        float em = 0.0f;
        float thermal = 0.0f;
        float kinetic = 0.0f;
        float explosive = 0.0f;
    };
    Resistances shield_resists;
    Resistances armor_resists;
    Resistances hull_resists;

    // Loot
    std::vector<std::string> loot_table;
};

/**
 * @brief Loads and stores NPC templates from JSON data files
 *
 * Reads NPC definitions from data/npcs/pirates.json at startup.
 * NPCs are looked up by their lowercase id.
 */
class NpcDatabase {
public:
    NpcDatabase() = default;

    /**
     * @brief Load NPC files from a directory
     * @param data_dir Path to the data/ directory (e.g. "../data")
     * @return Number of NPCs loaded
     */
    int loadFromDirectory(const std::string& data_dir);

    /**
     * @brief Load NPCs from a single JSON file
     * @param filepath Path to a JSON file
     * @return Number of NPCs loaded from this file
     */
    int loadFromFile(const std::string& filepath);

    /**
     * @brief Get an NPC template by id
     * @param npc_id Lowercase NPC id
     * @return Pointer to template, or nullptr if not found
     */
    const NpcTemplate* getNpc(const std::string& npc_id) const;

    /**
     * @brief Get all loaded NPC ids
     */
    std::vector<std::string> getNpcIds() const;

    /**
     * @brief Get total number of loaded NPCs
     */
    size_t getNpcCount() const { return npcs_.size(); }

private:
    std::unordered_map<std::string, NpcTemplate> npcs_;

    /**
     * @brief Parse a single NPC entry from JSON text
     */
    bool parseNpcEntry(const std::string& id, const std::string& json_block);

    // Lightweight JSON helpers (same pattern as ShipDatabase)
    static std::string extractString(const std::string& json, const std::string& key);
    static float extractFloat(const std::string& json, const std::string& key, float fallback = 0.0f);
    static int extractInt(const std::string& json, const std::string& key, int fallback = 0);
    static std::string extractBlock(const std::string& json, const std::string& key);
};

} // namespace data
} // namespace eve

#endif // EVE_DATA_NPC_DATABASE_H
