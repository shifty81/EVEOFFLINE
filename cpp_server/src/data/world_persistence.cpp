#include "data/world_persistence.h"
#include "components/game_components.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <zlib.h>

namespace atlas {
namespace data {

// Escape a string for safe JSON embedding
static std::string escapeJson(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '\"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    // Skip other control characters
                } else {
                    result += c;
                }
                break;
        }
    }
    return result;
}

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
    json << "{\"id\":\"" << escapeJson(entity->getId()) << "\"";

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
             << "\"ship_type\":\"" << escapeJson(ship->ship_type) << "\""
             << ",\"ship_class\":\"" << escapeJson(ship->ship_class) << "\""
             << ",\"ship_name\":\"" << escapeJson(ship->ship_name) << "\""
             << ",\"race\":\"" << escapeJson(ship->race) << "\""
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
             << "\"faction_name\":\"" << escapeJson(fac->faction_name) << "\""
             << "}";
    }

    // Standings
    auto* standings = entity->getComponent<components::Standings>();
    if (standings) {
        json << ",\"standings\":{";
        
        // Serialize personal standings
        if (!standings->personal_standings.empty()) {
            json << "\"personal\":{";
            bool first = true;
            for (const auto& [entity_id, standing] : standings->personal_standings) {
                if (!first) json << ",";
                json << "\"" << escapeJson(entity_id) << "\":" << standing;
                first = false;
            }
            json << "}";
        }
        
        // Serialize corporation standings
        if (!standings->corporation_standings.empty()) {
            if (!standings->personal_standings.empty()) json << ",";
            json << "\"corporation\":{";
            bool first = true;
            for (const auto& [corp_name, standing] : standings->corporation_standings) {
                if (!first) json << ",";
                json << "\"" << escapeJson(corp_name) << "\":" << standing;
                first = false;
            }
            json << "}";
        }
        
        // Serialize faction standings
        if (!standings->faction_standings.empty()) {
            if (!standings->personal_standings.empty() || !standings->corporation_standings.empty()) {
                json << ",";
            }
            json << "\"faction\":{";
            bool first = true;
            for (const auto& [faction_name, standing] : standings->faction_standings) {
                if (!first) json << ",";
                json << "\"" << escapeJson(faction_name) << "\":" << standing;
                first = false;
            }
            json << "}";
        }
        
        json << "}";
    }

    // AI
    auto* ai = entity->getComponent<components::AI>();
    if (ai) {
        json << ",\"ai\":{"
             << "\"behavior\":" << static_cast<int>(ai->behavior)
             << ",\"state\":" << static_cast<int>(ai->state)
             << ",\"target_entity_id\":\"" << escapeJson(ai->target_entity_id) << "\""
             << ",\"orbit_distance\":" << ai->orbit_distance
             << ",\"awareness_range\":" << ai->awareness_range
             << "}";
    }

    // Weapon
    auto* weapon = entity->getComponent<components::Weapon>();
    if (weapon) {
        json << ",\"weapon\":{"
             << "\"weapon_type\":\"" << escapeJson(weapon->weapon_type) << "\""
             << ",\"damage_type\":\"" << escapeJson(weapon->damage_type) << "\""
             << ",\"damage\":" << weapon->damage
             << ",\"optimal_range\":" << weapon->optimal_range
             << ",\"falloff_range\":" << weapon->falloff_range
             << ",\"tracking_speed\":" << weapon->tracking_speed
             << ",\"rate_of_fire\":" << weapon->rate_of_fire
             << ",\"capacitor_cost\":" << weapon->capacitor_cost
             << ",\"ammo_type\":\"" << escapeJson(weapon->ammo_type) << "\""
             << ",\"ammo_count\":" << weapon->ammo_count
             << "}";
    }

    // Player
    auto* player = entity->getComponent<components::Player>();
    if (player) {
        json << ",\"player\":{"
             << "\"player_id\":\"" << escapeJson(player->player_id) << "\""
             << ",\"character_name\":\"" << escapeJson(player->character_name) << "\""
             << ",\"isk\":" << player->isk
             << ",\"corporation\":\"" << escapeJson(player->corporation) << "\""
             << "}";
    }

    // WormholeConnection
    auto* wh = entity->getComponent<components::WormholeConnection>();
    if (wh) {
        json << ",\"wormhole_connection\":{"
             << "\"wormhole_id\":\"" << escapeJson(wh->wormhole_id) << "\""
             << ",\"source_system\":\"" << escapeJson(wh->source_system) << "\""
             << ",\"destination_system\":\"" << escapeJson(wh->destination_system) << "\""
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
             << "\"system_id\":\"" << escapeJson(ss->system_id) << "\""
             << ",\"system_name\":\"" << escapeJson(ss->system_name) << "\""
             << ",\"wormhole_class\":" << ss->wormhole_class
             << ",\"effect_name\":\"" << escapeJson(ss->effect_name) << "\""
             << ",\"dormants_spawned\":" << (ss->dormants_spawned ? "true" : "false")
             << "}";
    }

    // FleetMembership
    auto* fm = entity->getComponent<components::FleetMembership>();
    if (fm) {
        json << ",\"fleet_membership\":{"
             << "\"fleet_id\":\"" << escapeJson(fm->fleet_id) << "\""
             << ",\"role\":\"" << escapeJson(fm->role) << "\""
             << ",\"squad_id\":\"" << escapeJson(fm->squad_id) << "\""
             << ",\"wing_id\":\"" << escapeJson(fm->wing_id) << "\""
             << "}";
    }

    // Inventory
    auto* inv = entity->getComponent<components::Inventory>();
    if (inv) {
        json << ",\"inventory\":{"
             << "\"max_capacity\":" << inv->max_capacity
             << ",\"items\":[";
        bool first_item = true;
        for (const auto& item : inv->items) {
            if (!first_item) json << ",";
            first_item = false;
            json << "{\"item_id\":\"" << escapeJson(item.item_id) << "\""
                 << ",\"name\":\"" << escapeJson(item.name) << "\""
                 << ",\"type\":\"" << escapeJson(item.type) << "\""
                 << ",\"quantity\":" << item.quantity
                 << ",\"volume\":" << item.volume << "}";
        }
        json << "]}";
    }

    // LootTable
    auto* lt = entity->getComponent<components::LootTable>();
    if (lt) {
        json << ",\"loot_table\":{"
             << "\"isk_drop\":" << lt->isk_drop
             << ",\"entries\":[";
        bool first_entry = true;
        for (const auto& entry : lt->entries) {
            if (!first_entry) json << ",";
            first_entry = false;
            json << "{\"item_id\":\"" << escapeJson(entry.item_id) << "\""
                 << ",\"name\":\"" << escapeJson(entry.name) << "\""
                 << ",\"type\":\"" << escapeJson(entry.type) << "\""
                 << ",\"drop_chance\":" << entry.drop_chance
                 << ",\"min_quantity\":" << entry.min_quantity
                 << ",\"max_quantity\":" << entry.max_quantity
                 << ",\"volume\":" << entry.volume << "}";
        }
        json << "]}";
    }

    // Corporation
    auto* corp = entity->getComponent<components::Corporation>();
    if (corp) {
        json << ",\"corporation_data\":{"
             << "\"corp_id\":\"" << escapeJson(corp->corp_id) << "\""
             << ",\"corp_name\":\"" << escapeJson(corp->corp_name) << "\""
             << ",\"ticker\":\"" << escapeJson(corp->ticker) << "\""
             << ",\"ceo_id\":\"" << escapeJson(corp->ceo_id) << "\""
             << ",\"tax_rate\":" << corp->tax_rate
             << ",\"corp_wallet\":" << corp->corp_wallet
             << ",\"member_ids\":[";
        bool first_mid = true;
        for (const auto& mid : corp->member_ids) {
            if (!first_mid) json << ",";
            first_mid = false;
            json << "\"" << escapeJson(mid) << "\"";
        }
        json << "],\"hangar_items\":[";
        bool first_hi = true;
        for (const auto& item : corp->hangar_items) {
            if (!first_hi) json << ",";
            first_hi = false;
            json << "{\"item_id\":\"" << escapeJson(item.item_id) << "\""
                 << ",\"name\":\"" << escapeJson(item.name) << "\""
                 << ",\"type\":\"" << escapeJson(item.type) << "\""
                 << ",\"quantity\":" << item.quantity
                 << ",\"volume\":" << item.volume << "}";
        }
        json << "]}";
    }

    // DroneBay
    auto* db = entity->getComponent<components::DroneBay>();
    if (db) {
        json << ",\"drone_bay\":{"
             << "\"bay_capacity\":" << db->bay_capacity
             << ",\"max_bandwidth\":" << db->max_bandwidth
             << ",\"stored\":[";
        bool first_s = true;
        for (const auto& d : db->stored_drones) {
            if (!first_s) json << ",";
            first_s = false;
            json << "{\"drone_id\":\"" << escapeJson(d.drone_id) << "\""
                 << ",\"name\":\"" << escapeJson(d.name) << "\""
                 << ",\"type\":\"" << escapeJson(d.type) << "\""
                 << ",\"damage_type\":\"" << escapeJson(d.damage_type) << "\""
                 << ",\"damage\":" << d.damage
                 << ",\"rate_of_fire\":" << d.rate_of_fire
                 << ",\"optimal_range\":" << d.optimal_range
                 << ",\"hitpoints\":" << d.hitpoints
                 << ",\"current_hp\":" << d.current_hp
                 << ",\"bandwidth_use\":" << d.bandwidth_use
                 << ",\"volume\":" << d.volume << "}";
        }
        json << "],\"deployed\":[";
        bool first_d = true;
        for (const auto& d : db->deployed_drones) {
            if (!first_d) json << ",";
            first_d = false;
            json << "{\"drone_id\":\"" << escapeJson(d.drone_id) << "\""
                 << ",\"name\":\"" << escapeJson(d.name) << "\""
                 << ",\"type\":\"" << escapeJson(d.type) << "\""
                 << ",\"damage_type\":\"" << escapeJson(d.damage_type) << "\""
                 << ",\"damage\":" << d.damage
                 << ",\"rate_of_fire\":" << d.rate_of_fire
                 << ",\"optimal_range\":" << d.optimal_range
                 << ",\"hitpoints\":" << d.hitpoints
                 << ",\"current_hp\":" << d.current_hp
                 << ",\"bandwidth_use\":" << d.bandwidth_use
                 << ",\"volume\":" << d.volume << "}";
        }
        json << "]}";
    }

    // ContractBoard
    auto* cb = entity->getComponent<components::ContractBoard>();
    if (cb) {
        json << ",\"contract_board\":{\"contracts\":[";
        bool first_c = true;
        for (const auto& c : cb->contracts) {
            if (!first_c) json << ",";
            first_c = false;
            json << "{\"contract_id\":\"" << escapeJson(c.contract_id) << "\""
                 << ",\"issuer_id\":\"" << escapeJson(c.issuer_id) << "\""
                 << ",\"assignee_id\":\"" << escapeJson(c.assignee_id) << "\""
                 << ",\"type\":\"" << escapeJson(c.type) << "\""
                 << ",\"status\":\"" << escapeJson(c.status) << "\""
                 << ",\"isk_reward\":" << c.isk_reward
                 << ",\"isk_collateral\":" << c.isk_collateral
                 << ",\"duration_remaining\":" << c.duration_remaining
                 << ",\"days_to_complete\":" << c.days_to_complete
                 << ",\"items_offered\":[";
            bool first_io = true;
            for (const auto& item : c.items_offered) {
                if (!first_io) json << ",";
                first_io = false;
                json << "{\"item_id\":\"" << escapeJson(item.item_id) << "\""
                     << ",\"name\":\"" << escapeJson(item.name) << "\""
                     << ",\"quantity\":" << item.quantity
                     << ",\"volume\":" << item.volume << "}";
            }
            json << "],\"items_requested\":[";
            bool first_ir = true;
            for (const auto& item : c.items_requested) {
                if (!first_ir) json << ",";
                first_ir = false;
                json << "{\"item_id\":\"" << escapeJson(item.item_id) << "\""
                     << ",\"name\":\"" << escapeJson(item.name) << "\""
                     << ",\"quantity\":" << item.quantity
                     << ",\"volume\":" << item.volume << "}";
            }
            json << "]}";
        }
        json << "]}";
    }

    // Station
    auto* sta = entity->getComponent<components::Station>();
    if (sta) {
        json << ",\"station\":{"
             << "\"station_name\":\"" << escapeJson(sta->station_name) << "\""
             << ",\"docking_range\":" << sta->docking_range
             << ",\"repair_cost_per_hp\":" << sta->repair_cost_per_hp
             << ",\"docked_count\":" << sta->docked_count
             << "}";
    }

    // Docked
    auto* dck = entity->getComponent<components::Docked>();
    if (dck) {
        json << ",\"docked\":{"
             << "\"station_id\":\"" << escapeJson(dck->station_id) << "\""
             << "}";
    }

    // Wreck
    auto* wrk = entity->getComponent<components::Wreck>();
    if (wrk) {
        json << ",\"wreck\":{"
             << "\"source_entity_id\":\"" << escapeJson(wrk->source_entity_id) << "\""
             << ",\"lifetime_remaining\":" << wrk->lifetime_remaining
             << ",\"salvaged\":" << (wrk->salvaged ? "true" : "false")
             << "}";
    }

    // CaptainPersonality
    auto* cp = entity->getComponent<components::CaptainPersonality>();
    if (cp) {
        json << ",\"captain_personality\":{"
             << "\"aggression\":" << cp->aggression
             << ",\"sociability\":" << cp->sociability
             << ",\"optimism\":" << cp->optimism
             << ",\"professionalism\":" << cp->professionalism
             << ",\"loyalty\":" << cp->loyalty
             << ",\"paranoia\":" << cp->paranoia
             << ",\"ambition\":" << cp->ambition
             << ",\"adaptability\":" << cp->adaptability
             << ",\"captain_name\":\"" << escapeJson(cp->captain_name) << "\""
             << ",\"faction\":\"" << escapeJson(cp->faction) << "\""
             << "}";
    }

    // FleetMorale
    auto* fmor = entity->getComponent<components::FleetMorale>();
    if (fmor) {
        json << ",\"fleet_morale\":{"
             << "\"morale_score\":" << fmor->morale_score
             << ",\"wins\":" << fmor->wins
             << ",\"losses\":" << fmor->losses
             << ",\"ships_lost\":" << fmor->ships_lost
             << ",\"times_saved_by_player\":" << fmor->times_saved_by_player
             << ",\"times_player_saved\":" << fmor->times_player_saved
             << ",\"missions_together\":" << fmor->missions_together
             << ",\"morale_state\":\"" << escapeJson(fmor->morale_state) << "\""
             << "}";
    }

    // CaptainRelationship
    auto* cr = entity->getComponent<components::CaptainRelationship>();
    if (cr) {
        json << ",\"captain_relationship\":{\"relationships\":[";
        bool first_r = true;
        for (const auto& r : cr->relationships) {
            if (!first_r) json << ",";
            first_r = false;
            json << "{\"other_captain_id\":\"" << escapeJson(r.other_captain_id) << "\""
                 << ",\"affinity\":" << r.affinity << "}";
        }
        json << "]}";
    }

    // EmotionalState
    auto* es = entity->getComponent<components::EmotionalState>();
    if (es) {
        json << ",\"emotional_state\":{"
             << "\"confidence\":" << es->confidence
             << ",\"trust_in_player\":" << es->trust_in_player
             << ",\"fatigue\":" << es->fatigue
             << ",\"hope\":" << es->hope
             << "}";
    }

    // CaptainMemory
    auto* cm = entity->getComponent<components::CaptainMemory>();
    if (cm) {
        json << ",\"captain_memory\":{\"max_memories\":" << cm->max_memories
             << ",\"memories\":[";
        bool first_m = true;
        for (const auto& m : cm->memories) {
            if (!first_m) json << ",";
            first_m = false;
            json << "{\"event_type\":\"" << escapeJson(m.event_type) << "\""
                 << ",\"context\":\"" << escapeJson(m.context) << "\""
                 << ",\"timestamp\":" << m.timestamp
                 << ",\"emotional_weight\":" << m.emotional_weight << "}";
        }
        json << "]}";
    }

    // FleetFormation
    auto* ff = entity->getComponent<components::FleetFormation>();
    if (ff) {
        json << ",\"fleet_formation\":{"
             << "\"formation\":" << static_cast<int>(ff->formation)
             << ",\"slot_index\":" << ff->slot_index
             << ",\"offset_x\":" << ff->offset_x
             << ",\"offset_y\":" << ff->offset_y
             << ",\"offset_z\":" << ff->offset_z
             << ",\"spacing_modifier\":" << ff->spacing_modifier
             << "}";
    }

    // FleetCargoPool
    auto* fcp = entity->getComponent<components::FleetCargoPool>();
    if (fcp) {
        json << ",\"fleet_cargo_pool\":{"
             << "\"total_capacity\":" << fcp->total_capacity
             << ",\"used_capacity\":" << fcp->used_capacity
             << ",\"pooled_items\":{";
        bool first_pi = true;
        for (const auto& [item_type, qty] : fcp->pooled_items) {
            if (!first_pi) json << ",";
            first_pi = false;
            json << "\"" << escapeJson(item_type) << "\":" << qty;
        }
        json << "},\"contributor_ship_ids\":[";
        bool first_cs = true;
        for (const auto& sid : fcp->contributor_ship_ids) {
            if (!first_cs) json << ",";
            first_cs = false;
            json << "\"" << escapeJson(sid) << "\"";
        }
        json << "]}";
    }

    // RumorLog
    auto* rl = entity->getComponent<components::RumorLog>();
    if (rl) {
        json << ",\"rumor_log\":{\"rumors\":[";
        bool first_ru = true;
        for (const auto& r : rl->rumors) {
            if (!first_ru) json << ",";
            first_ru = false;
            json << "{\"rumor_id\":\"" << escapeJson(r.rumor_id) << "\""
                 << ",\"text\":\"" << escapeJson(r.text) << "\""
                 << ",\"belief_strength\":" << r.belief_strength
                 << ",\"personally_witnessed\":" << (r.personally_witnessed ? "true" : "false")
                 << ",\"times_heard\":" << r.times_heard << "}";
        }
        json << "]}";
    }

    // MineralDeposit
    auto* md = entity->getComponent<components::MineralDeposit>();
    if (md) {
        json << ",\"mineral_deposit\":{"
             << "\"mineral_type\":\"" << escapeJson(md->mineral_type) << "\""
             << ",\"quantity_remaining\":" << md->quantity_remaining
             << ",\"max_quantity\":" << md->max_quantity
             << ",\"yield_rate\":" << md->yield_rate
             << ",\"volume_per_unit\":" << md->volume_per_unit
             << "}";
    }

    // SystemResources
    auto* sr = entity->getComponent<components::SystemResources>();
    if (sr) {
        json << ",\"system_resources\":{\"resources\":[";
        bool first_sr = true;
        for (const auto& r : sr->resources) {
            if (!first_sr) json << ",";
            first_sr = false;
            json << "{\"mineral_type\":\"" << escapeJson(r.mineral_type) << "\""
                 << ",\"total_quantity\":" << r.total_quantity
                 << ",\"remaining_quantity\":" << r.remaining_quantity << "}";
        }
        json << "]}";
    }

    // MarketHub
    auto* mh = entity->getComponent<components::MarketHub>();
    if (mh) {
        json << ",\"market_hub\":{"
             << "\"station_id\":\"" << escapeJson(mh->station_id) << "\""
             << ",\"broker_fee_rate\":" << mh->broker_fee_rate
             << ",\"sales_tax_rate\":" << mh->sales_tax_rate
             << ",\"orders\":[";
        bool first_o = true;
        for (const auto& o : mh->orders) {
            if (!first_o) json << ",";
            first_o = false;
            json << "{\"order_id\":\"" << escapeJson(o.order_id) << "\""
                 << ",\"item_id\":\"" << escapeJson(o.item_id) << "\""
                 << ",\"item_name\":\"" << escapeJson(o.item_name) << "\""
                 << ",\"owner_id\":\"" << escapeJson(o.owner_id) << "\""
                 << ",\"is_buy_order\":" << (o.is_buy_order ? "true" : "false")
                 << ",\"price_per_unit\":" << o.price_per_unit
                 << ",\"quantity\":" << o.quantity
                 << ",\"quantity_remaining\":" << o.quantity_remaining
                 << ",\"duration_remaining\":" << o.duration_remaining
                 << ",\"fulfilled\":" << (o.fulfilled ? "true" : "false") << "}";
        }
        json << "]}";
    }

    // AnomalyVisualCue
    auto* avc = entity->getComponent<components::AnomalyVisualCue>();
    if (avc) {
        json << ",\"anomaly_visual_cue\":{"
             << "\"anomaly_id\":\"" << escapeJson(avc->anomaly_id) << "\""
             << ",\"cue_type\":" << static_cast<int>(avc->cue_type)
             << ",\"intensity\":" << avc->intensity
             << ",\"radius\":" << avc->radius
             << ",\"pulse_frequency\":" << avc->pulse_frequency
             << ",\"r\":" << avc->r
             << ",\"g\":" << avc->g
             << ",\"b\":" << avc->b
             << ",\"distortion_strength\":" << avc->distortion_strength
             << ",\"active\":" << (avc->active ? "true" : "false") << "}";
    }

    // LODPriority
    auto* lod = entity->getComponent<components::LODPriority>();
    if (lod) {
        json << ",\"lod_priority\":{"
             << "\"priority\":" << lod->priority
             << ",\"force_visible\":" << (lod->force_visible ? "true" : "false")
             << ",\"impostor_distance\":" << lod->impostor_distance << "}";
    }

    // WarpProfile
    auto* wp = entity->getComponent<components::WarpProfile>();
    if (wp) {
        json << ",\"warp_profile\":{"
             << "\"warp_speed\":" << wp->warp_speed
             << ",\"mass_norm\":" << wp->mass_norm
             << ",\"intensity\":" << wp->intensity
             << ",\"comfort_scale\":" << wp->comfort_scale << "}";
    }

    // WarpVisual
    auto* wv = entity->getComponent<components::WarpVisual>();
    if (wv) {
        json << ",\"warp_visual\":{"
             << "\"distortion_strength\":" << wv->distortion_strength
             << ",\"tunnel_noise_scale\":" << wv->tunnel_noise_scale
             << ",\"vignette_amount\":" << wv->vignette_amount
             << ",\"bloom_strength\":" << wv->bloom_strength
             << ",\"starfield_speed\":" << wv->starfield_speed << "}";
    }

    // WarpEvent
    auto* we = entity->getComponent<components::WarpEvent>();
    if (we) {
        json << ",\"warp_event\":{"
             << "\"current_event\":\"" << we->current_event << "\""
             << ",\"event_timer\":" << we->event_timer
             << ",\"severity\":" << we->severity << "}";
    }

    // TacticalProjection
    auto* tp = entity->getComponent<components::TacticalProjection>();
    if (tp) {
        json << ",\"tactical_projection\":{"
             << "\"projected_x\":" << tp->projected_x
             << ",\"projected_y\":" << tp->projected_y
             << ",\"vertical_offset\":" << tp->vertical_offset
             << ",\"visible\":" << (tp->visible ? "true" : "false") << "}";
    }

    // PlayerPresence
    auto* pp = entity->getComponent<components::PlayerPresence>();
    if (pp) {
        json << ",\"player_presence\":{"
             << "\"time_since_last_command\":" << pp->time_since_last_command
             << ",\"time_since_last_speech\":" << pp->time_since_last_speech << "}";
    }

    // FactionCulture
    auto* fc = entity->getComponent<components::FactionCulture>();
    if (fc) {
        json << ",\"faction_culture\":{"
             << "\"faction\":\"" << fc->faction << "\""
             << ",\"chatter_frequency_mod\":" << fc->chatter_frequency_mod
             << ",\"formation_tightness_mod\":" << fc->formation_tightness_mod
             << ",\"morale_sensitivity\":" << fc->morale_sensitivity
             << ",\"risk_tolerance\":" << fc->risk_tolerance << "}";
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

    // Standings
    std::string standings_json = extractObject(json, "standings");
    if (!standings_json.empty()) {
        auto standings = std::make_unique<components::Standings>();
        
        // Deserialize personal standings
        std::string personal_json = extractObject(standings_json, "personal");
        if (!personal_json.empty()) {
            // Parse the personal standings map
            // Format: {"entity_id": standing_value, ...}
            size_t pos = 0;
            while (pos < personal_json.size()) {
                // Find next key
                size_t key_start = personal_json.find("\"", pos);
                if (key_start == std::string::npos) break;
                key_start++;
                size_t key_end = personal_json.find("\"", key_start);
                if (key_end == std::string::npos) break;
                
                std::string entity_id = personal_json.substr(key_start, key_end - key_start);
                
                // Find value after colon
                size_t colon = personal_json.find(":", key_end);
                if (colon == std::string::npos) break;
                
                // Extract number
                size_t val_start = colon + 1;
                while (val_start < personal_json.size() && 
                       (personal_json[val_start] == ' ' || personal_json[val_start] == '\t')) {
                    val_start++;
                }
                size_t val_end = val_start;
                while (val_end < personal_json.size() && 
                       (std::isdigit(personal_json[val_end]) || personal_json[val_end] == '.' || 
                        personal_json[val_end] == '-' || personal_json[val_end] == '+')) {
                    val_end++;
                }
                
                if (val_end > val_start) {
                    float standing = std::stof(personal_json.substr(val_start, val_end - val_start));
                    standings->personal_standings[entity_id] = standing;
                }
                
                pos = val_end + 1;
            }
        }
        
        // Deserialize corporation standings
        std::string corp_json = extractObject(standings_json, "corporation");
        if (!corp_json.empty()) {
            size_t pos = 0;
            while (pos < corp_json.size()) {
                size_t key_start = corp_json.find("\"", pos);
                if (key_start == std::string::npos) break;
                key_start++;
                size_t key_end = corp_json.find("\"", key_start);
                if (key_end == std::string::npos) break;
                
                std::string corp_name = corp_json.substr(key_start, key_end - key_start);
                size_t colon = corp_json.find(":", key_end);
                if (colon == std::string::npos) break;
                
                size_t val_start = colon + 1;
                while (val_start < corp_json.size() && 
                       (corp_json[val_start] == ' ' || corp_json[val_start] == '\t')) {
                    val_start++;
                }
                size_t val_end = val_start;
                while (val_end < corp_json.size() && 
                       (std::isdigit(corp_json[val_end]) || corp_json[val_end] == '.' || 
                        corp_json[val_end] == '-' || corp_json[val_end] == '+')) {
                    val_end++;
                }
                
                if (val_end > val_start) {
                    float standing = std::stof(corp_json.substr(val_start, val_end - val_start));
                    standings->corporation_standings[corp_name] = standing;
                }
                
                pos = val_end + 1;
            }
        }
        
        // Deserialize faction standings
        std::string faction_json = extractObject(standings_json, "faction");
        if (!faction_json.empty()) {
            size_t pos = 0;
            while (pos < faction_json.size()) {
                size_t key_start = faction_json.find("\"", pos);
                if (key_start == std::string::npos) break;
                key_start++;
                size_t key_end = faction_json.find("\"", key_start);
                if (key_end == std::string::npos) break;
                
                std::string faction_name = faction_json.substr(key_start, key_end - key_start);
                size_t colon = faction_json.find(":", key_end);
                if (colon == std::string::npos) break;
                
                size_t val_start = colon + 1;
                while (val_start < faction_json.size() && 
                       (faction_json[val_start] == ' ' || faction_json[val_start] == '\t')) {
                    val_start++;
                }
                size_t val_end = val_start;
                while (val_end < faction_json.size() && 
                       (std::isdigit(faction_json[val_end]) || faction_json[val_end] == '.' || 
                        faction_json[val_end] == '-' || faction_json[val_end] == '+')) {
                    val_end++;
                }
                
                if (val_end > val_start) {
                    float standing = std::stof(faction_json.substr(val_start, val_end - val_start));
                    standings->faction_standings[faction_name] = standing;
                }
                
                pos = val_end + 1;
            }
        }
        
        entity->addComponent(std::move(standings));
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
        ss->dormants_spawned = extractBool(ss_json, "\"dormants_spawned\":");
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

    // Inventory
    std::string inv_json = extractObject(json, "inventory");
    if (!inv_json.empty()) {
        auto inv = std::make_unique<components::Inventory>();
        inv->max_capacity = extractFloat(inv_json, "\"max_capacity\":", 400.0f);

        // Parse items array
        size_t arr_start = inv_json.find("[");
        size_t arr_end = inv_json.rfind("]");
        if (arr_start != std::string::npos && arr_end != std::string::npos && arr_end > arr_start) {
            std::string items_content = inv_json.substr(arr_start + 1, arr_end - arr_start - 1);
            int depth = 0;
            size_t obj_start = std::string::npos;
            for (size_t i = 0; i < items_content.size(); ++i) {
                if (items_content[i] == '{') {
                    if (depth == 0) obj_start = i;
                    ++depth;
                } else if (items_content[i] == '}') {
                    --depth;
                    if (depth == 0 && obj_start != std::string::npos) {
                        std::string item_json = items_content.substr(obj_start, i - obj_start + 1);
                        components::Inventory::Item item;
                        item.item_id  = extractString(item_json, "item_id");
                        item.name     = extractString(item_json, "name");
                        item.type     = extractString(item_json, "type");
                        item.quantity = extractInt(item_json, "\"quantity\":");
                        item.volume   = extractFloat(item_json, "\"volume\":", 1.0f);
                        inv->items.push_back(item);
                        obj_start = std::string::npos;
                    }
                }
            }
        }
        entity->addComponent(std::move(inv));
    }

    // LootTable
    std::string lt_json = extractObject(json, "loot_table");
    if (!lt_json.empty()) {
        auto lt = std::make_unique<components::LootTable>();
        lt->isk_drop = extractDouble(lt_json, "\"isk_drop\":");

        // Parse entries array
        size_t arr_start = lt_json.find("[");
        size_t arr_end = lt_json.rfind("]");
        if (arr_start != std::string::npos && arr_end != std::string::npos && arr_end > arr_start) {
            std::string entries_content = lt_json.substr(arr_start + 1, arr_end - arr_start - 1);
            int depth = 0;
            size_t obj_start = std::string::npos;
            for (size_t i = 0; i < entries_content.size(); ++i) {
                if (entries_content[i] == '{') {
                    if (depth == 0) obj_start = i;
                    ++depth;
                } else if (entries_content[i] == '}') {
                    --depth;
                    if (depth == 0 && obj_start != std::string::npos) {
                        std::string entry_json = entries_content.substr(obj_start, i - obj_start + 1);
                        components::LootTable::LootEntry entry;
                        entry.item_id      = extractString(entry_json, "item_id");
                        entry.name         = extractString(entry_json, "name");
                        entry.type         = extractString(entry_json, "type");
                        entry.drop_chance  = extractFloat(entry_json, "\"drop_chance\":", 1.0f);
                        entry.min_quantity = extractInt(entry_json, "\"min_quantity\":", 1);
                        entry.max_quantity = extractInt(entry_json, "\"max_quantity\":", 1);
                        entry.volume       = extractFloat(entry_json, "\"volume\":", 1.0f);
                        lt->entries.push_back(entry);
                        obj_start = std::string::npos;
                    }
                }
            }
        }
        entity->addComponent(std::move(lt));
    }

    // Corporation
    std::string corp_json = extractObject(json, "corporation_data");
    if (!corp_json.empty()) {
        auto corp = std::make_unique<components::Corporation>();
        corp->corp_id    = extractString(corp_json, "corp_id");
        corp->corp_name  = extractString(corp_json, "corp_name");
        corp->ticker     = extractString(corp_json, "ticker");
        corp->ceo_id     = extractString(corp_json, "ceo_id");
        corp->tax_rate   = extractFloat(corp_json, "\"tax_rate\":", 0.05f);
        corp->corp_wallet = extractDouble(corp_json, "\"corp_wallet\":", 0.0);

        // Parse member_ids string array
        std::string mid_key = "\"member_ids\"";
        size_t mid_pos = corp_json.find(mid_key);
        if (mid_pos != std::string::npos) {
            size_t arr_s = corp_json.find("[", mid_pos);
            size_t arr_e = corp_json.find("]", arr_s);
            if (arr_s != std::string::npos && arr_e != std::string::npos) {
                std::string arr_content = corp_json.substr(arr_s + 1, arr_e - arr_s - 1);
                size_t q1 = 0;
                while ((q1 = arr_content.find("\"", q1)) != std::string::npos) {
                    size_t q2 = arr_content.find("\"", q1 + 1);
                    if (q2 == std::string::npos) break;
                    corp->member_ids.push_back(arr_content.substr(q1 + 1, q2 - q1 - 1));
                    q1 = q2 + 1;
                }
            }
        }

        // Parse hangar_items array
        std::string hi_key = "\"hangar_items\"";
        size_t hi_pos = corp_json.find(hi_key);
        if (hi_pos != std::string::npos) {
            size_t arr_start = corp_json.find("[", hi_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < corp_json.size() && bracket_depth > 0) {
                    if (corp_json[arr_end] == '[') ++bracket_depth;
                    else if (corp_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = corp_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string ij = content.substr(obj_start, i - obj_start + 1);
                                components::Corporation::CorpHangarItem item;
                                item.item_id  = extractString(ij, "item_id");
                                item.name     = extractString(ij, "name");
                                item.type     = extractString(ij, "type");
                                item.quantity = extractInt(ij, "\"quantity\":");
                                item.volume   = extractFloat(ij, "\"volume\":", 1.0f);
                                corp->hangar_items.push_back(item);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }

        entity->addComponent(std::move(corp));
    }

    // DroneBay
    std::string db_json = extractObject(json, "drone_bay");
    if (!db_json.empty()) {
        auto db = std::make_unique<components::DroneBay>();
        db->bay_capacity  = extractFloat(db_json, "\"bay_capacity\":", 25.0f);
        db->max_bandwidth = extractInt(db_json, "\"max_bandwidth\":", 25);

        // Helper lambda to parse a drone array
        auto parseDrones = [&](const std::string& array_key,
                               std::vector<components::DroneBay::DroneInfo>& out) {
            std::string key_search = "\"" + array_key + "\"";
            size_t key_pos = db_json.find(key_search);
            if (key_pos == std::string::npos) return;
            size_t arr_start = db_json.find("[", key_pos);
            // Find matching close bracket
            if (arr_start == std::string::npos) return;
            int bracket_depth = 1;
            size_t arr_end = arr_start + 1;
            while (arr_end < db_json.size() && bracket_depth > 0) {
                if (db_json[arr_end] == '[') ++bracket_depth;
                else if (db_json[arr_end] == ']') --bracket_depth;
                if (bracket_depth > 0) ++arr_end;
            }
            if (bracket_depth != 0) return;

            std::string content = db_json.substr(arr_start + 1, arr_end - arr_start - 1);
            int depth = 0;
            size_t obj_start = std::string::npos;
            for (size_t i = 0; i < content.size(); ++i) {
                if (content[i] == '{') {
                    if (depth == 0) obj_start = i;
                    ++depth;
                } else if (content[i] == '}') {
                    --depth;
                    if (depth == 0 && obj_start != std::string::npos) {
                        std::string dj = content.substr(obj_start, i - obj_start + 1);
                        components::DroneBay::DroneInfo info;
                        info.drone_id      = extractString(dj, "drone_id");
                        info.name          = extractString(dj, "name");
                        info.type          = extractString(dj, "type");
                        info.damage_type   = extractString(dj, "damage_type");
                        info.damage        = extractFloat(dj, "\"damage\":", 0.0f);
                        info.rate_of_fire  = extractFloat(dj, "\"rate_of_fire\":", 3.0f);
                        info.optimal_range = extractFloat(dj, "\"optimal_range\":", 5000.0f);
                        info.hitpoints     = extractFloat(dj, "\"hitpoints\":", 45.0f);
                        info.current_hp    = extractFloat(dj, "\"current_hp\":", 45.0f);
                        info.bandwidth_use = extractInt(dj, "\"bandwidth_use\":", 5);
                        info.volume        = extractFloat(dj, "\"volume\":", 5.0f);
                        out.push_back(info);
                        obj_start = std::string::npos;
                    }
                }
            }
        };

        parseDrones("stored",  db->stored_drones);
        parseDrones("deployed", db->deployed_drones);

        entity->addComponent(std::move(db));
    }

    // ContractBoard
    std::string cb_json = extractObject(json, "contract_board");
    if (!cb_json.empty()) {
        auto cb = std::make_unique<components::ContractBoard>();

        std::string contracts_key = "\"contracts\"";
        size_t ck_pos = cb_json.find(contracts_key);
        if (ck_pos != std::string::npos) {
            size_t arr_start = cb_json.find("[", ck_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < cb_json.size() && bracket_depth > 0) {
                    if (cb_json[arr_end] == '[') ++bracket_depth;
                    else if (cb_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = cb_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string cj = content.substr(obj_start, i - obj_start + 1);
                                components::ContractBoard::Contract contract;
                                contract.contract_id       = extractString(cj, "contract_id");
                                contract.issuer_id         = extractString(cj, "issuer_id");
                                contract.assignee_id       = extractString(cj, "assignee_id");
                                contract.type              = extractString(cj, "type");
                                contract.status            = extractString(cj, "status");
                                contract.isk_reward        = extractDouble(cj, "\"isk_reward\":", 0.0);
                                contract.isk_collateral    = extractDouble(cj, "\"isk_collateral\":", 0.0);
                                contract.duration_remaining = extractFloat(cj, "\"duration_remaining\":", -1.0f);
                                contract.days_to_complete  = extractFloat(cj, "\"days_to_complete\":", 3.0f);

                                auto parseItems = [&](const std::string& key,
                                                      std::vector<components::ContractBoard::ContractItem>& out) {
                                    std::string k = "\"" + key + "\"";
                                    size_t kp = cj.find(k);
                                    if (kp == std::string::npos) return;
                                    size_t as = cj.find("[", kp);
                                    if (as == std::string::npos) return;
                                    int bd = 1;
                                    size_t ae = as + 1;
                                    while (ae < cj.size() && bd > 0) {
                                        if (cj[ae] == '[') ++bd;
                                        else if (cj[ae] == ']') --bd;
                                        if (bd > 0) ++ae;
                                    }
                                    if (bd != 0) return;
                                    std::string ic = cj.substr(as + 1, ae - as - 1);
                                    int id2 = 0;
                                    size_t os2 = std::string::npos;
                                    for (size_t j = 0; j < ic.size(); ++j) {
                                        if (ic[j] == '{') {
                                            if (id2 == 0) os2 = j;
                                            ++id2;
                                        } else if (ic[j] == '}') {
                                            --id2;
                                            if (id2 == 0 && os2 != std::string::npos) {
                                                std::string ij = ic.substr(os2, j - os2 + 1);
                                                components::ContractBoard::ContractItem item;
                                                item.item_id  = extractString(ij, "item_id");
                                                item.name     = extractString(ij, "name");
                                                item.quantity = extractInt(ij, "\"quantity\":");
                                                item.volume   = extractFloat(ij, "\"volume\":", 1.0f);
                                                out.push_back(item);
                                                os2 = std::string::npos;
                                            }
                                        }
                                    }
                                };

                                parseItems("items_offered", contract.items_offered);
                                parseItems("items_requested", contract.items_requested);

                                cb->contracts.push_back(contract);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }

        entity->addComponent(std::move(cb));
    }

    // Station
    std::string sta_json = extractObject(json, "station");
    if (!sta_json.empty()) {
        auto sta = std::make_unique<components::Station>();
        sta->station_name      = extractString(sta_json, "station_name");
        sta->docking_range     = extractFloat(sta_json, "\"docking_range\":", 2500.0f);
        sta->repair_cost_per_hp = extractFloat(sta_json, "\"repair_cost_per_hp\":", 1.0f);
        sta->docked_count      = extractInt(sta_json, "\"docked_count\":");
        entity->addComponent(std::move(sta));
    }

    // Docked
    std::string dck_json = extractObject(json, "docked");
    if (!dck_json.empty()) {
        auto dck = std::make_unique<components::Docked>();
        dck->station_id = extractString(dck_json, "station_id");
        entity->addComponent(std::move(dck));
    }

    // Wreck
    std::string wrk_json = extractObject(json, "wreck");
    if (!wrk_json.empty()) {
        auto wrk = std::make_unique<components::Wreck>();
        wrk->source_entity_id   = extractString(wrk_json, "source_entity_id");
        wrk->lifetime_remaining = extractFloat(wrk_json, "\"lifetime_remaining\":", 1800.0f);
        wrk->salvaged           = extractBool(wrk_json, "\"salvaged\":", false);
        entity->addComponent(std::move(wrk));
    }

    // CaptainPersonality
    std::string cp_json = extractObject(json, "captain_personality");
    if (!cp_json.empty()) {
        auto cp = std::make_unique<components::CaptainPersonality>();
        cp->aggression       = extractFloat(cp_json, "\"aggression\":", 0.5f);
        cp->sociability      = extractFloat(cp_json, "\"sociability\":", 0.5f);
        cp->optimism         = extractFloat(cp_json, "\"optimism\":", 0.5f);
        cp->professionalism  = extractFloat(cp_json, "\"professionalism\":", 0.5f);
        cp->loyalty          = extractFloat(cp_json, "\"loyalty\":", 0.5f);
        cp->paranoia         = extractFloat(cp_json, "\"paranoia\":", 0.5f);
        cp->ambition         = extractFloat(cp_json, "\"ambition\":", 0.5f);
        cp->adaptability     = extractFloat(cp_json, "\"adaptability\":", 0.5f);
        cp->captain_name     = extractString(cp_json, "captain_name");
        cp->faction          = extractString(cp_json, "faction");
        entity->addComponent(std::move(cp));
    }

    // FleetMorale
    std::string fmor_json = extractObject(json, "fleet_morale");
    if (!fmor_json.empty()) {
        auto fmor = std::make_unique<components::FleetMorale>();
        fmor->morale_score          = extractFloat(fmor_json, "\"morale_score\":", 0.0f);
        fmor->wins                  = extractInt(fmor_json, "\"wins\":");
        fmor->losses                = extractInt(fmor_json, "\"losses\":");
        fmor->ships_lost            = extractInt(fmor_json, "\"ships_lost\":");
        fmor->times_saved_by_player = extractInt(fmor_json, "\"times_saved_by_player\":");
        fmor->times_player_saved    = extractInt(fmor_json, "\"times_player_saved\":");
        fmor->missions_together     = extractInt(fmor_json, "\"missions_together\":");
        fmor->morale_state          = extractString(fmor_json, "morale_state");
        if (fmor->morale_state.empty()) fmor->morale_state = "Steady";
        entity->addComponent(std::move(fmor));
    }

    // CaptainRelationship
    std::string cr_json = extractObject(json, "captain_relationship");
    if (!cr_json.empty()) {
        auto cr = std::make_unique<components::CaptainRelationship>();
        std::string rel_key = "\"relationships\"";
        size_t rel_pos = cr_json.find(rel_key);
        if (rel_pos != std::string::npos) {
            size_t arr_start = cr_json.find("[", rel_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < cr_json.size() && bracket_depth > 0) {
                    if (cr_json[arr_end] == '[') ++bracket_depth;
                    else if (cr_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = cr_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string rj = content.substr(obj_start, i - obj_start + 1);
                                components::CaptainRelationship::Relationship rel;
                                rel.other_captain_id = extractString(rj, "other_captain_id");
                                rel.affinity         = extractFloat(rj, "\"affinity\":", 0.0f);
                                cr->relationships.push_back(rel);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }
        entity->addComponent(std::move(cr));
    }

    // EmotionalState
    std::string es_json = extractObject(json, "emotional_state");
    if (!es_json.empty()) {
        auto es = std::make_unique<components::EmotionalState>();
        es->confidence      = extractFloat(es_json, "\"confidence\":", 50.0f);
        es->trust_in_player = extractFloat(es_json, "\"trust_in_player\":", 50.0f);
        es->fatigue         = extractFloat(es_json, "\"fatigue\":", 0.0f);
        es->hope            = extractFloat(es_json, "\"hope\":", 50.0f);
        entity->addComponent(std::move(es));
    }

    // CaptainMemory
    std::string cm_json = extractObject(json, "captain_memory");
    if (!cm_json.empty()) {
        auto cm = std::make_unique<components::CaptainMemory>();
        cm->max_memories = extractInt(cm_json, "\"max_memories\":", 50);
        std::string mem_key = "\"memories\"";
        size_t mem_pos = cm_json.find(mem_key);
        if (mem_pos != std::string::npos) {
            size_t arr_start = cm_json.find("[", mem_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < cm_json.size() && bracket_depth > 0) {
                    if (cm_json[arr_end] == '[') ++bracket_depth;
                    else if (cm_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = cm_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string mj = content.substr(obj_start, i - obj_start + 1);
                                components::CaptainMemory::MemoryEntry entry;
                                entry.event_type       = extractString(mj, "event_type");
                                entry.context          = extractString(mj, "context");
                                entry.timestamp        = extractFloat(mj, "\"timestamp\":", 0.0f);
                                entry.emotional_weight = extractFloat(mj, "\"emotional_weight\":", 0.0f);
                                cm->memories.push_back(entry);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }
        entity->addComponent(std::move(cm));
    }

    // FleetFormation
    std::string ff_json = extractObject(json, "fleet_formation");
    if (!ff_json.empty()) {
        auto ff = std::make_unique<components::FleetFormation>();
        int ft = extractInt(ff_json, "\"formation\":");
        ff->formation  = static_cast<components::FleetFormation::FormationType>(ft);
        ff->slot_index = extractInt(ff_json, "\"slot_index\":");
        ff->offset_x   = extractFloat(ff_json, "\"offset_x\":", 0.0f);
        ff->offset_y   = extractFloat(ff_json, "\"offset_y\":", 0.0f);
        ff->offset_z   = extractFloat(ff_json, "\"offset_z\":", 0.0f);
        ff->spacing_modifier = extractFloat(ff_json, "\"spacing_modifier\":", 1.0f);
        entity->addComponent(std::move(ff));
    }

    // FleetCargoPool
    std::string fcp_json = extractObject(json, "fleet_cargo_pool");
    if (!fcp_json.empty()) {
        auto fcp = std::make_unique<components::FleetCargoPool>();
        fcp->total_capacity = static_cast<uint64_t>(extractDouble(fcp_json, "\"total_capacity\":", 0.0));
        fcp->used_capacity  = static_cast<uint64_t>(extractDouble(fcp_json, "\"used_capacity\":", 0.0));

        std::string pi_json = extractObject(fcp_json, "pooled_items");
        if (!pi_json.empty()) {
            size_t pos = 0;
            while (pos < pi_json.size()) {
                size_t q1 = pi_json.find('\"', pos);
                if (q1 == std::string::npos) break;
                size_t q2 = pi_json.find('\"', q1 + 1);
                if (q2 == std::string::npos) break;
                std::string key = pi_json.substr(q1 + 1, q2 - q1 - 1);
                size_t colon = pi_json.find(':', q2 + 1);
                if (colon == std::string::npos) break;
                size_t vs = colon + 1;
                while (vs < pi_json.size() && pi_json[vs] == ' ') ++vs;
                size_t ve = vs;
                while (ve < pi_json.size() && (std::isdigit(pi_json[ve]) || pi_json[ve] == '.' || pi_json[ve] == '-' || pi_json[ve] == 'e' || pi_json[ve] == 'E' || pi_json[ve] == '+')) ++ve;
                if (ve > vs) {
                    uint64_t val = static_cast<uint64_t>(std::stod(pi_json.substr(vs, ve - vs)));
                    fcp->pooled_items[key] = val;
                }
                pos = ve;
            }
        }

        std::string cs_key = "\"contributor_ship_ids\"";
        size_t cs_pos = fcp_json.find(cs_key);
        if (cs_pos != std::string::npos) {
            size_t arr_start = fcp_json.find("[", cs_pos);
            size_t arr_end = fcp_json.find("]", arr_start);
            if (arr_start != std::string::npos && arr_end != std::string::npos) {
                std::string arr_content = fcp_json.substr(arr_start + 1, arr_end - arr_start - 1);
                size_t q1 = 0;
                while ((q1 = arr_content.find("\"", q1)) != std::string::npos) {
                    size_t q2 = arr_content.find("\"", q1 + 1);
                    if (q2 == std::string::npos) break;
                    fcp->contributor_ship_ids.push_back(arr_content.substr(q1 + 1, q2 - q1 - 1));
                    q1 = q2 + 1;
                }
            }
        }

        entity->addComponent(std::move(fcp));
    }

    // RumorLog
    std::string rl_json = extractObject(json, "rumor_log");
    if (!rl_json.empty()) {
        auto rl = std::make_unique<components::RumorLog>();
        std::string ru_key = "\"rumors\"";
        size_t ru_pos = rl_json.find(ru_key);
        if (ru_pos != std::string::npos) {
            size_t arr_start = rl_json.find("[", ru_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < rl_json.size() && bracket_depth > 0) {
                    if (rl_json[arr_end] == '[') ++bracket_depth;
                    else if (rl_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = rl_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string rj = content.substr(obj_start, i - obj_start + 1);
                                components::RumorLog::Rumor rumor;
                                rumor.rumor_id             = extractString(rj, "rumor_id");
                                rumor.text                 = extractString(rj, "text");
                                rumor.belief_strength      = extractFloat(rj, "\"belief_strength\":", 0.5f);
                                rumor.personally_witnessed = extractBool(rj, "\"personally_witnessed\":", false);
                                rumor.times_heard          = extractInt(rj, "\"times_heard\":");
                                rl->rumors.push_back(rumor);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }
        entity->addComponent(std::move(rl));
    }

    // MineralDeposit
    std::string md_json = extractObject(json, "mineral_deposit");
    if (!md_json.empty()) {
        auto md = std::make_unique<components::MineralDeposit>();
        md->mineral_type       = extractString(md_json, "mineral_type");
        if (md->mineral_type.empty()) md->mineral_type = "Veldspar";
        md->quantity_remaining = extractFloat(md_json, "\"quantity_remaining\":", 10000.0f);
        md->max_quantity       = extractFloat(md_json, "\"max_quantity\":", 10000.0f);
        md->yield_rate         = extractFloat(md_json, "\"yield_rate\":", 1.0f);
        md->volume_per_unit    = extractFloat(md_json, "\"volume_per_unit\":", 0.1f);
        entity->addComponent(std::move(md));
    }

    // SystemResources
    std::string sr_json = extractObject(json, "system_resources");
    if (!sr_json.empty()) {
        auto sr = std::make_unique<components::SystemResources>();
        std::string res_key = "\"resources\"";
        size_t res_pos = sr_json.find(res_key);
        if (res_pos != std::string::npos) {
            size_t arr_start = sr_json.find("[", res_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < sr_json.size() && bracket_depth > 0) {
                    if (sr_json[arr_end] == '[') ++bracket_depth;
                    else if (sr_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = sr_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string rj = content.substr(obj_start, i - obj_start + 1);
                                components::SystemResources::ResourceEntry entry;
                                entry.mineral_type       = extractString(rj, "mineral_type");
                                entry.total_quantity     = extractFloat(rj, "\"total_quantity\":", 0.0f);
                                entry.remaining_quantity = extractFloat(rj, "\"remaining_quantity\":", 0.0f);
                                sr->resources.push_back(entry);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }
        entity->addComponent(std::move(sr));
    }

    // MarketHub
    std::string mh_json = extractObject(json, "market_hub");
    if (!mh_json.empty()) {
        auto mh = std::make_unique<components::MarketHub>();
        mh->station_id     = extractString(mh_json, "station_id");
        mh->broker_fee_rate = extractDouble(mh_json, "\"broker_fee_rate\":", 0.02);
        mh->sales_tax_rate  = extractDouble(mh_json, "\"sales_tax_rate\":", 0.04);

        std::string ord_key = "\"orders\"";
        size_t ord_pos = mh_json.find(ord_key);
        if (ord_pos != std::string::npos) {
            size_t arr_start = mh_json.find("[", ord_pos);
            if (arr_start != std::string::npos) {
                int bracket_depth = 1;
                size_t arr_end = arr_start + 1;
                while (arr_end < mh_json.size() && bracket_depth > 0) {
                    if (mh_json[arr_end] == '[') ++bracket_depth;
                    else if (mh_json[arr_end] == ']') --bracket_depth;
                    if (bracket_depth > 0) ++arr_end;
                }
                if (bracket_depth == 0) {
                    std::string content = mh_json.substr(arr_start + 1, arr_end - arr_start - 1);
                    int depth = 0;
                    size_t obj_start = std::string::npos;
                    for (size_t i = 0; i < content.size(); ++i) {
                        if (content[i] == '{') {
                            if (depth == 0) obj_start = i;
                            ++depth;
                        } else if (content[i] == '}') {
                            --depth;
                            if (depth == 0 && obj_start != std::string::npos) {
                                std::string oj = content.substr(obj_start, i - obj_start + 1);
                                components::MarketHub::Order order;
                                order.order_id           = extractString(oj, "order_id");
                                order.item_id            = extractString(oj, "item_id");
                                order.item_name          = extractString(oj, "item_name");
                                order.owner_id           = extractString(oj, "owner_id");
                                order.is_buy_order       = extractBool(oj, "\"is_buy_order\":", false);
                                order.price_per_unit     = extractDouble(oj, "\"price_per_unit\":", 0.0);
                                order.quantity           = extractInt(oj, "\"quantity\":", 1);
                                order.quantity_remaining = extractInt(oj, "\"quantity_remaining\":", 1);
                                order.duration_remaining = extractFloat(oj, "\"duration_remaining\":", -1.0f);
                                order.fulfilled          = extractBool(oj, "\"fulfilled\":", false);
                                mh->orders.push_back(order);
                                obj_start = std::string::npos;
                            }
                        }
                    }
                }
            }
        }

        entity->addComponent(std::move(mh));
    }

    // AnomalyVisualCue
    std::string avc_json = extractObject(json, "anomaly_visual_cue");
    if (!avc_json.empty()) {
        auto avc = std::make_unique<components::AnomalyVisualCue>();
        avc->anomaly_id = extractString(avc_json, "anomaly_id");
        avc->cue_type = static_cast<components::AnomalyVisualCue::CueType>(
            extractInt(avc_json, "\"cue_type\":", 5));
        avc->intensity = extractFloat(avc_json, "\"intensity\":");
        avc->radius = extractFloat(avc_json, "\"radius\":");
        avc->pulse_frequency = extractFloat(avc_json, "\"pulse_frequency\":");
        avc->r = extractFloat(avc_json, "\"r\":");
        avc->g = extractFloat(avc_json, "\"g\":");
        avc->b = extractFloat(avc_json, "\"b\":");
        avc->distortion_strength = extractFloat(avc_json, "\"distortion_strength\":");
        avc->active = extractBool(avc_json, "\"active\":", true);
        entity->addComponent(std::move(avc));
    }

    // LODPriority
    std::string lod_json = extractObject(json, "lod_priority");
    if (!lod_json.empty()) {
        auto lod = std::make_unique<components::LODPriority>();
        lod->priority = extractFloat(lod_json, "\"priority\":");
        lod->force_visible = extractBool(lod_json, "\"force_visible\":", false);
        lod->impostor_distance = extractFloat(lod_json, "\"impostor_distance\":");
        entity->addComponent(std::move(lod));
    }

    // WarpProfile
    std::string wp_json = extractObject(json, "warp_profile");
    if (!wp_json.empty()) {
        auto wp = std::make_unique<components::WarpProfile>();
        wp->warp_speed = extractFloat(wp_json, "\"warp_speed\":");
        wp->mass_norm = extractFloat(wp_json, "\"mass_norm\":");
        wp->intensity = extractFloat(wp_json, "\"intensity\":");
        wp->comfort_scale = extractFloat(wp_json, "\"comfort_scale\":");
        entity->addComponent(std::move(wp));
    }

    // WarpVisual
    std::string wv_json = extractObject(json, "warp_visual");
    if (!wv_json.empty()) {
        auto wv = std::make_unique<components::WarpVisual>();
        wv->distortion_strength = extractFloat(wv_json, "\"distortion_strength\":");
        wv->tunnel_noise_scale = extractFloat(wv_json, "\"tunnel_noise_scale\":");
        wv->vignette_amount = extractFloat(wv_json, "\"vignette_amount\":");
        wv->bloom_strength = extractFloat(wv_json, "\"bloom_strength\":");
        wv->starfield_speed = extractFloat(wv_json, "\"starfield_speed\":");
        entity->addComponent(std::move(wv));
    }

    // WarpEvent
    std::string we_json = extractObject(json, "warp_event");
    if (!we_json.empty()) {
        auto we = std::make_unique<components::WarpEvent>();
        we->current_event = extractString(we_json, "current_event");
        we->event_timer = extractFloat(we_json, "\"event_timer\":");
        we->severity = extractInt(we_json, "\"severity\":", 0);
        entity->addComponent(std::move(we));
    }

    // TacticalProjection
    std::string tp_json = extractObject(json, "tactical_projection");
    if (!tp_json.empty()) {
        auto tp = std::make_unique<components::TacticalProjection>();
        tp->projected_x = extractFloat(tp_json, "\"projected_x\":");
        tp->projected_y = extractFloat(tp_json, "\"projected_y\":");
        tp->vertical_offset = extractFloat(tp_json, "\"vertical_offset\":");
        tp->visible = extractBool(tp_json, "\"visible\":", true);
        entity->addComponent(std::move(tp));
    }

    // PlayerPresence
    std::string pp_json = extractObject(json, "player_presence");
    if (!pp_json.empty()) {
        auto pp = std::make_unique<components::PlayerPresence>();
        pp->time_since_last_command = extractFloat(pp_json, "\"time_since_last_command\":");
        pp->time_since_last_speech = extractFloat(pp_json, "\"time_since_last_speech\":");
        entity->addComponent(std::move(pp));
    }

    // FactionCulture
    std::string fc_json = extractObject(json, "faction_culture");
    if (!fc_json.empty()) {
        auto fc = std::make_unique<components::FactionCulture>();
        fc->faction = extractString(fc_json, "faction");
        fc->chatter_frequency_mod = extractFloat(fc_json, "\"chatter_frequency_mod\":");
        fc->formation_tightness_mod = extractFloat(fc_json, "\"formation_tightness_mod\":");
        fc->morale_sensitivity = extractFloat(fc_json, "\"morale_sensitivity\":");
        fc->risk_tolerance = extractFloat(fc_json, "\"risk_tolerance\":");
        entity->addComponent(std::move(fc));
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

// ---------------------------------------------------------------------------
// Compressed Save / Load  (gzip via zlib)
// ---------------------------------------------------------------------------

bool WorldPersistence::saveWorldCompressed(const ecs::World* world,
                                           const std::string& filepath) {
    std::string json = serializeWorld(world);
    if (json.empty()) {
        std::cerr << "[WorldPersistence] Nothing to compress" << std::endl;
        return false;
    }

    gzFile gz = gzopen(filepath.c_str(), "wb9");  // max compression
    if (!gz) {
        std::cerr << "[WorldPersistence] Cannot open compressed file for writing: "
                  << filepath << std::endl;
        return false;
    }

    int written = gzwrite(gz, json.data(), static_cast<unsigned>(json.size()));
    gzclose(gz);

    if (written <= 0) {
        std::cerr << "[WorldPersistence] Compressed write failed" << std::endl;
        return false;
    }

    std::cout << "[WorldPersistence] World saved (compressed) to " << filepath
              << " (" << written << " bytes source, compressed on disk)" << std::endl;
    return true;
}

bool WorldPersistence::loadWorldCompressed(ecs::World* world,
                                           const std::string& filepath) {
    gzFile gz = gzopen(filepath.c_str(), "rb");
    if (!gz) {
        std::cerr << "[WorldPersistence] Cannot open compressed file for reading: "
                  << filepath << std::endl;
        return false;
    }

    std::string json;
    char buf[8192];
    int n;
    while ((n = gzread(gz, buf, sizeof(buf))) > 0) {
        json.append(buf, static_cast<size_t>(n));
    }
    gzclose(gz);

    if (json.empty()) {
        std::cerr << "[WorldPersistence] Decompressed data is empty" << std::endl;
        return false;
    }

    return deserializeWorld(world, json);
}

} // namespace data
} // namespace atlas
