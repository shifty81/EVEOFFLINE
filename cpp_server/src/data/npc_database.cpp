#include "data/npc_database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>

namespace atlas {
namespace data {

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

int NpcDatabase::loadFromDirectory(const std::string& data_dir) {
    // NPC JSON files located under data_dir/npcs/
    const std::string npc_dir = data_dir + "/npcs/";
    const char* files[] = {
        "pirates.json"
    };

    int total = 0;
    for (const char* file : files) {
        int count = loadFromFile(npc_dir + file);
        if (count > 0) {
            total += count;
        }
    }

    std::cout << "[NpcDatabase] Loaded " << total
              << " NPC templates from " << npc_dir << std::endl;
    return total;
}

int NpcDatabase::loadFromFile(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) {
        // Silently skip missing files
        return 0;
    }

    std::stringstream buf;
    buf << ifs.rdbuf();
    std::string content = buf.str();

    // The top-level JSON is an object where each key is an NPC id.
    int loaded = 0;
    size_t pos = 0;

    while (pos < content.size()) {
        // Find next quoted key at top level
        size_t key_start = content.find('\"', pos);
        if (key_start == std::string::npos) break;

        size_t key_end = content.find('\"', key_start + 1);
        if (key_end == std::string::npos) break;

        std::string key = content.substr(key_start + 1, key_end - key_start - 1);

        // Find the opening '{' of this NPC's block
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

        if (parseNpcEntry(key, block)) {
            ++loaded;
        }

        pos = block_end + 1;
    }

    return loaded;
}

const NpcTemplate* NpcDatabase::getNpc(const std::string& npc_id) const {
    auto it = npcs_.find(npc_id);
    if (it != npcs_.end()) {
        return &it->second;
    }
    return nullptr;
}

std::vector<std::string> NpcDatabase::getNpcIds() const {
    std::vector<std::string> ids;
    ids.reserve(npcs_.size());
    for (const auto& kv : npcs_) {
        ids.push_back(kv.first);
    }
    return ids;
}

// ---------------------------------------------------------------------------
// Parsing
// ---------------------------------------------------------------------------

bool NpcDatabase::parseNpcEntry(const std::string& id, const std::string& json) {
    NpcTemplate npc;
    npc.id   = id;
    npc.name = extractString(json, "name");
    npc.type = extractString(json, "type");
    npc.faction = extractString(json, "faction");

    if (npc.name.empty()) {
        return false;
    }

    // Health
    npc.hull_hp   = extractFloat(json, "hull_hp", 100.0f);
    npc.armor_hp  = extractFloat(json, "armor_hp", 100.0f);
    npc.shield_hp = extractFloat(json, "shield_hp", 100.0f);

    // Navigation / AI
    npc.max_velocity     = extractFloat(json, "max_velocity", 300.0f);
    npc.orbit_distance   = extractFloat(json, "orbit_distance", 1000.0f);
    npc.signature_radius = extractFloat(json, "signature_radius", 35.0f);

    // Bounty (read as float, store as double)
    npc.bounty = static_cast<double>(extractFloat(json, "bounty", 0.0f));

    // Behavior
    npc.behavior        = extractString(json, "behavior");
    npc.awareness_range = extractFloat(json, "awareness_range", 50000.0f);

    // Weapons – array of objects inside "weapons": [...]
    {
        std::string search = "\"weapons\"";
        size_t wpos = json.find(search);
        if (wpos != std::string::npos) {
            // Find the opening '['
            size_t arr_start = json.find('[', wpos);
            if (arr_start != std::string::npos) {
                // Find matching ']'
                int depth = 0;
                size_t arr_end = arr_start;
                for (size_t i = arr_start; i < json.size(); ++i) {
                    if (json[i] == '[') ++depth;
                    if (json[i] == ']') {
                        --depth;
                        if (depth == 0) { arr_end = i; break; }
                    }
                }

                // Parse individual weapon objects within the array
                std::string arr_content = json.substr(arr_start + 1, arr_end - arr_start - 1);
                size_t obj_pos = 0;
                while (obj_pos < arr_content.size()) {
                    size_t obj_start = arr_content.find('{', obj_pos);
                    if (obj_start == std::string::npos) break;

                    int d = 0;
                    size_t obj_end = obj_start;
                    bool in_str = false;
                    for (size_t i = obj_start; i < arr_content.size(); ++i) {
                        char c = arr_content[i];
                        if (c == '\\' && in_str) { ++i; continue; }
                        if (c == '\"') { in_str = !in_str; continue; }
                        if (in_str) continue;
                        if (c == '{') ++d;
                        if (c == '}') {
                            --d;
                            if (d == 0) { obj_end = i; break; }
                        }
                    }

                    std::string wblock = arr_content.substr(obj_start, obj_end - obj_start + 1);

                    NpcTemplate::WeaponData wd;
                    wd.type          = extractString(wblock, "type");
                    wd.damage        = extractFloat(wblock, "damage", 0.0f);
                    wd.damage_type   = extractString(wblock, "damage_type");
                    wd.optimal_range = extractFloat(wblock, "optimal_range", 0.0f);
                    wd.falloff_range = extractFloat(wblock, "falloff_range", 0.0f);
                    wd.rate_of_fire  = extractFloat(wblock, "rate_of_fire", 0.0f);
                    npc.weapons.push_back(wd);

                    obj_pos = obj_end + 1;
                }
            }
        }
    }

    // Resistances – stored as percentage (0-100) in JSON, convert to 0.0-1.0
    std::string resist_block = extractBlock(json, "resistances");
    if (!resist_block.empty()) {
        std::string shield_block = extractBlock(resist_block, "shield");
        if (!shield_block.empty()) {
            npc.shield_resists.em        = extractFloat(shield_block, "em", 0.0f) / 100.0f;
            npc.shield_resists.thermal   = extractFloat(shield_block, "thermal", 0.0f) / 100.0f;
            npc.shield_resists.kinetic   = extractFloat(shield_block, "kinetic", 0.0f) / 100.0f;
            npc.shield_resists.explosive = extractFloat(shield_block, "explosive", 0.0f) / 100.0f;
        }
        std::string armor_block = extractBlock(resist_block, "armor");
        if (!armor_block.empty()) {
            npc.armor_resists.em        = extractFloat(armor_block, "em", 0.0f) / 100.0f;
            npc.armor_resists.thermal   = extractFloat(armor_block, "thermal", 0.0f) / 100.0f;
            npc.armor_resists.kinetic   = extractFloat(armor_block, "kinetic", 0.0f) / 100.0f;
            npc.armor_resists.explosive = extractFloat(armor_block, "explosive", 0.0f) / 100.0f;
        }
        std::string hull_block = extractBlock(resist_block, "hull");
        if (!hull_block.empty()) {
            npc.hull_resists.em        = extractFloat(hull_block, "em", 0.0f) / 100.0f;
            npc.hull_resists.thermal   = extractFloat(hull_block, "thermal", 0.0f) / 100.0f;
            npc.hull_resists.kinetic   = extractFloat(hull_block, "kinetic", 0.0f) / 100.0f;
            npc.hull_resists.explosive = extractFloat(hull_block, "explosive", 0.0f) / 100.0f;
        }
    }

    // Loot table – array of strings
    {
        std::string search = "\"loot_table\"";
        size_t lpos = json.find(search);
        if (lpos != std::string::npos) {
            size_t arr_start = json.find('[', lpos);
            if (arr_start != std::string::npos) {
                size_t arr_end = json.find(']', arr_start);
                if (arr_end != std::string::npos) {
                    std::string arr_content = json.substr(arr_start + 1, arr_end - arr_start - 1);
                    size_t spos = 0;
                    while (spos < arr_content.size()) {
                        size_t qs = arr_content.find('\"', spos);
                        if (qs == std::string::npos) break;
                        size_t qe = arr_content.find('\"', qs + 1);
                        if (qe == std::string::npos) break;
                        npc.loot_table.push_back(arr_content.substr(qs + 1, qe - qs - 1));
                        spos = qe + 1;
                    }
                }
            }
        }
    }

    npcs_[id] = std::move(npc);
    return true;
}

// ---------------------------------------------------------------------------
// Lightweight JSON helpers
// ---------------------------------------------------------------------------

std::string NpcDatabase::extractString(const std::string& json, const std::string& key) {
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

float NpcDatabase::extractFloat(const std::string& json, const std::string& key, float fallback) {
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

int NpcDatabase::extractInt(const std::string& json, const std::string& key, int fallback) {
    return static_cast<int>(extractFloat(json, key, static_cast<float>(fallback)));
}

std::string NpcDatabase::extractBlock(const std::string& json, const std::string& key) {
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
} // namespace atlas
