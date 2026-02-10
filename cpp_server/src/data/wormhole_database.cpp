#include "data/wormhole_database.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

namespace eve {
namespace data {

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

int WormholeDatabase::loadFromDirectory(const std::string& data_dir) {
    int total = 0;
    total += loadClasses(data_dir + "/wormholes/wormhole_classes.json");
    total += loadEffects(data_dir + "/wormholes/wormhole_effects.json");
    std::cout << "[WormholeDatabase] Loaded " << classes_.size()
              << " wormhole classes and " << effects_.size()
              << " effects from " << data_dir << std::endl;
    return total;
}

const WormholeClassTemplate* WormholeDatabase::getWormholeClass(const std::string& class_id) const {
    auto it = classes_.find(class_id);
    return (it != classes_.end()) ? &it->second : nullptr;
}

const WormholeEffect* WormholeDatabase::getEffect(const std::string& effect_id) const {
    auto it = effects_.find(effect_id);
    return (it != effects_.end()) ? &it->second : nullptr;
}

std::vector<std::string> WormholeDatabase::getClassIds() const {
    std::vector<std::string> ids;
    ids.reserve(classes_.size());
    for (const auto& kv : classes_) ids.push_back(kv.first);
    return ids;
}

std::vector<std::string> WormholeDatabase::getEffectIds() const {
    std::vector<std::string> ids;
    ids.reserve(effects_.size());
    for (const auto& kv : effects_) ids.push_back(kv.first);
    return ids;
}

// ---------------------------------------------------------------------------
// Loaders
// ---------------------------------------------------------------------------

int WormholeDatabase::loadClasses(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) return 0;

    std::stringstream buf;
    buf << ifs.rdbuf();
    std::string content = buf.str();

    int loaded = 0;
    size_t pos = 0;

    while (pos < content.size()) {
        size_t key_start = content.find('\"', pos);
        if (key_start == std::string::npos) break;
        size_t key_end = content.find('\"', key_start + 1);
        if (key_end == std::string::npos) break;

        std::string key = content.substr(key_start + 1, key_end - key_start - 1);

        size_t block_start = content.find('{', key_end);
        if (block_start == std::string::npos) break;

        int depth = 0;
        size_t block_end = block_start;
        bool in_string = false;
        for (size_t i = block_start; i < content.size(); ++i) {
            char c = content[i];
            if (c == '\\' && in_string) { ++i; continue; }
            if (c == '\"') { in_string = !in_string; continue; }
            if (in_string) continue;
            if (c == '{') ++depth;
            if (c == '}') { --depth; if (depth == 0) { block_end = i; break; } }
        }

        std::string block = content.substr(block_start, block_end - block_start + 1);
        if (key.empty() || block.size() <= 2) { pos = block_end + 1; continue; }

        WormholeClassTemplate tmpl;
        tmpl.id   = key;
        tmpl.name = extractString(block, "name");
        if (tmpl.name.empty()) { pos = block_end + 1; continue; }

        tmpl.wormhole_class          = extractInt(block, "class", 1);
        tmpl.difficulty              = extractString(block, "difficulty");
        tmpl.description             = extractString(block, "description");
        tmpl.max_ship_class          = extractString(block, "max_ship_class");
        tmpl.max_ship_mass           = static_cast<double>(extractFloat(block, "max_ship_mass", 20000000.0f));
        tmpl.max_wormhole_stability  = static_cast<double>(extractFloat(block, "max_wormhole_stability", 500000000.0f));
        tmpl.max_wormhole_lifetime_hours = extractFloat(block, "max_wormhole_lifetime_hours", 24.0f);
        tmpl.salvage_value_multiplier = extractFloat(block, "salvage_value_multiplier", 1.0f);
        tmpl.blue_loot_isk           = static_cast<double>(extractFloat(block, "blue_loot_isk", 150000.0f));

        // Static connections
        std::string sc_arr = extractArray(block, "static_connections");
        if (!sc_arr.empty()) tmpl.static_connections = parseStringArray(sc_arr);

        // Dormant NPC spawns
        std::string spawns_arr = extractArray(block, "dormant_spawns");
        if (!spawns_arr.empty()) {
            // Each element is { ... }
            size_t sp = 0;
            while (sp < spawns_arr.size()) {
                size_t obj_start = spawns_arr.find('{', sp);
                if (obj_start == std::string::npos) break;
                int d = 0; size_t obj_end = obj_start;
                for (size_t i = obj_start; i < spawns_arr.size(); ++i) {
                    if (spawns_arr[i] == '{') ++d;
                    if (spawns_arr[i] == '}') { --d; if (d == 0) { obj_end = i; break; } }
                }
                std::string obj = spawns_arr.substr(obj_start, obj_end - obj_start + 1);

                DormantSpawn spawn;
                spawn.id        = extractString(obj, "id");
                spawn.name      = extractString(obj, "name");
                spawn.type      = extractString(obj, "type");
                spawn.count_min = extractInt(obj, "count_min", 1);
                spawn.count_max = extractInt(obj, "count_max", 2);
                if (!spawn.id.empty()) tmpl.dormant_spawns.push_back(std::move(spawn));

                sp = obj_end + 1;
            }
        }

        classes_[key] = std::move(tmpl);
        ++loaded;
        pos = block_end + 1;
    }
    return loaded;
}

int WormholeDatabase::loadEffects(const std::string& filepath) {
    std::ifstream ifs(filepath);
    if (!ifs.is_open()) return 0;

    std::stringstream buf;
    buf << ifs.rdbuf();
    std::string content = buf.str();

    int loaded = 0;
    size_t pos = 0;

    while (pos < content.size()) {
        size_t key_start = content.find('\"', pos);
        if (key_start == std::string::npos) break;
        size_t key_end = content.find('\"', key_start + 1);
        if (key_end == std::string::npos) break;

        std::string key = content.substr(key_start + 1, key_end - key_start - 1);

        size_t block_start = content.find('{', key_end);
        if (block_start == std::string::npos) break;

        int depth = 0; size_t block_end = block_start; bool in_str = false;
        for (size_t i = block_start; i < content.size(); ++i) {
            char c = content[i];
            if (c == '\\' && in_str) { ++i; continue; }
            if (c == '\"') { in_str = !in_str; continue; }
            if (in_str) continue;
            if (c == '{') ++depth;
            if (c == '}') { --depth; if (depth == 0) { block_end = i; break; } }
        }

        std::string block = content.substr(block_start, block_end - block_start + 1);
        if (key.empty() || block.size() <= 2) { pos = block_end + 1; continue; }

        WormholeEffect eff;
        eff.id          = key;
        eff.name        = extractString(block, "name");
        eff.description = extractString(block, "description");
        if (eff.name.empty()) { pos = block_end + 1; continue; }

        // Parse modifiers block
        std::string mod_block = extractBlock(block, "modifiers");
        if (!mod_block.empty()) {
            // Walk through key-value pairs inside the modifiers object
            size_t mp = 1; // skip opening {
            while (mp < mod_block.size()) {
                size_t mk_start = mod_block.find('\"', mp);
                if (mk_start == std::string::npos) break;
                size_t mk_end = mod_block.find('\"', mk_start + 1);
                if (mk_end == std::string::npos) break;
                std::string mk = mod_block.substr(mk_start + 1, mk_end - mk_start - 1);

                size_t colon = mod_block.find(':', mk_end);
                if (colon == std::string::npos) break;

                float val = 1.0f;
                size_t vp = colon + 1;
                while (vp < mod_block.size() && (mod_block[vp] == ' ' || mod_block[vp] == '\t'))
                    ++vp;
                size_t ve = vp;
                while (ve < mod_block.size() && (mod_block[ve] == '-' || mod_block[ve] == '.' ||
                       (mod_block[ve] >= '0' && mod_block[ve] <= '9')))
                    ++ve;
                if (ve > vp) {
                    try { val = std::stof(mod_block.substr(vp, ve - vp)); } catch (...) {}
                }

                if (!mk.empty()) eff.modifiers[mk] = val;
                mp = ve;
            }
        }

        effects_[key] = std::move(eff);
        ++loaded;
        pos = block_end + 1;
    }
    return loaded;
}

// ---------------------------------------------------------------------------
// JSON helpers (same pattern as ShipDatabase)
// ---------------------------------------------------------------------------

std::string WormholeDatabase::extractString(const std::string& json, const std::string& key) {
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

float WormholeDatabase::extractFloat(const std::string& json, const std::string& key, float fallback) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return fallback;
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return fallback;
    ++pos;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\n' || json[pos] == '\r'))
        ++pos;
    try {
        size_t end = pos;
        while (end < json.size() &&
               (json[end] == '-' || json[end] == '.' ||
                (json[end] >= '0' && json[end] <= '9') ||
                json[end] == 'e' || json[end] == 'E' || json[end] == '+'))
            ++end;
        return std::stof(json.substr(pos, end - pos));
    } catch (...) { return fallback; }
}

int WormholeDatabase::extractInt(const std::string& json, const std::string& key, int fallback) {
    return static_cast<int>(extractFloat(json, key, static_cast<float>(fallback)));
}

std::string WormholeDatabase::extractBlock(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos = json.find('{', pos + search.size());
    if (pos == std::string::npos) return "";
    int depth = 0; size_t end = pos; bool in_str = false;
    for (size_t i = pos; i < json.size(); ++i) {
        char c = json[i];
        if (c == '\\' && in_str) { ++i; continue; }
        if (c == '\"') { in_str = !in_str; continue; }
        if (in_str) continue;
        if (c == '{') ++depth;
        if (c == '}') { --depth; if (depth == 0) { end = i; break; } }
    }
    return json.substr(pos, end - pos + 1);
}

std::string WormholeDatabase::extractArray(const std::string& json, const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";
    pos = json.find('[', pos + search.size());
    if (pos == std::string::npos) return "";
    int depth = 0; size_t end = pos;
    bool in_str = false;
    for (size_t i = pos; i < json.size(); ++i) {
        char c = json[i];
        if (c == '\\' && in_str) { ++i; continue; }
        if (c == '\"') { in_str = !in_str; continue; }
        if (in_str) continue;
        if (c == '[') ++depth;
        if (c == ']') { --depth; if (depth == 0) { end = i; break; } }
    }
    return json.substr(pos, end - pos + 1);
}

std::vector<std::string> WormholeDatabase::parseStringArray(const std::string& arr) {
    std::vector<std::string> result;
    size_t pos = 0;
    while (pos < arr.size()) {
        size_t qs = arr.find('\"', pos);
        if (qs == std::string::npos) break;
        size_t qe = arr.find('\"', qs + 1);
        if (qe == std::string::npos) break;
        result.push_back(arr.substr(qs + 1, qe - qs - 1));
        pos = qe + 1;
    }
    return result;
}

} // namespace data
} // namespace eve
