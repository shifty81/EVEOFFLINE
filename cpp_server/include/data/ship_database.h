#ifndef EVE_DATA_SHIP_DATABASE_H
#define EVE_DATA_SHIP_DATABASE_H

#include <string>
#include <unordered_map>
#include <vector>

namespace atlas {
namespace data {

/**
 * @brief Ship template loaded from JSON data files
 *
 * Represents the base stats for a ship type, read from
 * data/ships/*.json.  Used when creating player or NPC entities
 * so that stats are data-driven rather than hard-coded.
 */
struct ShipTemplate {
    std::string id;
    std::string name;
    std::string ship_class;   // e.g. "Frigate", "Cruiser"
    std::string race;         // e.g. "Keldari", "Veyren"
    std::string description;

    // Health pools
    float hull_hp = 100.0f;
    float armor_hp = 100.0f;
    float shield_hp = 100.0f;

    // Capacitor
    float capacitor = 100.0f;
    float capacitor_recharge_time = 200.0f;  // seconds

    // Fitting
    float cpu = 100.0f;
    float powergrid = 50.0f;
    int high_slots = 3;
    int mid_slots = 3;
    int low_slots = 3;
    int rig_slots = 3;

    // Navigation
    float max_velocity = 300.0f;
    float inertia_modifier = 3.0f;
    float cargo_capacity = 100.0f;

    // Targeting
    float signature_radius = 35.0f;
    float scan_resolution = 400.0f;
    int max_locked_targets = 3;
    float max_targeting_range = 20000.0f;
    float shield_recharge_time = 625.0f;

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

    // Model generation data (for procedural ship model creation)
    struct ModelData {
        int turret_hardpoints = 0;
        int launcher_hardpoints = 0;
        int drone_bays = 0;
        int engine_count = 2;
        int generation_seed = 0;
        bool has_model_data = false;  // true when model_data block exists in JSON
    };
    ModelData model_data;
};

/**
 * @brief Loads and stores ship templates from JSON data files
 *
 * Reads ship definitions from data/ships/*.json at startup.
 * Ships are looked up by their lowercase id (e.g. "rifter").
 */
class ShipDatabase {
public:
    ShipDatabase() = default;

    /**
     * @brief Load all ship files from a directory
     * @param data_dir Path to the data/ directory (e.g. "../data")
     * @return Number of ships loaded
     */
    int loadFromDirectory(const std::string& data_dir);

    /**
     * @brief Load ships from a single JSON file
     * @param filepath Path to a JSON file (e.g. "data/ships/frigates.json")
     * @return Number of ships loaded from this file
     */
    int loadFromFile(const std::string& filepath);

    /**
     * @brief Get a ship template by id
     * @param ship_id Lowercase ship id (e.g. "rifter")
     * @return Pointer to template, or nullptr if not found
     */
    const ShipTemplate* getShip(const std::string& ship_id) const;

    /**
     * @brief Get all loaded ship ids
     */
    std::vector<std::string> getShipIds() const;

    /**
     * @brief Get total number of loaded ships
     */
    size_t getShipCount() const { return ships_.size(); }

private:
    std::unordered_map<std::string, ShipTemplate> ships_;

    /**
     * @brief Parse a single ship entry from JSON text
     * @param id Ship id key
     * @param json_block The JSON block for this ship
     * @return true on success
     */
    bool parseShipEntry(const std::string& id, const std::string& json_block);

    // Lightweight JSON helpers
    static std::string extractString(const std::string& json, const std::string& key);
    static float extractFloat(const std::string& json, const std::string& key, float fallback = 0.0f);
    static int extractInt(const std::string& json, const std::string& key, int fallback = 0);
    static std::string extractBlock(const std::string& json, const std::string& key);
};

} // namespace data
} // namespace atlas

#endif // EVE_DATA_SHIP_DATABASE_H
