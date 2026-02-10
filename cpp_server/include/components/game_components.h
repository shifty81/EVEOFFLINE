#ifndef EVE_COMPONENTS_GAME_COMPONENTS_H
#define EVE_COMPONENTS_GAME_COMPONENTS_H

#include "ecs/component.h"
#include <string>
#include <vector>
#include <map>

namespace eve {
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
        Attacking
    };
    
    Behavior behavior = Behavior::Aggressive;
    State state = State::Idle;
    std::string target_entity_id;
    float orbit_distance = 1000.0f;  // preferred orbit distance
    float awareness_range = 50000.0f;  // meters
    
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
        new_standing = std::max(-10.0f, std::min(10.0f, new_standing));
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
        std::string type;          // "light_combat_drone", "medium_combat_drone", etc.
        std::string damage_type;   // "em", "thermal", "kinetic", "explosive"
        float damage = 0.0f;
        float rate_of_fire = 3.0f; // seconds between shots
        float cooldown = 0.0f;     // current cooldown timer
        float optimal_range = 5000.0f;
        float hitpoints = 45.0f;
        float current_hp = 45.0f;
        int bandwidth_use = 5;
        float volume = 5.0f;       // m3 per drone
    };

    std::vector<DroneInfo> stored_drones;    // drones in bay (not deployed)
    std::vector<DroneInfo> deployed_drones;  // drones in space

    float bay_capacity = 25.0f;     // m3 total bay capacity
    int max_bandwidth = 25;         // Mbit/s bandwidth limit

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

} // namespace components
} // namespace eve

#endif // EVE_COMPONENTS_GAME_COMPONENTS_H
