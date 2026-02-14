#ifndef EVE_COMPONENTS_GAME_COMPONENTS_H
#define EVE_COMPONENTS_GAME_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cstdint>

namespace atlas {
namespace components {

/**
 * @brief Position and orientation in 3D space
 */
class Position : public ecs::Component {
public:
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float rotation = 0.0f;  // radians
    
    COMPONENT_TYPE(Position)
};

/**
 * @brief Velocity and movement
 */
class Velocity : public ecs::Component {
public:
    float vx = 0.0f;
    float vy = 0.0f;
    float vz = 0.0f;
    float angular_velocity = 0.0f;
    float max_speed = 100.0f;
    
    COMPONENT_TYPE(Velocity)
};

/**
 * @brief Health pools (shield, armor, hull) like EVE ONLINE
 */
class Health : public ecs::Component {
public:
    // Health pools
    float hull_hp = 100.0f;
    float hull_max = 100.0f;
    float armor_hp = 100.0f;
    float armor_max = 100.0f;
    float shield_hp = 100.0f;
    float shield_max = 100.0f;
    float shield_recharge_rate = 1.0f;  // HP per second
    
    // Hull resistances (0.0 = no resist, 0.5 = 50% resist)
    float hull_em_resist = 0.0f;
    float hull_thermal_resist = 0.0f;
    float hull_kinetic_resist = 0.0f;
    float hull_explosive_resist = 0.0f;
    
    // Armor resistances
    float armor_em_resist = 0.0f;
    float armor_thermal_resist = 0.0f;
    float armor_kinetic_resist = 0.0f;
    float armor_explosive_resist = 0.0f;
    
    // Shield resistances
    float shield_em_resist = 0.0f;
    float shield_thermal_resist = 0.0f;
    float shield_kinetic_resist = 0.0f;
    float shield_explosive_resist = 0.0f;
    
    bool isAlive() const {
        return hull_hp > 0.0f;
    }
    
    COMPONENT_TYPE(Health)
};

/**
 * @brief Energy capacitor like EVE ONLINE
 */
class Capacitor : public ecs::Component {
public:
    float capacitor = 100.0f;
    float capacitor_max = 100.0f;
    float recharge_rate = 2.0f;  // GJ per second
    
    COMPONENT_TYPE(Capacitor)
};

/**
 * @brief Ship-specific data
 */
class Ship : public ecs::Component {
public:
    std::string ship_type = "Frigate";
    std::string ship_class = "Frigate";
    std::string ship_name = "Fang";
    std::string race = "Keldari";
    
    // Fitting resources
    float cpu = 0.0f;
    float cpu_max = 100.0f;
    float powergrid = 0.0f;
    float powergrid_max = 50.0f;
    
    // Signature and targeting
    float signature_radius = 35.0f;  // meters
    float scan_resolution = 400.0f;  // mm
    int max_locked_targets = 3;
    float max_targeting_range = 20000.0f;  // meters
    
    // Warp parameters (per ship class, from warp_mechanics.json)
    float warp_speed_au = 5.0f;      // AU/s (frigate default)
    float align_time = 2.5f;          // seconds to align for warp (frigate default)
    int warp_strength = 1;             // warp core strength (points needed to disrupt)
    
    COMPONENT_TYPE(Ship)
};

/**
 * @brief Targeting information
 */
class Target : public ecs::Component {
public:
    std::vector<std::string> locked_targets;  // entity IDs
    std::map<std::string, float> locking_targets;  // entity_id: progress (0-1)
    
    COMPONENT_TYPE(Target)
};

/**
 * @brief Weapon system
 */
class Weapon : public ecs::Component {
public:
    std::string weapon_type = "Projectile";  // Projectile, Energy, Missile, Hybrid
    std::string damage_type = "kinetic";  // em, thermal, kinetic, explosive
    float damage = 10.0f;
    float optimal_range = 5000.0f;  // meters
    float falloff_range = 2500.0f;  // meters
    float tracking_speed = 0.5f;  // radians per second
    float rate_of_fire = 3.0f;  // seconds between shots
    float cooldown = 0.0f;  // current cooldown timer
    float capacitor_cost = 5.0f;  // GJ per shot
    std::string ammo_type = "EMP";
    int ammo_count = 100;
    
    COMPONENT_TYPE(Weapon)
};

/**
 * @brief AI behavior for NPCs
 */
class AI : public ecs::Component {
public:
    enum class Behavior {
        Aggressive,
        Defensive,
        Passive,
        Flee
    };
    
    enum class State {
        Idle,
        Approaching,
        Orbiting,
        Fleeing,
        Attacking,
        Mining
    };
    
    /**
     * @brief Target selection strategy for AI combat
     */
    enum class TargetSelection {
        Closest,     // Target nearest entity (default)
        LowestHP,    // Target entity with lowest total HP fraction
        HighestThreat // Target entity dealing the most damage to us
    };
    
    Behavior behavior = Behavior::Aggressive;
    State state = State::Idle;
    std::string target_entity_id;
    float orbit_distance = 1000.0f;  // preferred orbit distance (0 = auto from ship class)
    float awareness_range = 50000.0f;  // meters
    float flee_threshold = 0.25f;  // flee when total HP (shield+armor+hull) below this fraction of max
    TargetSelection target_selection = TargetSelection::Closest;  // how to pick targets
    bool use_dynamic_orbit = false;  // if true, orbit_distance set from ship class
    float engagement_range = 0.0f;  // 0 = derive from weapon optimal+falloff
    
    COMPONENT_TYPE(AI)
};

/**
 * @brief Player-controlled entity
 */
class Player : public ecs::Component {
public:
    std::string player_id;
    std::string character_name = "Pilot";
    double isk = 1000000.0;  // Starting ISK
    std::string corporation = "NPC Corp";
    
    COMPONENT_TYPE(Player)
};

/**
 * @brief Faction affiliation
 */
class Faction : public ecs::Component {
public:
    std::string faction_name = "Neutral";  // Veyren, Aurelian, Solari, Keldari, Venom Syndicate, etc.
    std::map<std::string, float> standings;  // faction_name: standing (-10 to +10)
    
    COMPONENT_TYPE(Faction)
};

/**
 * @brief Personal standings with entities, corporations, and factions
 * 
 * Tracks relationships on a -10 to +10 scale:
 * - Personal standings: Individual player/NPC relationships
 * - Corporation standings: Corporation-level relationships
 * - Faction standings: Faction-wide relationships
 * 
 * Standings affect:
 * - Agent access (requires positive corp/faction standing)
 * - NPC aggression (negative standings cause attacks)
 * - Market taxes and broker fees
 * - Mission rewards and LP gains
 */
class Standings : public ecs::Component {
public:
    // Personal standings with individual entities (player_id or npc_id)
    std::map<std::string, float> personal_standings;
    
    // Corporation standings (corporation_name: standing)
    std::map<std::string, float> corporation_standings;
    
    // Faction standings (faction_name: standing) 
    // Duplicated from Faction component for convenience
    std::map<std::string, float> faction_standings;
    
    /**
     * @brief Get standing with an entity
     * Checks personal, then corporation, then faction standings in order
     * @return Standing value from -10 to +10, or 0 if no standing exists
     */
    float getStandingWith(const std::string& entity_id, 
                         const std::string& entity_corp = "",
                         const std::string& entity_faction = "") const {
        // Check personal standing first (highest priority)
        auto personal_it = personal_standings.find(entity_id);
        if (personal_it != personal_standings.end()) {
            return personal_it->second;
        }
        
        // Check corporation standing
        if (!entity_corp.empty()) {
            auto corp_it = corporation_standings.find(entity_corp);
            if (corp_it != corporation_standings.end()) {
                return corp_it->second;
            }
        }
        
        // Check faction standing (lowest priority)
        if (!entity_faction.empty()) {
            auto faction_it = faction_standings.find(entity_faction);
            if (faction_it != faction_standings.end()) {
                return faction_it->second;
            }
        }
        
        return 0.0f;  // Neutral if no standing found
    }
    
    /**
     * @brief Modify standing with clamping to valid range
     * @param standing_map The map to modify (personal, corp, or faction)
     * @param key The entity/corp/faction identifier
     * @param change Amount to change (can be negative)
     */
    static void modifyStanding(std::map<std::string, float>& standing_map,
                              const std::string& key,
                              float change) {
        float current = 0.0f;
        auto it = standing_map.find(key);
        if (it != standing_map.end()) {
            current = it->second;
        }
        
        // Apply change and clamp to -10 to +10
        float new_standing = current + change;
        new_standing = (std::max)(-10.0f, (std::min)(10.0f, new_standing));
        standing_map[key] = new_standing;
    }
    
    COMPONENT_TYPE(Standings)
};

/**
 * @brief Solar system properties for wormhole space
 *
 * Tracks the wormhole class (C1-C6), active system-wide effects,
 * and whether dormant NPCs have already been spawned.
 */
class SolarSystem : public ecs::Component {
public:
    std::string system_id;
    std::string system_name;
    int wormhole_class = 0;               // 0 = k-space, 1-6 = wormhole class
    std::string effect_name;              // e.g. "magnetar", "pulsar", "" for none
    bool dormants_spawned = false;
    
    COMPONENT_TYPE(SolarSystem)
};

/**
 * @brief A wormhole connection between two systems
 *
 * Models mass limits, remaining stability, and lifetime so that
 * the WormholeSystem can decay and eventually collapse connections.
 */
class WormholeConnection : public ecs::Component {
public:
    std::string wormhole_id;
    std::string source_system;            // system entity id
    std::string destination_system;       // system entity id
    double max_mass = 500000000.0;        // kg total mass allowed
    double remaining_mass = 500000000.0;  // kg remaining before collapse
    double max_jump_mass = 20000000.0;    // kg max single-ship mass
    float max_lifetime_hours = 24.0f;     // hours until natural collapse
    float elapsed_hours = 0.0f;           // hours elapsed since spawn
    bool collapsed = false;
    
    bool isStable() const {
        return !collapsed && elapsed_hours < max_lifetime_hours && remaining_mass > 0.0;
    }
    
    COMPONENT_TYPE(WormholeConnection)
};

/**
 * @brief Fleet membership for an entity (attached to each fleet member)
 *
 * Tracks which fleet a player belongs to, their role, and any
 * active fleet bonuses being applied.
 */
class FleetMembership : public ecs::Component {
public:
    std::string fleet_id;
    std::string role = "Member";  // "FleetCommander", "WingCommander", "SquadCommander", "Member"
    std::string squad_id;
    std::string wing_id;
    std::map<std::string, float> active_bonuses;  // e.g. "armor_hp_bonus" -> 0.10
    
    COMPONENT_TYPE(FleetMembership)
};

/**
 * @brief Active mission tracking for a player entity
 * 
 * Tracks missions the player has accepted, their objectives,
 * and progress. Supports multiple concurrent missions.
 */
class MissionTracker : public ecs::Component {
public:
    struct Objective {
        std::string type;          // "destroy", "mine", "deliver", "reach"
        std::string target;        // entity type or item name
        int required = 1;
        int completed = 0;
        bool done() const { return completed >= required; }
    };

    struct ActiveMission {
        std::string mission_id;
        std::string name;
        int level = 1;
        std::string type;          // "combat", "mining", "courier"
        std::string agent_faction;
        std::vector<Objective> objectives;
        double isk_reward = 0.0;
        double lp_reward = 0.0;
        float standing_reward = 0.0f;
        float time_remaining = -1.0f;  // seconds, -1 = no limit
        bool completed = false;
        bool failed = false;

        bool allObjectivesDone() const {
            for (const auto& obj : objectives)
                if (!obj.done()) return false;
            return !objectives.empty();
        }
    };

    std::vector<ActiveMission> active_missions;
    std::vector<std::string> completed_mission_ids;

    COMPONENT_TYPE(MissionTracker)
};

/**
 * @brief Skill training and bonuses for a player entity
 *
 * Tracks trained skills, current training queue, and provides
 * methods to compute skill bonuses on ship stats.
 */
class SkillSet : public ecs::Component {
public:
    struct TrainedSkill {
        std::string skill_id;
        std::string name;
        int level = 0;           // 0-5
        int max_level = 5;
        float training_multiplier = 1.0f;
    };

    struct QueueEntry {
        std::string skill_id;
        int target_level = 1;
        float time_remaining = 0.0f;  // seconds remaining
    };

    // All trained skills indexed by skill_id
    std::map<std::string, TrainedSkill> skills;

    // Training queue (FIFO)
    std::vector<QueueEntry> training_queue;

    // Total skill points
    double total_sp = 0.0;

    int getSkillLevel(const std::string& skill_id) const {
        auto it = skills.find(skill_id);
        return (it != skills.end()) ? it->second.level : 0;
    }

    COMPONENT_TYPE(SkillSet)
};

/**
 * @brief Module activation state for fitted modules on a ship
 *
 * Tracks which modules are fitted, their activation state,
 * and cycling timers. Separate from Weapon component which
 * handles NPC auto-fire; this handles player-initiated module use.
 */
class ModuleRack : public ecs::Component {
public:
    struct FittedModule {
        std::string module_id;
        std::string name;
        std::string slot_type;     // "high", "mid", "low"
        int slot_index = 0;
        bool active = false;       // currently cycling
        float cycle_time = 5.0f;   // seconds per cycle
        float cycle_progress = 0.0f; // 0-1 progress through current cycle
        float capacitor_cost = 5.0f;
        float cpu_usage = 10.0f;
        float powergrid_usage = 5.0f;

        // Effects applied while active (key: stat_name, value: modifier)
        std::map<std::string, float> effects;
    };

    std::vector<FittedModule> high_slots;
    std::vector<FittedModule> mid_slots;
    std::vector<FittedModule> low_slots;

    COMPONENT_TYPE(ModuleRack)
};

/**
 * @brief Cargo inventory for ships, wrecks, containers
 */
class Inventory : public ecs::Component {
public:
    struct Item {
        std::string item_id;
        std::string name;
        std::string type;        // "weapon", "module", "ammo", "ore", "salvage", "commodity"
        int quantity = 1;
        float volume = 1.0f;     // m3 per unit
    };

    std::vector<Item> items;
    float max_capacity = 400.0f;  // m3 cargo hold

    float usedCapacity() const {
        float total = 0.0f;
        for (const auto& item : items)
            total += item.volume * item.quantity;
        return total;
    }

    float freeCapacity() const {
        return max_capacity - usedCapacity();
    }

    COMPONENT_TYPE(Inventory)
};

/**
 * @brief Loot drop table attached to NPCs
 */
class LootTable : public ecs::Component {
public:
    struct LootEntry {
        std::string item_id;
        std::string name;
        std::string type;
        float drop_chance = 1.0f;  // 0.0-1.0
        int min_quantity = 1;
        int max_quantity = 1;
        float volume = 1.0f;
    };

    std::vector<LootEntry> entries;
    double isk_drop = 0.0;     // ISK bounty

    COMPONENT_TYPE(LootTable)
};

/**
 * @brief Drone bay and deployed drone management
 *
 * Tracks which drones are stored in the drone bay and which are
 * currently deployed in space.  Enforces bandwidth and bay capacity.
 */
class DroneBay : public ecs::Component {
public:
    struct DroneInfo {
        std::string drone_id;
        std::string name;
        std::string type;          // "light_combat_drone", "medium_combat_drone", "mining_drone", "salvage_drone", etc.
        std::string damage_type;   // "em", "thermal", "kinetic", "explosive"
        float damage = 0.0f;
        float rate_of_fire = 3.0f; // seconds between shots
        float cooldown = 0.0f;     // current cooldown timer
        float optimal_range = 5000.0f;
        float hitpoints = 45.0f;
        float current_hp = 45.0f;
        int bandwidth_use = 5;
        float volume = 5.0f;       // m3 per drone
        float mining_yield = 0.0f;  // units of ore per cycle (mining drones)
        float salvage_chance = 0.0f; // probability of successful salvage per cycle (salvage drones)
    };

    std::vector<DroneInfo> stored_drones;    // drones in bay (not deployed)
    std::vector<DroneInfo> deployed_drones;  // drones in space

    float bay_capacity = 25.0f;     // m3 total bay capacity
    int max_bandwidth = 25;         // Mbit/s bandwidth limit

    std::string mining_target_id;   // entity id of deposit for mining drones
    std::string salvage_target_id;  // entity id of wreck for salvage drones

    int usedBandwidth() const {
        int total = 0;
        for (const auto& d : deployed_drones)
            total += d.bandwidth_use;
        return total;
    }

    float usedBayVolume() const {
        float total = 0.0f;
        for (const auto& d : stored_drones)
            total += d.volume;
        for (const auto& d : deployed_drones)
            total += d.volume;
        return total;
    }

    COMPONENT_TYPE(DroneBay)
};

/**
 * @brief Insurance policy on a ship
 */
class InsurancePolicy : public ecs::Component {
public:
    std::string policy_id;
    std::string ship_type;
    std::string tier = "basic";    // "basic", "standard", "platinum"
    float coverage_fraction = 0.5f; // fraction of ship value paid out
    double premium_paid = 0.0;     // ISK paid for policy
    double payout_value = 0.0;     // ISK paid out on loss
    float duration_remaining = -1.0f; // seconds, -1 = permanent
    bool active = true;
    bool claimed = false;

    COMPONENT_TYPE(InsurancePolicy)
};

/**
 * @brief Tracks bounty rewards earned by a player
 */
class BountyLedger : public ecs::Component {
public:
    double total_bounty_earned = 0.0;
    int total_kills = 0;
    
    struct BountyRecord {
        std::string target_id;
        std::string target_name;
        double bounty_amount = 0.0;
        std::string faction;
    };
    
    std::vector<BountyRecord> recent_kills;  // last N kills
    static constexpr int MAX_RECENT = 50;
    
    COMPONENT_TYPE(BountyLedger)
};

/**
 * @brief Market order tracking for stations
 */
class MarketHub : public ecs::Component {
public:
    struct Order {
        std::string order_id;
        std::string item_id;
        std::string item_name;
        std::string owner_id;       // entity that placed the order
        bool is_buy_order = false;   // true = buy, false = sell
        double price_per_unit = 0.0;
        int quantity = 1;
        int quantity_remaining = 1;
        float duration_remaining = -1.0f;  // seconds, -1 = permanent
        bool fulfilled = false;
    };

    std::string station_id;
    std::vector<Order> orders;
    double broker_fee_rate = 0.02;  // 2% broker fee
    double sales_tax_rate = 0.04;   // 4% sales tax

    COMPONENT_TYPE(MarketHub)
};

class Corporation : public ecs::Component {
public:
    std::string corp_id;
    std::string corp_name;
    std::string ticker;
    std::string ceo_id;
    float tax_rate = 0.05f;
    std::vector<std::string> member_ids;
    double corp_wallet = 0.0;

    struct CorpHangarItem {
        std::string item_id;
        std::string name;
        std::string type;
        int quantity = 1;
        float volume = 1.0f;
    };

    std::vector<CorpHangarItem> hangar_items;

    COMPONENT_TYPE(Corporation)
};

class ContractBoard : public ecs::Component {
public:
    struct ContractItem {
        std::string item_id;
        std::string name;
        int quantity = 1;
        float volume = 1.0f;
    };

    struct Contract {
        std::string contract_id;
        std::string issuer_id;
        std::string assignee_id;
        std::string type;            // "item_exchange", "courier", "auction"
        std::string status;          // "outstanding", "in_progress", "completed", "expired", "failed"
        std::vector<ContractItem> items_offered;
        std::vector<ContractItem> items_requested;
        double isk_reward = 0.0;
        double isk_collateral = 0.0;
        float duration_remaining = -1.0f;
        float days_to_complete = 3.0f;
    };

    std::vector<Contract> contracts;

    COMPONENT_TYPE(ContractBoard)
};

/**
 * @brief Planetary Interaction colony on a planet
 *
 * Tracks extractors, processors, and storage for PI resources.
 * Each colony has a CPU and powergrid budget from the planet type.
 */
class PlanetaryColony : public ecs::Component {
public:
    std::string colony_id;
    std::string owner_id;       // player entity id
    std::string planet_type;    // "barren", "temperate", "oceanic", "lava", "gas", "ice", "storm", "plasma"
    std::string system_id;

    struct Extractor {
        std::string extractor_id;
        std::string resource_type;  // e.g. "base_metals", "aqueous_liquids"
        float cycle_time = 3600.0f; // seconds per extraction cycle
        float cycle_progress = 0.0f;
        int quantity_per_cycle = 100;
        bool active = true;
        float cpu_usage = 45.0f;
        float powergrid_usage = 550.0f;
    };

    struct Processor {
        std::string processor_id;
        std::string input_type;
        std::string output_type;
        int input_quantity = 40;     // units consumed per cycle
        int output_quantity = 5;     // units produced per cycle
        float cycle_time = 1800.0f;  // seconds per processing cycle
        float cycle_progress = 0.0f;
        bool active = true;
        float cpu_usage = 200.0f;
        float powergrid_usage = 800.0f;
    };

    struct StoredResource {
        std::string resource_type;
        int quantity = 0;
    };

    std::vector<Extractor> extractors;
    std::vector<Processor> processors;
    std::vector<StoredResource> storage;
    float storage_capacity = 10000.0f;  // units

    float cpu_max = 1675.0f;
    float powergrid_max = 6000.0f;

    float usedCpu() const {
        float total = 0.0f;
        for (const auto& e : extractors) total += e.cpu_usage;
        for (const auto& p : processors) total += p.cpu_usage;
        return total;
    }

    float usedPowergrid() const {
        float total = 0.0f;
        for (const auto& e : extractors) total += e.powergrid_usage;
        for (const auto& p : processors) total += p.powergrid_usage;
        return total;
    }

    int totalStored() const {
        int total = 0;
        for (const auto& s : storage) total += s.quantity;
        return total;
    }

    COMPONENT_TYPE(PlanetaryColony)
};

/**
 * @brief Manufacturing facility for blueprint-based production
 *
 * Tracks manufacturing jobs: blueprint, materials, output,
 * time remaining, and job status.
 */
class ManufacturingFacility : public ecs::Component {
public:
    std::string facility_id;
    std::string station_id;
    int max_jobs = 1;  // concurrent job slots

    struct MaterialRequirement {
        std::string material_id;
        int quantity = 0;
    };

    struct ManufacturingJob {
        std::string job_id;
        std::string blueprint_id;
        std::string owner_id;
        std::string output_item_id;
        std::string output_item_name;
        int output_quantity = 1;
        int runs = 1;
        int runs_completed = 0;
        float time_per_run = 3600.0f;   // seconds per run
        float time_remaining = 3600.0f; // current run time remaining
        std::string status;             // "pending", "active", "completed", "cancelled"
        double install_cost = 0.0;
        std::vector<MaterialRequirement> materials;
        float material_efficiency = 0.0f;  // 0-10, reduces material cost
        float time_efficiency = 0.0f;      // 0-20, reduces time
    };

    std::vector<ManufacturingJob> jobs;

    int activeJobCount() const {
        int count = 0;
        for (const auto& j : jobs)
            if (j.status == "active") ++count;
        return count;
    }

    COMPONENT_TYPE(ManufacturingFacility)
};

/**
 * @brief Research laboratory for invention and blueprint research
 *
 * Supports ME/TE research on blueprints and Tech II invention.
 */
class ResearchLab : public ecs::Component {
public:
    std::string lab_id;
    std::string station_id;
    int max_jobs = 1;

    struct ResearchJob {
        std::string job_id;
        std::string blueprint_id;
        std::string owner_id;
        std::string research_type;       // "material_efficiency", "time_efficiency", "invention"
        float time_remaining = 3600.0f;
        float total_time = 3600.0f;
        std::string status;              // "active", "completed", "failed"
        // ME/TE research
        int target_level = 1;            // target ME or TE level
        // Invention
        std::string output_blueprint_id; // T2 blueprint on success
        float success_chance = 0.5f;     // 0.0-1.0
        std::string datacore_1;
        std::string datacore_2;
        double install_cost = 0.0;
    };

    std::vector<ResearchJob> jobs;

    int activeJobCount() const {
        int count = 0;
        for (const auto& j : jobs)
            if (j.status == "active") ++count;
        return count;
    }

    COMPONENT_TYPE(ResearchLab)
};

/**
 * @brief Chat channel for persistent messaging
 *
 * Supports multiple channel types (local, corp, fleet, private)
 * with message history, moderation, and member tracking.
 */
class ChatChannel : public ecs::Component {
public:
    std::string channel_id;
    std::string channel_name;
    std::string channel_type = "local";  // "local", "corp", "fleet", "alliance", "private"
    std::string owner_id;                // channel creator/owner
    std::string motd;                    // message of the day
    int max_members = 0;                 // 0 = unlimited
    bool is_moderated = false;

    struct ChatMessage {
        std::string message_id;
        std::string sender_id;
        std::string sender_name;
        std::string content;
        std::string timestamp;
        bool is_system_message = false;
    };

    struct ChannelMember {
        std::string player_id;
        std::string player_name;
        std::string role = "member";     // "member", "moderator", "operator", "owner"
        bool is_muted = false;
    };

    std::vector<ChatMessage> messages;
    std::vector<ChannelMember> members;
    int max_history = 200;               // max messages to keep

    int memberCount() const { return static_cast<int>(members.size()); }

    COMPONENT_TYPE(ChatChannel)
};

/**
 * @brief Character sheet for player identity and attributes
 *
 * Tracks race, bloodline, ancestry, clone, implants, and
 * character attributes for creation and progression.
 */
class CharacterSheet : public ecs::Component {
public:
    std::string character_id;
    std::string character_name;
    std::string race = "Caldari";           // "Caldari", "Amarr", "Gallente", "Minmatar"
    std::string bloodline;                   // race-specific bloodline
    std::string ancestry;                    // background/origin
    std::string gender = "male";             // "male", "female"
    float date_of_birth = 0.0f;              // simulation time of creation

    // Attributes (base values, modified by implants)
    int intelligence = 20;
    int perception = 20;
    int charisma = 19;
    int willpower = 20;
    int memory = 20;

    // Clone
    std::string clone_grade = "alpha";       // "alpha", "omega"
    std::string clone_location;              // station ID for medical clone
    int clone_jump_cooldown = 0;             // seconds remaining

    // Implants (slots 1-10)
    struct Implant {
        std::string implant_id;
        std::string implant_name;
        int slot = 0;                        // 1-10
        std::string attribute_bonus;         // attribute boosted
        int bonus_amount = 0;
    };
    std::vector<Implant> implants;

    // Security status
    float security_status = 0.0f;            // -10.0 to 10.0

    // Employment history
    struct EmploymentRecord {
        std::string corp_id;
        std::string corp_name;
        float join_date = 0.0f;
        float leave_date = 0.0f;
    };
    std::vector<EmploymentRecord> employment_history;

    int getEffectiveAttribute(const std::string& attr) const {
        int base = 0;
        if (attr == "intelligence") base = intelligence;
        else if (attr == "perception") base = perception;
        else if (attr == "charisma") base = charisma;
        else if (attr == "willpower") base = willpower;
        else if (attr == "memory") base = memory;

        for (const auto& imp : implants) {
            if (imp.attribute_bonus == attr) base += imp.bonus_amount;
        }
        return base;
    }

    COMPONENT_TYPE(CharacterSheet)
};

/**
 * @brief Tournament bracket for competitive PvE events
 *
 * Tracks tournament lifecycle: registration, active rounds,
 * participant scoring, and final results with rewards.
 */
class Tournament : public ecs::Component {
public:
    std::string tournament_id;
    std::string name;
    std::string status = "registration";  // "registration", "active", "completed", "cancelled"

    int max_participants = 16;
    int current_round = 0;
    int total_rounds = 0;
    float round_duration = 600.0f;        // seconds per round
    float round_timer = 0.0f;             // countdown for current round

    double entry_fee = 0.0;
    double prize_pool = 0.0;

    struct Participant {
        std::string player_id;
        std::string player_name;
        int score = 0;
        int kills = 0;
        bool eliminated = false;
    };

    std::vector<Participant> participants;

    struct RoundResult {
        int round_number = 0;
        std::string winner_id;
        int winner_score = 0;
        int participant_count = 0;
    };

    std::vector<RoundResult> round_results;

    COMPONENT_TYPE(Tournament)
};

/**
 * @brief Leaderboard for tracking player rankings and achievements
 *
 * Aggregates player stats across categories (kills, ISK earned,
 * missions completed, etc.) and tracks unlocked achievements.
 */
class Leaderboard : public ecs::Component {
public:
    struct PlayerEntry {
        std::string player_id;
        std::string player_name;
        int total_kills = 0;
        double total_isk_earned = 0.0;
        int missions_completed = 0;
        int tournaments_won = 0;
        double total_bounty = 0.0;
        int ships_destroyed = 0;
        int ships_lost = 0;
        double total_damage_dealt = 0.0;
    };

    struct Achievement {
        std::string achievement_id;
        std::string name;
        std::string description;
        std::string category;             // "combat", "industry", "exploration", "social"
        int requirement = 1;              // threshold to unlock
        std::string stat_key;             // which stat to check against requirement
    };

    struct UnlockedAchievement {
        std::string achievement_id;
        std::string player_id;
        float unlock_time = 0.0f;
    };

    std::string board_id;
    std::vector<PlayerEntry> entries;
    std::vector<Achievement> achievements;
    std::vector<UnlockedAchievement> unlocked;

    COMPONENT_TYPE(Leaderboard)
};

/**
 * @brief Station entity — represents a dockable station in space
 */
class Station : public ecs::Component {
public:
    std::string station_name;
    float docking_range = 2500.0f;       // metres
    float repair_cost_per_hp = 1.0f;     // ISK per HP repaired
    int docked_count = 0;                // number of ships currently docked

    COMPONENT_TYPE(Station)
};

/**
 * @brief Docked state — attached to entities that are inside a station
 */
class Docked : public ecs::Component {
public:
    std::string station_id;              // entity id of the station

    COMPONENT_TYPE(Docked)
};

/**
 * @brief Wreck entity — remains of a destroyed ship
 */
class Wreck : public ecs::Component {
public:
    std::string source_entity_id;        // entity that was destroyed
    float lifetime_remaining = 1800.0f;  // seconds before despawn (default 30 min)
    bool salvaged = false;               // true once a player has salvaged it

    COMPONENT_TYPE(Wreck)
};

/**
 * @brief Personality axes for AI fleet captains
 *
 * Eight axes capturing both behavioural style and deeper psychology.
 * The original four (aggression, sociability, optimism, professionalism)
 * describe observable behaviour; the new four (loyalty, paranoia, ambition,
 * adaptability) drive long-term decision making and relationship dynamics.
 */
class CaptainPersonality : public ecs::Component {
public:
    // Behavioural axes
    float aggression = 0.5f;        // 0=cautious, 1=bold
    float sociability = 0.5f;       // 0=quiet, 1=talkative
    float optimism = 0.5f;          // 0=grim, 1=hopeful
    float professionalism = 0.5f;   // 0=casual, 1=formal

    // Psychological axes (Phase 1)
    float loyalty = 0.5f;           // 0=self-serving, 1=devoted to fleet
    float paranoia = 0.5f;          // 0=trusting, 1=suspicious/cautious
    float ambition = 0.5f;          // 0=content, 1=driven/power-seeking
    float adaptability = 0.5f;      // 0=rigid, 1=flexible

    std::string captain_name;
    std::string faction;            // Solari, Veyren, Aurelian, Keldari

    COMPONENT_TYPE(CaptainPersonality)
};

/**
 * @brief Tracks fleet morale for an entity
 */
class FleetMorale : public ecs::Component {
public:
    float morale_score = 0.0f;              // clamped -100 to +100
    int wins = 0;
    int losses = 0;
    int ships_lost = 0;
    int times_saved_by_player = 0;
    int times_player_saved = 0;
    int missions_together = 0;
    std::string morale_state = "Steady";    // Inspired/Steady/Doubtful/Disengaged

    void updateMoraleScore() {
        morale_score = static_cast<float>(wins) * 1.0f
                     - static_cast<float>(losses) * 1.5f
                     - static_cast<float>(ships_lost) * 2.0f
                     + static_cast<float>(times_saved_by_player) * 1.2f;
        morale_score = std::clamp(morale_score, -100.0f, 100.0f);
        if (morale_score >= 50.0f) {
            morale_state = "Inspired";
        } else if (morale_score >= 0.0f) {
            morale_state = "Steady";
        } else if (morale_score >= -50.0f) {
            morale_state = "Doubtful";
        } else {
            morale_state = "Disengaged";
        }
    }

    COMPONENT_TYPE(FleetMorale)
};

/**
 * @brief Social graph for fleet captains
 */
class CaptainRelationship : public ecs::Component {
public:
    struct Relationship {
        std::string other_captain_id;
        float affinity = 0.0f;  // -100 to +100
    };

    std::vector<Relationship> relationships;

    float getAffinityWith(const std::string& id) const {
        for (const auto& r : relationships) {
            if (r.other_captain_id == id) {
                return r.affinity;
            }
        }
        return 0.0f;
    }

    void modifyAffinity(const std::string& id, float change) {
        for (auto& r : relationships) {
            if (r.other_captain_id == id) {
                r.affinity = std::clamp(r.affinity + change, -100.0f, 100.0f);
                return;
            }
        }
        Relationship rel;
        rel.other_captain_id = id;
        rel.affinity = std::clamp(change, -100.0f, 100.0f);
        relationships.push_back(rel);
    }

    COMPONENT_TYPE(CaptainRelationship)
};

/**
 * @brief Long-term emotional arcs
 */
class EmotionalState : public ecs::Component {
public:
    float confidence = 50.0f;       // 0-100
    float trust_in_player = 50.0f;  // 0-100
    float fatigue = 0.0f;           // 0-100
    float hope = 50.0f;             // 0-100

    COMPONENT_TYPE(EmotionalState)
};

/**
 * @brief Warp phase tracking (for warp anomaly system)
 */
class WarpState : public ecs::Component {
public:
    enum class WarpPhase { None, Align, Entry, Cruise, Event, Exit };

    WarpPhase phase = WarpPhase::None;
    float warp_time = 0.0f;
    float distance_remaining = 0.0f;
    float warp_speed = 3.0f;    // AU/s (initialized from Ship component)
    float mass_norm = 0.0f;     // 0=frigate, 1=capital
    float intensity = 0.0f;     // computed from time + mass
    int warp_disrupt_strength = 0;  // total disruption applied to this entity

    COMPONENT_TYPE(WarpState)
};

/**
 * @brief Chatter state for an entity
 */
class FleetChatterState : public ecs::Component {
public:
    float chatter_cooldown = 0.0f;
    bool is_speaking = false;
    float priority = 0.0f;
    std::string current_activity = "Idle";  // Warp/Mining/Combat/Travel/Idle
    std::string last_line_spoken;
    int lines_spoken_total = 0;

    COMPONENT_TYPE(FleetChatterState)
};

/**
 * @brief Rumors heard/witnessed by a captain
 */
class RumorLog : public ecs::Component {
public:
    struct Rumor {
        std::string rumor_id;
        std::string text;
        float belief_strength = 0.5f;
        bool personally_witnessed = false;
        int times_heard = 0;
    };

    std::vector<Rumor> rumors;

    bool hasRumor(const std::string& id) const {
        for (const auto& r : rumors) {
            if (r.rumor_id == id) {
                return true;
            }
        }
        return false;
    }

    void addRumor(const std::string& id, const std::string& text, bool witnessed) {
        for (auto& r : rumors) {
            if (r.rumor_id == id) {
                r.times_heard++;
                return;
            }
        }
        Rumor rumor;
        rumor.rumor_id = id;
        rumor.text = text;
        rumor.belief_strength = 0.5f;
        rumor.personally_witnessed = witnessed;
        rumor.times_heard = 1;
        rumors.push_back(rumor);
    }

    COMPONENT_TYPE(RumorLog)
};

/**
 * @brief Aggregated fleet cargo
 */
class FleetCargoPool : public ecs::Component {
public:
    uint64_t total_capacity = 0;
    uint64_t used_capacity = 0;
    std::map<std::string, uint64_t> pooled_items;   // item_type -> quantity
    std::vector<std::string> contributor_ship_ids;

    COMPONENT_TYPE(FleetCargoPool)
};

/**
 * @brief Tactical overlay state
 */
class TacticalOverlayState : public ecs::Component {
public:
    bool enabled = false;
    std::vector<float> ring_distances = {5.0f, 10.0f, 20.0f, 30.0f, 50.0f, 100.0f};
    float tool_range = 0.0f;
    std::string tool_type;

    COMPONENT_TYPE(TacticalOverlayState)
};

/**
 * @brief Damage event tracking for visual feedback
 * 
 * Records recent damage hits so the client can render appropriate
 * visual effects: shield ripple (blue), armor flash (yellow/orange),
 * hull pulse (red + screen shake).
 */
class DamageEvent : public ecs::Component {
public:
    struct HitRecord {
        float damage_amount = 0.0f;
        std::string damage_type;      // em, thermal, kinetic, explosive
        std::string layer_hit;        // shield, armor, hull
        float timestamp = 0.0f;
        bool shield_depleted = false;  // shield reached 0 on this hit
        bool armor_depleted = false;   // armor reached 0 on this hit
        bool hull_critical = false;    // hull below 25% after this hit
    };

    std::vector<HitRecord> recent_hits;
    float last_hit_time = 0.0f;
    float total_damage_taken = 0.0f;

    void addHit(float damage, const std::string& type, const std::string& layer,
                float time, bool shield_dep = false, bool armor_dep = false,
                bool hull_crit = false) {
        HitRecord hit;
        hit.damage_amount = damage;
        hit.damage_type = type;
        hit.layer_hit = layer;
        hit.timestamp = time;
        hit.shield_depleted = shield_dep;
        hit.armor_depleted = armor_dep;
        hit.hull_critical = hull_crit;
        recent_hits.push_back(hit);
        last_hit_time = time;
        total_damage_taken += damage;
    }

    void clearOldHits(float current_time, float max_age = 5.0f) {
        recent_hits.erase(
            std::remove_if(recent_hits.begin(), recent_hits.end(),
                [current_time, max_age](const HitRecord& h) {
                    return (current_time - h.timestamp) > max_age;
                }),
            recent_hits.end());
    }

    COMPONENT_TYPE(DamageEvent)
};

/**
 * @brief Mineral deposit — an asteroid or ore site containing minable resources
 *
 * Attached to asteroid belt entities.  Each deposit has a mineral type,
 * a remaining quantity (units), and a yield rate that controls how much
 * ore is extracted per mining cycle.
 */
class MineralDeposit : public ecs::Component {
public:
    std::string mineral_type = "Veldspar";   // ore name
    float quantity_remaining = 10000.0f;     // units of ore left
    float max_quantity = 10000.0f;           // original total
    float yield_rate = 1.0f;                 // multiplier on mining yield
    float volume_per_unit = 0.1f;            // m3 per unit of ore

    bool isDepleted() const { return quantity_remaining <= 0.0f; }

    COMPONENT_TYPE(MineralDeposit)
};

/**
 * @brief Mining laser module — attached to ships that can mine
 *
 * Tracks the mining cycle timer and yield per cycle.  When the cycle
 * completes the MiningSystem transfers ore from the targeted deposit
 * into the ship's Inventory.
 */
class MiningLaser : public ecs::Component {
public:
    float yield_per_cycle = 100.0f;          // base units mined per cycle
    float cycle_time = 60.0f;                // seconds per mining cycle
    float cycle_progress = 0.0f;             // seconds elapsed in current cycle
    bool active = false;                     // currently mining?
    std::string target_deposit_id;           // entity id of the deposit being mined

    COMPONENT_TYPE(MiningLaser)
};

/**
 * @brief Per–solar-system resource tracking
 *
 * Attached to the solar system entity to record total and remaining
 * resources so the server can balance spawn rates and depletion.
 */
class SystemResources : public ecs::Component {
public:
    struct ResourceEntry {
        std::string mineral_type;
        float total_quantity = 0.0f;
        float remaining_quantity = 0.0f;
    };

    std::vector<ResourceEntry> resources;

    float totalRemaining() const {
        float sum = 0.0f;
        for (const auto& r : resources) sum += r.remaining_quantity;
        return sum;
    }

    COMPONENT_TYPE(SystemResources)
};

/**
 * @brief Fleet formation assignment for an entity
 *
 * Stores the formation type the fleet is using and this member's
 * computed offset relative to the fleet commander.
 */
class FleetFormation : public ecs::Component {
public:
    enum class FormationType { None, Arrow, Line, Wedge, Spread, Diamond };

    FormationType formation = FormationType::Arrow;
    int slot_index = 0;           // position within the formation (0 = leader)
    float offset_x = 0.0f;       // metres relative to commander
    float offset_y = 0.0f;
    float offset_z = 0.0f;

    COMPONENT_TYPE(FleetFormation)
};

/**
 * @brief Persistent memory for an AI fleet captain
 *
 * Records significant events so the captain can reference them in chatter
 * and factor them into personality-driven decisions.
 */
class CaptainMemory : public ecs::Component {
public:
    struct MemoryEntry {
        std::string event_type;     // "combat_win", "combat_loss", "ship_lost", "saved_by_player", "warp_anomaly"
        std::string context;        // free-form detail (e.g. enemy name)
        float timestamp = 0.0f;     // in-game seconds since session start
        float emotional_weight = 0.0f; // -1=traumatic, +1=uplifting
    };

    std::vector<MemoryEntry> memories;
    int max_memories = 50;          // cap to prevent unbounded growth

    void addMemory(const std::string& event, const std::string& ctx,
                   float time, float weight) {
        if (static_cast<int>(memories.size()) >= max_memories) {
            memories.erase(memories.begin()); // drop oldest
        }
        memories.push_back({event, ctx, time, weight});
    }

    int countByType(const std::string& event_type) const {
        int n = 0;
        for (const auto& m : memories) {
            if (m.event_type == event_type) ++n;
        }
        return n;
    }

    float averageWeight() const {
        if (memories.empty()) return 0.0f;
        float sum = 0.0f;
        for (const auto& m : memories) sum += m.emotional_weight;
        return sum / static_cast<float>(memories.size());
    }

    COMPONENT_TYPE(CaptainMemory)
};

/**
 * @brief Warp tunnel visual layer configuration (cinematic warp system)
 *
 * Stores per-entity shader layer intensities computed by WarpCinematicSystem.
 * Client reads these to drive the multi-layer warp tunnel overlay.
 */
class WarpTunnelConfig : public ecs::Component {
public:
    // Shader layer intensities (0.0–1.0)
    float radial_distortion = 0.0f;    // Radial distortion layer
    float starfield_bloom   = 0.0f;    // Starfield velocity bloom
    float tunnel_skin       = 0.0f;    // Tunnel skin/noise layer
    float vignette          = 0.0f;    // Edge vignette darkening

    // Composite intensity derived from ship mass + phase
    float composite_intensity = 0.0f;

    COMPONENT_TYPE(WarpTunnelConfig)
};

/**
 * @brief Warp audio profile for adaptive warp sounds
 *
 * Drives three audio channels during warp: engine core (sub-bass),
 * warp field harmonics, and environmental shimmer.
 */
class WarpAudioProfile : public ecs::Component {
public:
    float engine_core_volume  = 0.0f;   // Sub-bass engine drone (0.0–1.0)
    float harmonics_volume    = 0.0f;   // Warp field harmonics (0.0–1.0)
    float shimmer_volume      = 0.0f;   // Environmental shimmer (0.0–1.0)
    float engine_core_pitch   = 1.0f;   // Pitch multiplier for engine core
    float harmonics_pitch     = 1.0f;   // Pitch multiplier for harmonics

    COMPONENT_TYPE(WarpAudioProfile)
};

/**
 * @brief Accessibility settings for warp visual/audio effects
 *
 * Allows players to reduce motion, bass, and blur intensity
 * to accommodate different sensitivities.
 */
class WarpAccessibility : public ecs::Component {
public:
    float motion_intensity = 1.0f;   // Motion effect scale (0.0–1.0, 0=off)
    float bass_intensity   = 1.0f;   // Sub-bass volume scale (0.0–1.0)
    float blur_intensity   = 1.0f;   // Blur/distortion scale (0.0–1.0)

    COMPONENT_TYPE(WarpAccessibility)
};

/**
 * @brief Refining facility — converts raw ore into refined minerals
 *
 * Attached to station entities that offer refining services.
 * Efficiency determines the yield: at 1.0 (100%) all ore converts
 * to minerals; at 0.5 only half does.
 *
 * Each ore type maps to one or more mineral outputs via refine_ratios.
 */
class RefiningFacility : public ecs::Component {
public:
    struct RefineRecipe {
        std::string ore_type;           // input ore name (e.g. "Veldspar")
        int ore_units_required = 100;   // units consumed per batch
        struct MineralOutput {
            std::string mineral_type;   // output mineral name (e.g. "Tritanium")
            int base_quantity = 333;    // base output per batch at 100% efficiency
        };
        std::vector<MineralOutput> outputs;
    };

    float efficiency = 0.5f;            // 0.0–1.0, refining yield multiplier
    float tax_rate = 0.05f;             // fraction taken as tax
    std::vector<RefineRecipe> recipes;

    COMPONENT_TYPE(RefiningFacility)
};

/**
 * @brief An in-space anomaly (combat site, mining site, data site, etc.)
 *
 * Generated procedurally from a solar system seed.  Players discover
 * anomalies via the ScannerSystem and warp to them for content.
 */
class Anomaly : public ecs::Component {
public:
    enum class Type { Combat, Mining, Data, Relic, Gas, Wormhole };
    enum class Difficulty { Trivial, Easy, Medium, Hard, Deadly };

    std::string anomaly_id;
    std::string anomaly_name;
    std::string system_id;                  // owning solar system entity
    Type type = Type::Combat;
    Difficulty difficulty = Difficulty::Medium;
    float signature_strength = 0.5f;        // 0.0–1.0, affects scan difficulty
    float x = 0.0f;                         // position in system
    float y = 0.0f;
    float z = 0.0f;
    bool discovered = false;                // has anyone scanned this down?
    bool completed = false;                 // has content been cleared?
    float despawn_timer = 3600.0f;          // seconds until natural despawn
    int npc_count = 0;                      // NPCs to spawn on warp-in
    float loot_multiplier = 1.0f;           // scales drop quality

    COMPONENT_TYPE(Anomaly)
};

/**
 * @brief Probe scanner — attached to ships that can scan for anomalies
 *
 * Players deploy probes to discover hidden anomalies in a solar system.
 * Scan strength and deviation improve with skill and probe count.
 */
class Scanner : public ecs::Component {
public:
    float scan_strength = 50.0f;         // base scan strength (affected by skills/modules)
    float scan_deviation = 4.0f;         // positional error in AU (decreases with better scans)
    int probe_count = 8;                 // number of probes deployed
    float scan_duration = 10.0f;         // seconds per scan cycle
    float scan_progress = 0.0f;          // current scan cycle progress
    bool scanning = false;               // currently scanning?
    std::string target_system_id;        // system being scanned
    
    struct ScanResult {
        std::string anomaly_id;
        float signal_strength = 0.0f;    // 0.0–1.0 (1.0 = fully scanned)
        float deviation = 0.0f;          // positional error remaining
        bool warpable = false;           // true when signal >= 1.0
    };
    
    std::vector<ScanResult> results;

    COMPONENT_TYPE(Scanner)
};

/**
 * @brief Per-system difficulty modifier based on security status
 *
 * Attached to solar system entities.  Scales NPC stats, spawn rates,
 * and loot quality based on the zone's security level.
 */
class DifficultyZone : public ecs::Component {
public:
    float security_status = 0.5f;        // 1.0 highsec → 0.0 nullsec
    float npc_hp_multiplier = 1.0f;      // applied to NPC health pools
    float npc_damage_multiplier = 1.0f;  // applied to NPC weapon damage
    float spawn_rate_multiplier = 1.0f;  // controls how often NPCs respawn
    float loot_quality_multiplier = 1.0f; // scales loot drop quality
    float ore_richness_multiplier = 1.0f; // scales mining yield
    int max_npc_tier = 1;                // highest NPC tier that can spawn (1-5)

    COMPONENT_TYPE(DifficultyZone)
};

/**
 * @brief A template for procedurally generating missions
 *
 * Stored as entities in a template library.  MissionTemplateSystem uses
 * these to produce concrete ActiveMission instances with deterministic
 * objective counts and scaled rewards.
 */
class MissionTemplate : public ecs::Component {
public:
    struct ObjectiveTemplate {
        std::string type;          // "destroy", "mine", "deliver", "reach"
        std::string target;        // target type (e.g., "pirate_frigate", "Veldspar", "Trade Goods")
        int count_min = 1;
        int count_max = 5;
    };

    std::string template_id;
    std::string name_pattern;      // e.g., "Pirate Clearance: {system}"
    std::string type;              // "combat", "mining", "courier", "trade", "exploration"
    int level = 1;                 // 1-5
    std::string required_faction;  // faction that offers this mission ("" = any)
    float min_standing = 0.0f;     // minimum faction standing required

    std::vector<ObjectiveTemplate> objective_templates;

    // Reward scaling
    double base_isk = 100000.0;
    double isk_per_level = 50000.0;
    float base_standing_reward = 0.1f;
    float standing_per_level = 0.05f;
    float base_time_limit = 3600.0f; // seconds, -1 = no limit

    COMPONENT_TYPE(MissionTemplate)
};

} // namespace components
} // namespace atlas

#endif // EVE_COMPONENTS_GAME_COMPONENTS_H
