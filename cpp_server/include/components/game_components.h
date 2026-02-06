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
    std::string ship_name = "Rifter";
    std::string race = "Minmatar";
    
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
    std::string faction_name = "Neutral";  // Caldari, Gallente, Amarr, Minmatar, Serpentis, etc.
    std::map<std::string, float> standings;  // faction_name: standing (-10 to +10)
    
    COMPONENT_TYPE(Faction)
};

} // namespace components
} // namespace eve

#endif // EVE_COMPONENTS_GAME_COMPONENTS_H
