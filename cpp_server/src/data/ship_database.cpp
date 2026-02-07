#include "data/ship_database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace eve {
namespace data {

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

int ShipDatabase::loadFromDirectory(const std::string& data_dir) {
    // Ship JSON files located under data_dir/ships/
    const std::string ship_dir = data_dir + "/ships/";
    const char* files[] = {
        "frigates.json",
        "tech2_frigates.json",
        "destroyers.json",
        "cruisers.json",
        "tech2_cruisers.json",
        "battlecruisers.json",
        "battleships.json",
        "capitals.json",
        "exhumers.json",
        "mining_barges.json"
    };

    int total = 0;
    for (const char* file : files) {
        int count = loadFromFile(ship_dir + file);
        if (count > 0) {
            total += count;
        }
    }

    std::cout << "[ShipDatabase] Loaded " << total
              << " ship templates from " << ship_dir << std::endl;
    return total;
}

int ShipDatabase::loadFromFile(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        // Silently skip missing files (some may not exist yet)
        return 0;
    }

    std::stringstream buf;
    buf << ifs.rdbuf();
    std::string content = buf.str();

    // The top-level JSON is an object where each key is a ship id.
    // We do lightweight parsing: find each top-level key and its block.
    int loaded = 0;
    size_t pos = 0;

    while (pos < content.size()) {
        // Find next quoted key at top level (depth == 1 means inside root {})
        size_t key_start = content.find('\"', pos);
        if (key_start == std::string::npos) break;

        size_t key_end = content.find('\"', key_start + 1);
        if (key_end == std::string::npos) break;

        std::string key = content.substr(key_start + 1, key_end - key_start - 1);

        // Find the opening '{' of this ship's block
        size_t block_start = content.find('{', key_end);
        if (block_start == std::string::npos) break;

        // Walk forward counting braces to find the matching '}'
        int depth = 0;
        size_t block_end = block_start;
        bool in_string = false;
        for (size_t i = block_start; i < content.size(); ++i) {
            char c = content[i];
            if (c == '\\' && in_string) { ++i; continue; }
            if (c == '\"') { in_string = !in_string; continue; }
            if (in_string) continue;
            if (c == '{') ++depth;
            if (c == '}') {
                --depth;
                if (depth == 0) { block_end = i; break; }
            }
        }

        std::string block = content.substr(block_start, block_end - block_start + 1);

        // Skip the root-level braces themselves
        if (key.empty() || block.size() <= 2) {
            pos = block_end + 1;
            continue;
        }

        if (parseShipEntry(key, block)) {
            ++loaded;
        }

        pos = block_end + 1;
    }

    return loaded;
}

const ShipTemplate* ShipDatabase::getShip(const std::string& ship_id) const {
    auto it = ships_.find(ship_id);
    if (it != ships_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> ShipDatabase::getShipIds() const {
    std::vector<std::string> ids;
    ids.reserve(ships_.size());
    for (const auto& kv : ships_) {
        ids.push_back(kv.first);
    }
    return ids;
}

// ---------------------------------------------------------------------------
// Parsing
// ---------------------------------------------------------------------------

bool ShipDatabase::parseShipEntry(const std::string& id, const std::string& json) {
    ShipTemplate ship;
    ship.id = id;
    ship.name        = extractString(json, "name");
    ship.ship_class  = extractString(json, "class");
    ship.race        = extractString(json, "race");
    ship.description = extractString(json, "description");

    if (ship.name.empty()) {
        // Likely not a real ship entry
        return false;
    }

    // Health
    ship.hull_hp   = extractFloat(json, "hull_hp", 100.0f);
    ship.armor_hp  = extractFloat(json, "armor_hp", 100.0f);
    ship.shield_hp = extractFloat(json, "shield_hp", 100.0f);

    // Capacitor
    ship.capacitor              = extractFloat(json, "capacitor", 100.0f);
    ship.capacitor_recharge_time = extractFloat(json, "capacitor_recharge_time", 200.0f);

    // Fitting
    ship.cpu       = extractFloat(json, "cpu", 100.0f);
    ship.powergrid = extractFloat(json, "powergrid", 50.0f);
    ship.high_slots = extractInt(json, "high_slots", 3);
    ship.mid_slots  = extractInt(json, "mid_slots", 3);
    ship.low_slots  = extractInt(json, "low_slots", 3);
    ship.rig_slots  = extractInt(json, "rig_slots", 3);

    // Navigation
    ship.max_velocity     = extractFloat(json, "max_velocity", 300.0f);
    ship.inertia_modifier = extractFloat(json, "inertia_modifier", 3.0f);
    ship.cargo_capacity   = extractFloat(json, "cargo_capacity", 100.0f);

    // Targeting
    ship.signature_radius    = extractFloat(json, "signature_radius", 35.0f);
    ship.scan_resolution     = extractFloat(json, "scan_resolution", 400.0f);
    ship.max_locked_targets  = extractInt(json, "max_locked_targets", 3);
    ship.max_targeting_range = extractFloat(json, "max_targeting_range", 20000.0f);
    ship.shield_recharge_time = extractFloat(json, "shield_recharge_time", 625.0f);

    // Resistances – stored as percentage (0-100) in JSON, convert to 0.0-1.0
    std::string resist_block = extractBlock(json, "resistances");
    if (!resist_block.empty()) {
        std::string shield_block = extractBlock(resist_block, "shield");
        if (!shield_block.empty()) {
            ship.shield_resists.em        = extractFloat(shield_block, "em", 0.0f) / 100.0f;
            ship.shield_resists.thermal   = extractFloat(shield_block, "thermal", 0.0f) / 100.0f;
            ship.shield_resists.kinetic   = extractFloat(shield_block, "kinetic", 0.0f) / 100.0f;
            ship.shield_resists.explosive = extractFloat(shield_block, "explosive", 0.0f) / 100.0f;
        }
        std::string armor_block = extractBlock(resist_block, "armor");
        if (!armor_block.empty()) {
            ship.armor_resists.em        = extractFloat(armor_block, "em", 0.0f) / 100.0f;
            ship.armor_resists.thermal   = extractFloat(armor_block, "thermal", 0.0f) / 100.0f;
            ship.armor_resists.kinetic   = extractFloat(armor_block, "kinetic", 0.0f) / 100.0f;
            ship.armor_resists.explosive = extractFloat(armor_block, "explosive", 0.0f) / 100.0f;
        }
        std::string hull_block = extractBlock(resist_block, "hull");
        if (!hull_block.empty()) {
            ship.hull_resists.em        = extractFloat(hull_block, "em", 0.0f) / 100.0f;
            ship.hull_resists.thermal   = extractFloat(hull_block, "thermal", 0.0f) / 100.0f;
            ship.hull_resists.kinetic   = extractFloat(hull_block, "kinetic", 0.0f) / 100.0f;
            ship.hull_resists.explosive = extractFloat(hull_block, "explosive", 0.0f) / 100.0f;
        }
    }

    ships_[id] = std::move(ship);
    return true;
}

// ---------------------------------------------------------------------------
// Lightweight JSON helpers
// ---------------------------------------------------------------------------

std::string ShipDatabase::extractString(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return "";

    pos = json.find('\"', pos + 1);
    if (pos == std::string::npos) return "";

    size_t end = pos + 1;
    while (end < json.size()) {
        if (json[end] == '\\') { end += 2; continue; }
        if (json[end] == '\"') break;
        ++end;
    }
    if (end >= json.size()) return "";

    return json.substr(pos + 1, end - pos - 1);
}

float ShipDatabase::extractFloat(const std::string& json, const std::string& key, float fallback) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return fallback;

    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return fallback;
    ++pos;

    // Skip whitespace
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r'))
        ++pos;

    try {
        size_t end = pos;
        while (end < json.size() &&
               (json[end] == '-' || json[end] == '.' ||
                (json[end] >= '0' && json[end] <= '9') ||
                json[end] == 'e' || json[end] == 'E' || json[end] == '+')) {
            ++end;
        }
        return std::stof(json.substr(pos, end - pos));
    } catch (...) {
        return fallback;
    }
}

int ShipDatabase::extractInt(const std::string& json, const std::string& key, int fallback) {
    return static_cast<int>(extractFloat(json, key, static_cast<float>(fallback)));
}

std::string ShipDatabase::extractBlock(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos = json.find('{', pos + search.size());
    if (pos == std::string::npos) return "";

    int depth = 0;
    size_t end = pos;
    bool in_str = false;
    for (size_t i = pos; i < json.size(); ++i) {
        char c = json[i];
        if (c == '\\' && in_str) { ++i; continue; }
        if (c == '\"') { in_str = !in_str; continue; }
        if (in_str) continue;
        if (c == '{') ++depth;
        if (c == '}') {
            --depth;
            if (depth == 0) { end = i; break; }
        }
    }

    return json.substr(pos, end - pos + 1);
}

} // namespace data
} // namespace eve
