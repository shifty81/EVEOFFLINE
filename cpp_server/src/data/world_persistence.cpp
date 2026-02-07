#include "data/world_persistence.h"
#include "components/game_components.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>

namespace eve {
namespace data {

// ---------------------------------------------------------------------------
// File I/O
// ---------------------------------------------------------------------------

bool WorldPersistence::saveWorld(const ecs::World* world,
                                 const std::string& filepath) {
    std::string json = serializeWorld(world);

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[WorldPersistence] Cannot open file for writing: "
                  << filepath << std::endl;
        return false;
    }

    file << json;
    file.close();

    std::cout << "[WorldPersistence] World saved to " << filepath << std::endl;
    return true;
}

bool WorldPersistence::loadWorld(ecs::World* world,
                                 const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "[WorldPersistence] Cannot open file for reading: "
                  << filepath << std::endl;
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    file.close();

    return deserializeWorld(world, ss.str());
}

// ---------------------------------------------------------------------------
// Serialization
// ---------------------------------------------------------------------------

std::string WorldPersistence::serializeWorld(const ecs::World* world) const {
    // We need a non-const World* to call getAllEntities (existing API limitation)
    auto* mutable_world = const_cast<ecs::World*>(world);
    auto entities = mutable_world->getAllEntities();

    std::ostringstream json;
    json << "{\"entities\":[";

    bool first = true;
    for (const auto* entity : entities) {
        if (!first) json << ",";
        first = false;
        json << serializeEntity(entity);
    }

    json << "]}";
    return json.str();
}

bool WorldPersistence::deserializeWorld(ecs::World* world,
                                        const std::string& json) const {
    // Find the entities array
    size_t arr_start = json.find("[");
    size_t arr_end   = json.rfind("]");
    if (arr_start == std::string::npos || arr_end == std::string::npos ||
        arr_end <= arr_start) {
        std::cerr << "[WorldPersistence] Invalid JSON structure" << std::endl;
        return false;
    }

    std::string array_content = json.substr(arr_start + 1,
                                            arr_end - arr_start - 1);

    // Parse individual entity objects by matching braces
    int depth = 0;
    size_t obj_start = std::string::npos;
    int entity_count = 0;

    for (size_t i = 0; i < array_content.size(); ++i) {
        char c = array_content[i];
        if (c == '{') {
            if (depth == 0) obj_start = i;
            ++depth;
        } else if (c == '}') {
            --depth;
            if (depth == 0 && obj_start != std::string::npos) {
                std::string entity_json =
                    array_content.substr(obj_start, i - obj_start + 1);
                if (deserializeEntity(world, entity_json)) {
                    ++entity_count;
                }
                obj_start = std::string::npos;
            }
        }
    }

    std::cout << "[WorldPersistence] Loaded " << entity_count
              << " entities" << std::endl;
    return true;
}

// ---------------------------------------------------------------------------
// Single-entity serialization
// ---------------------------------------------------------------------------

std::string WorldPersistence::serializeEntity(
        const ecs::Entity* entity) const {
    std::ostringstream json;
    json << "{\"id\":\"" << entity->getId() << "\"";

    // Position
    auto* pos = entity->getComponent<components::Position>();
    if (pos) {
        json << ",\"position\":{\"x\":" << pos->x
             << ",\"y\":" << pos->y
             << ",\"z\":" << pos->z
             << ",\"rotation\":" << pos->rotation << "}";
    }

    // Velocity
    auto* vel = entity->getComponent<components::Velocity>();
    if (vel) {
        json << ",\"velocity\":{\"vx\":" << vel->vx
             << ",\"vy\":" << vel->vy
             << ",\"vz\":" << vel->vz
             << ",\"angular_velocity\":" << vel->angular_velocity
             << ",\"max_speed\":" << vel->max_speed << "}";
    }

    // Health
    auto* hp = entity->getComponent<components::Health>();
    if (hp) {
        json << ",\"health\":{"
             << "\"hull_hp\":" << hp->hull_hp
             << ",\"hull_max\":" << hp->hull_max
             << ",\"armor_hp\":" << hp->armor_hp
             << ",\"armor_max\":" << hp->armor_max
             << ",\"shield_hp\":" << hp->shield_hp
             << ",\"shield_max\":" << hp->shield_max
             << ",\"shield_recharge_rate\":" << hp->shield_recharge_rate
             << ",\"hull_em_resist\":" << hp->hull_em_resist
             << ",\"hull_thermal_resist\":" << hp->hull_thermal_resist
             << ",\"hull_kinetic_resist\":" << hp->hull_kinetic_resist
             << ",\"hull_explosive_resist\":" << hp->hull_explosive_resist
             << ",\"armor_em_resist\":" << hp->armor_em_resist
             << ",\"armor_thermal_resist\":" << hp->armor_thermal_resist
             << ",\"armor_kinetic_resist\":" << hp->armor_kinetic_resist
             << ",\"armor_explosive_resist\":" << hp->armor_explosive_resist
             << ",\"shield_em_resist\":" << hp->shield_em_resist
             << ",\"shield_thermal_resist\":" << hp->shield_thermal_resist
             << ",\"shield_kinetic_resist\":" << hp->shield_kinetic_resist
             << ",\"shield_explosive_resist\":" << hp->shield_explosive_resist
             << "}";
    }

    // Capacitor
    auto* cap = entity->getComponent<components::Capacitor>();
    if (cap) {
        json << ",\"capacitor\":{"
             << "\"capacitor\":" << cap->capacitor
             << ",\"capacitor_max\":" << cap->capacitor_max
             << ",\"recharge_rate\":" << cap->recharge_rate << "}";
    }

    // Ship
    auto* ship = entity->getComponent<components::Ship>();
    if (ship) {
        json << ",\"ship\":{"
             << "\"ship_type\":\"" << ship->ship_type << "\""
             << ",\"ship_class\":\"" << ship->ship_class << "\""
             << ",\"ship_name\":\"" << ship->ship_name << "\""
             << ",\"race\":\"" << ship->race << "\""
             << ",\"cpu\":" << ship->cpu
             << ",\"cpu_max\":" << ship->cpu_max
             << ",\"powergrid\":" << ship->powergrid
             << ",\"powergrid_max\":" << ship->powergrid_max
             << ",\"signature_radius\":" << ship->signature_radius
             << ",\"scan_resolution\":" << ship->scan_resolution
             << ",\"max_locked_targets\":" << ship->max_locked_targets
             << ",\"max_targeting_range\":" << ship->max_targeting_range
             << "}";
    }

    // Faction
    auto* fac = entity->getComponent<components::Faction>();
    if (fac) {
        json << ",\"faction\":{"
             << "\"faction_name\":\"" << fac->faction_name << "\""
             << "}";
    }

    // AI
    auto* ai = entity->getComponent<components::AI>();
    if (ai) {
        json << ",\"ai\":{"
             << "\"behavior\":" << static_cast<int>(ai->behavior)
             << ",\"state\":" << static_cast<int>(ai->state)
             << ",\"target_entity_id\":\"" << ai->target_entity_id << "\""
             << ",\"orbit_distance\":" << ai->orbit_distance
             << ",\"awareness_range\":" << ai->awareness_range
             << "}";
    }

    // Weapon
    auto* weapon = entity->getComponent<components::Weapon>();
    if (weapon) {
        json << ",\"weapon\":{"
             << "\"weapon_type\":\"" << weapon->weapon_type << "\""
             << ",\"damage_type\":\"" << weapon->damage_type << "\""
             << ",\"damage\":" << weapon->damage
             << ",\"optimal_range\":" << weapon->optimal_range
             << ",\"falloff_range\":" << weapon->falloff_range
             << ",\"tracking_speed\":" << weapon->tracking_speed
             << ",\"rate_of_fire\":" << weapon->rate_of_fire
             << ",\"capacitor_cost\":" << weapon->capacitor_cost
             << ",\"ammo_type\":\"" << weapon->ammo_type << "\""
             << ",\"ammo_count\":" << weapon->ammo_count
             << "}";
    }

    // Player
    auto* player = entity->getComponent<components::Player>();
    if (player) {
        json << ",\"player\":{"
             << "\"player_id\":\"" << player->player_id << "\""
             << ",\"character_name\":\"" << player->character_name << "\""
             << ",\"isk\":" << player->isk
             << ",\"corporation\":\"" << player->corporation << "\""
             << "}";
    }

    // WormholeConnection
    auto* wh = entity->getComponent<components::WormholeConnection>();
    if (wh) {
        json << ",\"wormhole_connection\":{"
             << "\"wormhole_id\":\"" << wh->wormhole_id << "\""
             << ",\"source_system\":\"" << wh->source_system << "\""
             << ",\"destination_system\":\"" << wh->destination_system << "\""
             << ",\"max_mass\":" << wh->max_mass
             << ",\"remaining_mass\":" << wh->remaining_mass
             << ",\"max_jump_mass\":" << wh->max_jump_mass
             << ",\"max_lifetime_hours\":" << wh->max_lifetime_hours
             << ",\"elapsed_hours\":" << wh->elapsed_hours
             << ",\"collapsed\":" << (wh->collapsed ? "true" : "false")
             << "}";
    }

    // SolarSystem
    auto* ss = entity->getComponent<components::SolarSystem>();
    if (ss) {
        json << ",\"solar_system\":{"
             << "\"system_id\":\"" << ss->system_id << "\""
             << ",\"system_name\":\"" << ss->system_name << "\""
             << ",\"wormhole_class\":" << ss->wormhole_class
             << ",\"effect_name\":\"" << ss->effect_name << "\""
             << ",\"sleepers_spawned\":" << (ss->sleepers_spawned ? "true" : "false")
             << "}";
    }

    // FleetMembership
    auto* fm = entity->getComponent<components::FleetMembership>();
    if (fm) {
        json << ",\"fleet_membership\":{"
             << "\"fleet_id\":\"" << fm->fleet_id << "\""
             << ",\"role\":\"" << fm->role << "\""
             << ",\"squad_id\":\"" << fm->squad_id << "\""
             << ",\"wing_id\":\"" << fm->wing_id << "\""
             << "}";
    }

    json << "}";
    return json.str();
}

bool WorldPersistence::deserializeEntity(ecs::World* world,
                                          const std::string& json) const {
    std::string id = extractString(json, "id");
    if (id.empty()) return false;

    auto* entity = world->createEntity(id);
    if (!entity) return false;

    // Position
    std::string pos_json = extractObject(json, "position");
    if (!pos_json.empty()) {
        auto pos = std::make_unique<components::Position>();
        pos->x = extractFloat(pos_json, "\"x\":");
        pos->y = extractFloat(pos_json, "\"y\":");
        pos->z = extractFloat(pos_json, "\"z\":");
        pos->rotation = extractFloat(pos_json, "\"rotation\":");
        entity->addComponent(std::move(pos));
    }

    // Velocity
    std::string vel_json = extractObject(json, "velocity");
    if (!vel_json.empty()) {
        auto vel = std::make_unique<components::Velocity>();
        vel->vx = extractFloat(vel_json, "\"vx\":");
        vel->vy = extractFloat(vel_json, "\"vy\":");
        vel->vz = extractFloat(vel_json, "\"vz\":");
        vel->angular_velocity = extractFloat(vel_json, "\"angular_velocity\":");
        vel->max_speed = extractFloat(vel_json, "\"max_speed\":", 100.0f);
        entity->addComponent(std::move(vel));
    }

    // Health
    std::string hp_json = extractObject(json, "health");
    if (!hp_json.empty()) {
        auto hp = std::make_unique<components::Health>();
        hp->hull_hp   = extractFloat(hp_json, "\"hull_hp\":", 100.0f);
        hp->hull_max  = extractFloat(hp_json, "\"hull_max\":", 100.0f);
        hp->armor_hp  = extractFloat(hp_json, "\"armor_hp\":", 100.0f);
        hp->armor_max = extractFloat(hp_json, "\"armor_max\":", 100.0f);
        hp->shield_hp  = extractFloat(hp_json, "\"shield_hp\":", 100.0f);
        hp->shield_max = extractFloat(hp_json, "\"shield_max\":", 100.0f);
        hp->shield_recharge_rate = extractFloat(hp_json, "\"shield_recharge_rate\":", 1.0f);
        hp->hull_em_resist        = extractFloat(hp_json, "\"hull_em_resist\":");
        hp->hull_thermal_resist   = extractFloat(hp_json, "\"hull_thermal_resist\":");
        hp->hull_kinetic_resist   = extractFloat(hp_json, "\"hull_kinetic_resist\":");
        hp->hull_explosive_resist = extractFloat(hp_json, "\"hull_explosive_resist\":");
        hp->armor_em_resist        = extractFloat(hp_json, "\"armor_em_resist\":");
        hp->armor_thermal_resist   = extractFloat(hp_json, "\"armor_thermal_resist\":");
        hp->armor_kinetic_resist   = extractFloat(hp_json, "\"armor_kinetic_resist\":");
        hp->armor_explosive_resist = extractFloat(hp_json, "\"armor_explosive_resist\":");
        hp->shield_em_resist        = extractFloat(hp_json, "\"shield_em_resist\":");
        hp->shield_thermal_resist   = extractFloat(hp_json, "\"shield_thermal_resist\":");
        hp->shield_kinetic_resist   = extractFloat(hp_json, "\"shield_kinetic_resist\":");
        hp->shield_explosive_resist = extractFloat(hp_json, "\"shield_explosive_resist\":");
        entity->addComponent(std::move(hp));
    }

    // Capacitor
    std::string cap_json = extractObject(json, "capacitor");
    if (!cap_json.empty()) {
        auto cap = std::make_unique<components::Capacitor>();
        cap->capacitor     = extractFloat(cap_json, "\"capacitor\":", 100.0f);
        cap->capacitor_max = extractFloat(cap_json, "\"capacitor_max\":", 100.0f);
        cap->recharge_rate = extractFloat(cap_json, "\"recharge_rate\":", 2.0f);
        entity->addComponent(std::move(cap));
    }

    // Ship
    std::string ship_json = extractObject(json, "ship");
    if (!ship_json.empty()) {
        auto ship = std::make_unique<components::Ship>();
        ship->ship_type  = extractString(ship_json, "ship_type");
        ship->ship_class = extractString(ship_json, "ship_class");
        ship->ship_name  = extractString(ship_json, "ship_name");
        ship->race       = extractString(ship_json, "race");
        ship->cpu        = extractFloat(ship_json, "\"cpu\":");
        ship->cpu_max    = extractFloat(ship_json, "\"cpu_max\":", 100.0f);
        ship->powergrid     = extractFloat(ship_json, "\"powergrid\":");
        ship->powergrid_max = extractFloat(ship_json, "\"powergrid_max\":", 50.0f);
        ship->signature_radius    = extractFloat(ship_json, "\"signature_radius\":", 35.0f);
        ship->scan_resolution     = extractFloat(ship_json, "\"scan_resolution\":", 400.0f);
        ship->max_locked_targets  = extractInt(ship_json, "\"max_locked_targets\":", 3);
        ship->max_targeting_range = extractFloat(ship_json, "\"max_targeting_range\":", 20000.0f);
        entity->addComponent(std::move(ship));
    }

    // Faction
    std::string fac_json = extractObject(json, "faction");
    if (!fac_json.empty()) {
        auto fac = std::make_unique<components::Faction>();
        fac->faction_name = extractString(fac_json, "faction_name");
        entity->addComponent(std::move(fac));
    }

    // AI
    std::string ai_json = extractObject(json, "ai");
    if (!ai_json.empty()) {
        auto ai = std::make_unique<components::AI>();
        ai->behavior = static_cast<components::AI::Behavior>(
            extractInt(ai_json, "\"behavior\":"));
        ai->state = static_cast<components::AI::State>(
            extractInt(ai_json, "\"state\":"));
        ai->target_entity_id = extractString(ai_json, "target_entity_id");
        ai->orbit_distance   = extractFloat(ai_json, "\"orbit_distance\":", 1000.0f);
        ai->awareness_range  = extractFloat(ai_json, "\"awareness_range\":", 50000.0f);
        entity->addComponent(std::move(ai));
    }

    // Weapon
    std::string wep_json = extractObject(json, "weapon");
    if (!wep_json.empty()) {
        auto weapon = std::make_unique<components::Weapon>();
        weapon->weapon_type    = extractString(wep_json, "weapon_type");
        weapon->damage_type    = extractString(wep_json, "damage_type");
        weapon->damage         = extractFloat(wep_json, "\"damage\":", 10.0f);
        weapon->optimal_range  = extractFloat(wep_json, "\"optimal_range\":", 5000.0f);
        weapon->falloff_range  = extractFloat(wep_json, "\"falloff_range\":", 2500.0f);
        weapon->tracking_speed = extractFloat(wep_json, "\"tracking_speed\":", 0.5f);
        weapon->rate_of_fire   = extractFloat(wep_json, "\"rate_of_fire\":", 3.0f);
        weapon->capacitor_cost = extractFloat(wep_json, "\"capacitor_cost\":", 5.0f);
        weapon->ammo_type      = extractString(wep_json, "ammo_type");
        weapon->ammo_count     = extractInt(wep_json, "\"ammo_count\":", 100);
        entity->addComponent(std::move(weapon));
    }

    // Player
    std::string player_json = extractObject(json, "player");
    if (!player_json.empty()) {
        auto player = std::make_unique<components::Player>();
        player->player_id      = extractString(player_json, "player_id");
        player->character_name = extractString(player_json, "character_name");
        player->isk            = extractDouble(player_json, "\"isk\":", 1000000.0);
        player->corporation    = extractString(player_json, "corporation");
        entity->addComponent(std::move(player));
    }

    // WormholeConnection
    std::string wh_json = extractObject(json, "wormhole_connection");
    if (!wh_json.empty()) {
        auto wh = std::make_unique<components::WormholeConnection>();
        wh->wormhole_id         = extractString(wh_json, "wormhole_id");
        wh->source_system       = extractString(wh_json, "source_system");
        wh->destination_system  = extractString(wh_json, "destination_system");
        wh->max_mass            = extractDouble(wh_json, "\"max_mass\":", 500000000.0);
        wh->remaining_mass      = extractDouble(wh_json, "\"remaining_mass\":", 500000000.0);
        wh->max_jump_mass       = extractDouble(wh_json, "\"max_jump_mass\":", 20000000.0);
        wh->max_lifetime_hours  = extractFloat(wh_json, "\"max_lifetime_hours\":", 24.0f);
        wh->elapsed_hours       = extractFloat(wh_json, "\"elapsed_hours\":");
        wh->collapsed           = extractBool(wh_json, "\"collapsed\":");
        entity->addComponent(std::move(wh));
    }

    // SolarSystem
    std::string ss_json = extractObject(json, "solar_system");
    if (!ss_json.empty()) {
        auto ss = std::make_unique<components::SolarSystem>();
        ss->system_id       = extractString(ss_json, "system_id");
        ss->system_name     = extractString(ss_json, "system_name");
        ss->wormhole_class  = extractInt(ss_json, "\"wormhole_class\":");
        ss->effect_name     = extractString(ss_json, "effect_name");
        ss->sleepers_spawned = extractBool(ss_json, "\"sleepers_spawned\":");
        entity->addComponent(std::move(ss));
    }

    // FleetMembership
    std::string fm_json = extractObject(json, "fleet_membership");
    if (!fm_json.empty()) {
        auto fm = std::make_unique<components::FleetMembership>();
        fm->fleet_id = extractString(fm_json, "fleet_id");
        fm->role     = extractString(fm_json, "role");
        fm->squad_id = extractString(fm_json, "squad_id");
        fm->wing_id  = extractString(fm_json, "wing_id");
        entity->addComponent(std::move(fm));
    }

    return true;
}

// ---------------------------------------------------------------------------
// Lightweight JSON helpers
// ---------------------------------------------------------------------------

std::string WorldPersistence::extractString(const std::string& json,
                                            const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return "";

    pos = json.find('\"', pos + 1);
    if (pos == std::string::npos) return "";

    size_t end = json.find('\"', pos + 1);
    if (end == std::string::npos) return "";

    return json.substr(pos + 1, end - pos - 1);
}

float WorldPersistence::extractFloat(const std::string& json,
                                     const std::string& key,
                                     float fallback) {
    size_t pos = json.find(key);
    if (pos == std::string::npos) return fallback;

    pos += key.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;

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

int WorldPersistence::extractInt(const std::string& json,
                                 const std::string& key,
                                 int fallback) {
    size_t pos = json.find(key);
    if (pos == std::string::npos) return fallback;

    pos += key.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;

    try {
        size_t end = pos;
        while (end < json.size() &&
               (json[end] == '-' || (json[end] >= '0' && json[end] <= '9'))) {
            ++end;
        }
        return std::stoi(json.substr(pos, end - pos));
    } catch (...) {
        return fallback;
    }
}

double WorldPersistence::extractDouble(const std::string& json,
                                       const std::string& key,
                                       double fallback) {
    size_t pos = json.find(key);
    if (pos == std::string::npos) return fallback;

    pos += key.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;

    try {
        size_t end = pos;
        while (end < json.size() &&
               (json[end] == '-' || json[end] == '.' ||
                (json[end] >= '0' && json[end] <= '9') ||
                json[end] == 'e' || json[end] == 'E' || json[end] == '+')) {
            ++end;
        }
        return std::stod(json.substr(pos, end - pos));
    } catch (...) {
        return fallback;
    }
}

bool WorldPersistence::extractBool(const std::string& json,
                                   const std::string& key,
                                   bool fallback) {
    size_t pos = json.find(key);
    if (pos == std::string::npos) return fallback;

    pos += key.size();
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;

    if (pos + 4 <= json.size() && json.substr(pos, 4) == "true")  return true;
    if (pos + 5 <= json.size() && json.substr(pos, 5) == "false") return false;
    return fallback;
}

std::string WorldPersistence::extractObject(const std::string& json,
                                            const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    pos = json.find('{', pos + search.size());
    if (pos == std::string::npos) return "";

    int depth = 0;
    for (size_t i = pos; i < json.size(); ++i) {
        if (json[i] == '{') ++depth;
        else if (json[i] == '}') {
            --depth;
            if (depth == 0) {
                return json.substr(pos, i - pos + 1);
            }
        }
    }

    return "";
}

} // namespace data
} // namespace eve
