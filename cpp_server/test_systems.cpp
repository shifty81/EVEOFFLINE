/**
 * Test all ECS systems for the C++ server
 * 
 * Tests dedicated ECS systems including Capacitor, Shield, Weapon,
 * Targeting, Wormhole, Fleet, Mission, Skill, Module, Inventory,
 * Loot, Drone, Insurance, Bounty, Market, and more.
 */

#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include "systems/capacitor_system.h"
#include "systems/shield_recharge_system.h"
#include "systems/weapon_system.h"
#include "systems/targeting_system.h"
#include "data/ship_database.h"
#include "data/wormhole_database.h"
#include "systems/wormhole_system.h"
#include "systems/fleet_system.h"
#include "systems/mission_system.h"
#include "systems/skill_system.h"
#include "systems/module_system.h"
#include "systems/inventory_system.h"
#include "systems/loot_system.h"
#include "systems/drone_system.h"
#include "systems/insurance_system.h"
#include "systems/corporation_system.h"
#include "systems/bounty_system.h"
#include "systems/market_system.h"
#include "systems/contract_system.h"
#include "systems/pi_system.h"
#include "systems/manufacturing_system.h"
#include "systems/research_system.h"
#include "systems/chat_system.h"
#include "systems/character_creation_system.h"
#include "systems/tournament_system.h"
#include "systems/leaderboard_system.h"
#include "data/world_persistence.h"
#include "data/npc_database.h"
#include "systems/movement_system.h"
#include "systems/station_system.h"
#include "systems/wreck_salvage_system.h"
#include "systems/fleet_morale_system.h"
#include "systems/captain_personality_system.h"
#include "systems/fleet_chatter_system.h"
#include "systems/warp_anomaly_system.h"
#include "systems/captain_relationship_system.h"
#include "systems/emotional_arc_system.h"
#include "systems/fleet_cargo_system.h"
#include "systems/tactical_overlay_system.h"
#include "systems/combat_system.h"
#include "systems/ai_system.h"
#include "systems/mining_system.h"
#include "systems/fleet_formation_system.h"
#include "systems/captain_memory_system.h"
#include "systems/ship_fitting_system.h"
#include "network/protocol_handler.h"
#include "ui/server_console.h"
#include "utils/logger.h"
#include "utils/server_metrics.h"
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>
#include <memory>
#include <fstream>
#include <thread>

using namespace atlas;

// Test counters
int testsRun = 0;
int testsPassed = 0;

void assertTrue(bool condition, const std::string& testName) {
    testsRun++;
    if (condition) {
        testsPassed++;
        std::cout << "  \xe2\x9c\x93 " << testName << std::endl;
    } else {
        std::cout << "  \xe2\x9c\x97 " << testName << " FAILED" << std::endl;
    }
}

bool approxEqual(float a, float b, float epsilon = 0.01f) {
    return std::fabs(a - b) < epsilon;
}

// Helper to add a component and return a raw pointer to it
template<typename T>
T* addComp(ecs::Entity* e) {
    auto c = std::make_unique<T>();
    T* ptr = c.get();
    e->addComponent(std::move(c));
    return ptr;
}

// ==================== CapacitorSystem Tests ====================

void testCapacitorRecharge() {
    std::cout << "\n=== Capacitor Recharge ===" << std::endl;
    
    ecs::World world;
    systems::CapacitorSystem capSys(&world);
    
    auto* entity = world.createEntity("test_ship");
    auto* cap = addComp<components::Capacitor>(entity);
    cap->capacitor = 50.0f;
    cap->capacitor_max = 100.0f;
    cap->recharge_rate = 10.0f;
    
    capSys.update(1.0f);
    assertTrue(approxEqual(cap->capacitor, 60.0f), "Capacitor recharges by rate * delta_time");
    
    capSys.update(5.0f);
    assertTrue(approxEqual(cap->capacitor, 100.0f), "Capacitor does not exceed max");
    
    capSys.update(1.0f);
    assertTrue(approxEqual(cap->capacitor, 100.0f), "Full capacitor stays at max");
}

void testCapacitorConsume() {
    std::cout << "\n=== Capacitor Consumption ===" << std::endl;
    
    ecs::World world;
    systems::CapacitorSystem capSys(&world);
    
    auto* entity = world.createEntity("test_ship");
    auto* cap = addComp<components::Capacitor>(entity);
    cap->capacitor = 50.0f;
    cap->capacitor_max = 100.0f;
    
    bool result = capSys.consumeCapacitor("test_ship", 30.0f);
    assertTrue(result, "Consume succeeds when enough capacitor");
    assertTrue(approxEqual(cap->capacitor, 20.0f), "Capacitor reduced by consumed amount");
    
    result = capSys.consumeCapacitor("test_ship", 25.0f);
    assertTrue(!result, "Consume fails when not enough capacitor");
    assertTrue(approxEqual(cap->capacitor, 20.0f), "Capacitor unchanged on failed consume");
    
    result = capSys.consumeCapacitor("nonexistent", 10.0f);
    assertTrue(!result, "Consume fails for nonexistent entity");
}

void testCapacitorPercentage() {
    std::cout << "\n=== Capacitor Percentage ===" << std::endl;
    
    ecs::World world;
    systems::CapacitorSystem capSys(&world);
    
    auto* entity = world.createEntity("test_ship");
    auto* cap = addComp<components::Capacitor>(entity);
    cap->capacitor = 75.0f;
    cap->capacitor_max = 100.0f;
    
    float pct = capSys.getCapacitorPercentage("test_ship");
    assertTrue(approxEqual(pct, 0.75f), "Capacitor percentage is correct (75%)");
    
    float noEntity = capSys.getCapacitorPercentage("nonexistent");
    assertTrue(noEntity < 0.0f, "Returns -1 for nonexistent entity");
}

// ==================== ShieldRechargeSystem Tests ====================

void testShieldRecharge() {
    std::cout << "\n=== Shield Recharge ===" << std::endl;
    
    ecs::World world;
    systems::ShieldRechargeSystem shieldSys(&world);
    
    auto* entity = world.createEntity("test_ship");
    auto* health = addComp<components::Health>(entity);
    health->shield_hp = 50.0f;
    health->shield_max = 100.0f;
    health->shield_recharge_rate = 5.0f;
    
    shieldSys.update(2.0f);
    assertTrue(approxEqual(health->shield_hp, 60.0f), "Shield recharges by rate * delta_time");
    
    shieldSys.update(10.0f);
    assertTrue(approxEqual(health->shield_hp, 100.0f), "Shield does not exceed max");
    
    shieldSys.update(1.0f);
    assertTrue(approxEqual(health->shield_hp, 100.0f), "Full shields stay at max");
}

void testShieldPercentage() {
    std::cout << "\n=== Shield Percentage ===" << std::endl;
    
    ecs::World world;
    systems::ShieldRechargeSystem shieldSys(&world);
    
    auto* entity = world.createEntity("test_ship");
    auto* health = addComp<components::Health>(entity);
    health->shield_hp = 40.0f;
    health->shield_max = 200.0f;
    
    float pct = shieldSys.getShieldPercentage("test_ship");
    assertTrue(approxEqual(pct, 0.2f), "Shield percentage is correct (20%)");
    
    float noEntity = shieldSys.getShieldPercentage("nonexistent");
    assertTrue(noEntity < 0.0f, "Returns -1 for nonexistent entity");
}

// ==================== WeaponSystem Tests ====================

void testWeaponCooldown() {
    std::cout << "\n=== Weapon Cooldown ===" << std::endl;
    
    ecs::World world;
    systems::WeaponSystem weaponSys(&world);
    
    auto* entity = world.createEntity("test_ship");
    auto* weapon = addComp<components::Weapon>(entity);
    weapon->cooldown = 3.0f;
    weapon->rate_of_fire = 3.0f;
    
    weaponSys.update(1.0f);
    assertTrue(approxEqual(weapon->cooldown, 2.0f), "Cooldown decreases by delta_time");
    
    weaponSys.update(3.0f);
    assertTrue(approxEqual(weapon->cooldown, 0.0f), "Cooldown floors at zero");
}

void testWeaponFireWithCapacitor() {
    std::cout << "\n=== Weapon Fire With Capacitor ===" << std::endl;
    
    ecs::World world;
    systems::WeaponSystem weaponSys(&world);
    
    auto* shooter = world.createEntity("shooter");
    auto* weapon = addComp<components::Weapon>(shooter);
    weapon->damage = 50.0f;
    weapon->damage_type = "kinetic";
    weapon->optimal_range = 10000.0f;
    weapon->falloff_range = 5000.0f;
    weapon->rate_of_fire = 3.0f;
    weapon->cooldown = 0.0f;
    weapon->capacitor_cost = 10.0f;
    weapon->ammo_count = 100;
    
    auto* shooterPos = addComp<components::Position>(shooter);
    shooterPos->x = 0.0f;
    shooterPos->y = 0.0f;
    shooterPos->z = 0.0f;
    
    auto* cap = addComp<components::Capacitor>(shooter);
    cap->capacitor = 50.0f;
    cap->capacitor_max = 100.0f;
    
    auto* target = world.createEntity("target");
    auto* targetPos = addComp<components::Position>(target);
    targetPos->x = 5000.0f;
    targetPos->y = 0.0f;
    targetPos->z = 0.0f;
    
    auto* targetHealth = addComp<components::Health>(target);
    targetHealth->shield_hp = 100.0f;
    targetHealth->shield_max = 100.0f;
    targetHealth->armor_hp = 100.0f;
    targetHealth->armor_max = 100.0f;
    targetHealth->hull_hp = 100.0f;
    targetHealth->hull_max = 100.0f;
    
    bool fired = weaponSys.fireWeapon("shooter", "target");
    assertTrue(fired, "Weapon fires successfully");
    assertTrue(approxEqual(cap->capacitor, 40.0f), "Capacitor consumed on fire");
    assertTrue(weapon->cooldown > 0.0f, "Cooldown set after firing");
    assertTrue(weapon->ammo_count == 99, "Ammo consumed");
    assertTrue(targetHealth->shield_hp < 100.0f, "Target took shield damage");
}

void testWeaponFireInsufficientCapacitor() {
    std::cout << "\n=== Weapon Fire Insufficient Capacitor ===" << std::endl;
    
    ecs::World world;
    systems::WeaponSystem weaponSys(&world);
    
    auto* shooter = world.createEntity("shooter");
    auto* weapon = addComp<components::Weapon>(shooter);
    weapon->damage = 50.0f;
    weapon->capacitor_cost = 20.0f;
    weapon->cooldown = 0.0f;
    weapon->ammo_count = 100;
    weapon->optimal_range = 10000.0f;
    weapon->falloff_range = 5000.0f;
    weapon->damage_type = "kinetic";
    
    addComp<components::Position>(shooter);
    auto* cap = addComp<components::Capacitor>(shooter);
    cap->capacitor = 5.0f;
    cap->capacitor_max = 100.0f;
    
    auto* target = world.createEntity("target");
    addComp<components::Position>(target);
    addComp<components::Health>(target);
    
    bool fired = weaponSys.fireWeapon("shooter", "target");
    assertTrue(!fired, "Weapon fails to fire with insufficient capacitor");
    assertTrue(approxEqual(cap->capacitor, 5.0f), "Capacitor not consumed on failure");
}

void testWeaponFireOutOfRange() {
    std::cout << "\n=== Weapon Fire Out of Range ===" << std::endl;
    
    ecs::World world;
    systems::WeaponSystem weaponSys(&world);
    
    auto* shooter = world.createEntity("shooter");
    auto* weapon = addComp<components::Weapon>(shooter);
    weapon->optimal_range = 5000.0f;
    weapon->falloff_range = 2500.0f;
    weapon->cooldown = 0.0f;
    weapon->capacitor_cost = 0.0f;
    weapon->ammo_count = 100;
    weapon->damage_type = "kinetic";
    
    auto* shooterPos = addComp<components::Position>(shooter);
    shooterPos->x = 0.0f;
    
    auto* target = world.createEntity("target");
    auto* targetPos = addComp<components::Position>(target);
    targetPos->x = 10000.0f;
    
    addComp<components::Health>(target);
    
    bool fired = weaponSys.fireWeapon("shooter", "target");
    assertTrue(!fired, "Weapon fails to fire when target is out of range");
}

void testWeaponDamageFalloff() {
    std::cout << "\n=== Weapon Damage Falloff ===" << std::endl;
    
    ecs::World world;
    systems::WeaponSystem weaponSys(&world);
    
    auto* shooter = world.createEntity("shooter");
    auto* weapon = addComp<components::Weapon>(shooter);
    weapon->damage = 100.0f;
    weapon->damage_type = "em";
    weapon->optimal_range = 5000.0f;
    weapon->falloff_range = 5000.0f;
    weapon->rate_of_fire = 0.1f;
    weapon->cooldown = 0.0f;
    weapon->capacitor_cost = 0.0f;
    weapon->ammo_count = 100;
    
    auto* shooterPos = addComp<components::Position>(shooter);
    shooterPos->x = 0.0f;
    
    // Target at optimal range - full damage
    auto* target1 = world.createEntity("target1");
    auto* t1Pos = addComp<components::Position>(target1);
    t1Pos->x = 5000.0f;
    auto* t1Health = addComp<components::Health>(target1);
    t1Health->shield_hp = 200.0f;
    t1Health->shield_max = 200.0f;
    
    weaponSys.fireWeapon("shooter", "target1");
    float damageAtOptimal = 200.0f - t1Health->shield_hp;
    assertTrue(approxEqual(damageAtOptimal, 100.0f), "Full damage at optimal range");
    
    weapon->cooldown = 0.0f;
    
    // Target at 50% falloff
    auto* target2 = world.createEntity("target2");
    auto* t2Pos = addComp<components::Position>(target2);
    t2Pos->x = 7500.0f;
    auto* t2Health = addComp<components::Health>(target2);
    t2Health->shield_hp = 200.0f;
    t2Health->shield_max = 200.0f;
    
    weaponSys.fireWeapon("shooter", "target2");
    float damageAtHalfFalloff = 200.0f - t2Health->shield_hp;
    assertTrue(approxEqual(damageAtHalfFalloff, 50.0f), "50% damage at 50% falloff");
}

void testWeaponDamageResistances() {
    std::cout << "\n=== Weapon Damage Resistances ===" << std::endl;
    
    ecs::World world;
    systems::WeaponSystem weaponSys(&world);
    
    auto* shooter = world.createEntity("shooter");
    auto* weapon = addComp<components::Weapon>(shooter);
    weapon->damage = 100.0f;
    weapon->damage_type = "thermal";
    weapon->optimal_range = 10000.0f;
    weapon->falloff_range = 5000.0f;
    weapon->cooldown = 0.0f;
    weapon->capacitor_cost = 0.0f;
    weapon->ammo_count = 100;
    weapon->rate_of_fire = 0.1f;
    
    addComp<components::Position>(shooter);
    
    auto* target = world.createEntity("target");
    addComp<components::Position>(target);
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 500.0f;
    health->shield_max = 500.0f;
    health->shield_thermal_resist = 0.5f;
    
    weaponSys.fireWeapon("shooter", "target");
    float damageTaken = 500.0f - health->shield_hp;
    assertTrue(approxEqual(damageTaken, 50.0f), "50% thermal resist reduces 100 damage to 50");
}

void testWeaponAutoFireAI() {
    std::cout << "\n=== Weapon Auto-fire for AI ===" << std::endl;
    
    ecs::World world;
    systems::WeaponSystem weaponSys(&world);
    
    auto* npc = world.createEntity("npc");
    auto* weapon = addComp<components::Weapon>(npc);
    weapon->damage = 20.0f;
    weapon->damage_type = "kinetic";
    weapon->optimal_range = 10000.0f;
    weapon->falloff_range = 5000.0f;
    weapon->rate_of_fire = 2.0f;
    weapon->cooldown = 0.0f;
    weapon->capacitor_cost = 0.0f;
    weapon->ammo_count = 100;
    
    addComp<components::Position>(npc);
    
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Attacking;
    ai->target_entity_id = "player";
    
    auto* player = world.createEntity("player");
    addComp<components::Position>(player);
    auto* playerHealth = addComp<components::Health>(player);
    playerHealth->shield_hp = 100.0f;
    playerHealth->shield_max = 100.0f;
    
    weaponSys.update(0.033f);
    assertTrue(playerHealth->shield_hp < 100.0f, "AI auto-fires at target during Attacking state");
    assertTrue(weapon->cooldown > 0.0f, "Weapon cooldown set after auto-fire");
}

void testWeaponNoAutoFireIdleAI() {
    std::cout << "\n=== No Auto-fire for Idle AI ===" << std::endl;
    
    ecs::World world;
    systems::WeaponSystem weaponSys(&world);
    
    auto* npc = world.createEntity("npc");
    auto* weapon = addComp<components::Weapon>(npc);
    weapon->damage = 20.0f;
    weapon->cooldown = 0.0f;
    weapon->optimal_range = 10000.0f;
    weapon->falloff_range = 5000.0f;
    weapon->capacitor_cost = 0.0f;
    weapon->ammo_count = 100;
    weapon->damage_type = "kinetic";
    
    addComp<components::Position>(npc);
    
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Idle;
    ai->target_entity_id = "player";
    
    auto* player = world.createEntity("player");
    addComp<components::Position>(player);
    auto* playerHealth = addComp<components::Health>(player);
    playerHealth->shield_hp = 100.0f;
    playerHealth->shield_max = 100.0f;
    
    weaponSys.update(0.033f);
    assertTrue(approxEqual(playerHealth->shield_hp, 100.0f), "Idle AI does not auto-fire");
}

// ==================== TargetingSystem Tests ====================

void testTargetLockUnlock() {
    std::cout << "\n=== Target Lock/Unlock ===" << std::endl;
    
    ecs::World world;
    systems::TargetingSystem targetSys(&world);
    
    auto* ship1 = world.createEntity("ship1");
    auto* target_comp = addComp<components::Target>(ship1);
    auto* shipComp = addComp<components::Ship>(ship1);
    shipComp->scan_resolution = 500.0f;
    shipComp->max_locked_targets = 3;
    shipComp->max_targeting_range = 50000.0f;
    addComp<components::Position>(ship1);
    
    auto* npc = world.createEntity("npc1");
    addComp<components::Position>(npc);
    
    bool result = targetSys.startLock("ship1", "npc1");
    assertTrue(result, "Start lock succeeds");
    assertTrue(!targetSys.isTargetLocked("ship1", "npc1"), "Not yet locked (in progress)");
    
    // Simulate enough time for the lock to complete
    // lock_time = 1000 / 500 = 2 seconds
    targetSys.update(3.0f);
    assertTrue(targetSys.isTargetLocked("ship1", "npc1"), "Target locked after sufficient time");
    
    // Unlock
    targetSys.unlockTarget("ship1", "npc1");
    assertTrue(!targetSys.isTargetLocked("ship1", "npc1"), "Target unlocked");
}

void testTargetLockMaxTargets() {
    std::cout << "\n=== Target Lock Max Targets ===" << std::endl;
    
    ecs::World world;
    systems::TargetingSystem targetSys(&world);
    
    auto* ship1 = world.createEntity("ship1");
    addComp<components::Target>(ship1);
    auto* shipComp = addComp<components::Ship>(ship1);
    shipComp->scan_resolution = 1000.0f;  // fast lock
    shipComp->max_locked_targets = 2;
    addComp<components::Position>(ship1);
    
    world.createEntity("t1");
    addComp<components::Position>(world.getEntity("t1"));
    world.createEntity("t2");
    addComp<components::Position>(world.getEntity("t2"));
    world.createEntity("t3");
    addComp<components::Position>(world.getEntity("t3"));
    
    assertTrue(targetSys.startLock("ship1", "t1"), "Lock t1 succeeds");
    assertTrue(targetSys.startLock("ship1", "t2"), "Lock t2 succeeds");
    bool result = targetSys.startLock("ship1", "t3");
    assertTrue(!result, "Lock t3 fails (max 2 targets)");
}

void testTargetLockNonexistent() {
    std::cout << "\n=== Target Lock Nonexistent ===" << std::endl;
    
    ecs::World world;
    systems::TargetingSystem targetSys(&world);
    
    auto* ship1 = world.createEntity("ship1");
    addComp<components::Target>(ship1);
    addComp<components::Ship>(ship1);
    addComp<components::Position>(ship1);
    
    bool result = targetSys.startLock("ship1", "ghost");
    assertTrue(!result, "Lock nonexistent target fails");
    
    result = targetSys.startLock("ghost", "ship1");
    assertTrue(!result, "Lock from nonexistent entity fails");
}

// ==================== ShipDatabase Tests ====================

void testShipDatabaseLoadFromDirectory() {
    std::cout << "\n=== ShipDatabase Load From Directory ===" << std::endl;
    
    data::ShipDatabase db;
    int count = db.loadFromDirectory("../data");
    
    // If data/ isn't at ../data (depends on CWD), try other paths
    if (count == 0) {
        count = db.loadFromDirectory("data");
    }
    if (count == 0) {
        count = db.loadFromDirectory("../../data");
    }
    
    assertTrue(count > 0, "Loaded at least 1 ship from data directory");
    assertTrue(db.getShipCount() > 0, "Ship count > 0");
}

void testShipDatabaseGetShip() {
    std::cout << "\n=== ShipDatabase Get Ship ===" << std::endl;
    
    data::ShipDatabase db;
    // Try multiple paths
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    const data::ShipTemplate* fang = db.getShip("fang");
    if (fang) {
        assertTrue(fang->name == "Fang", "Fang name correct");
        assertTrue(fang->ship_class == "Frigate", "Fang class is Frigate");
        assertTrue(fang->race == "Keldari", "Fang race is Keldari");
        assertTrue(fang->shield_hp > 0.0f, "Fang has shield HP");
        assertTrue(fang->armor_hp > 0.0f, "Fang has armor HP");
        assertTrue(fang->hull_hp > 0.0f, "Fang has hull HP");
        assertTrue(fang->cpu > 0.0f, "Fang has CPU");
        assertTrue(fang->powergrid > 0.0f, "Fang has powergrid");
        assertTrue(fang->max_velocity > 0.0f, "Fang has velocity");
        assertTrue(fang->scan_resolution > 0.0f, "Fang has scan resolution");
        assertTrue(fang->max_locked_targets > 0, "Fang has max locked targets");
    } else {
        assertTrue(false, "Fang template found in database");
    }
    
    const data::ShipTemplate* missing = db.getShip("nonexistent_ship");
    assertTrue(missing == nullptr, "Nonexistent ship returns nullptr");
}

void testShipDatabaseResistances() {
    std::cout << "\n=== ShipDatabase Resistances ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    const data::ShipTemplate* fang = db.getShip("fang");
    if (fang) {
        // Fang shield: em=0, thermal=20, kinetic=40, explosive=50 (in JSON)
        // Converted to fractions: 0.0, 0.20, 0.40, 0.50
        assertTrue(approxEqual(fang->shield_resists.em, 0.0f), "Shield EM resist = 0%");
        assertTrue(approxEqual(fang->shield_resists.thermal, 0.20f), "Shield thermal resist = 20%");
        assertTrue(approxEqual(fang->shield_resists.kinetic, 0.40f), "Shield kinetic resist = 40%");
        assertTrue(approxEqual(fang->shield_resists.explosive, 0.50f), "Shield explosive resist = 50%");
        
        // Armor: em=60, thermal=35, kinetic=25, explosive=10
        assertTrue(approxEqual(fang->armor_resists.em, 0.60f), "Armor EM resist = 60%");
        assertTrue(approxEqual(fang->armor_resists.thermal, 0.35f), "Armor thermal resist = 35%");
    } else {
        assertTrue(false, "Fang template found for resistance check");
    }
}

void testShipDatabaseGetShipIds() {
    std::cout << "\n=== ShipDatabase Get Ship IDs ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    auto ids = db.getShipIds();
    assertTrue(ids.size() > 0, "getShipIds returns non-empty list");
    
    // Check that 'fang' is in the list
    bool found = false;
    for (const auto& id : ids) {
        if (id == "fang") found = true;
    }
    assertTrue(found, "fang is in ship ID list");
}

void testShipDatabaseCapitalShips() {
    std::cout << "\n=== ShipDatabase Capital Ships ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    // Verify capital ships are loaded
    const data::ShipTemplate* solarius = db.getShip("solarius");
    if (solarius) {
        assertTrue(solarius->name == "Solarius", "Solarius name correct");
        assertTrue(solarius->ship_class == "Carrier", "Solarius class is Carrier");
        assertTrue(solarius->race == "Solari", "Solarius race is Solari");
        assertTrue(solarius->hull_hp > 10000.0f, "Solarius has high hull HP");
        assertTrue(solarius->armor_hp > 50000.0f, "Solarius has high armor HP");
    } else {
        assertTrue(false, "Solarius carrier found in database");
    }
    
    // Verify titan is loaded
    const data::ShipTemplate* empyrean = db.getShip("empyrean");
    if (empyrean) {
        assertTrue(empyrean->name == "Empyrean", "Empyrean name correct");
        assertTrue(empyrean->ship_class == "Titan", "Empyrean class is Titan");
        assertTrue(empyrean->hull_hp > 100000.0f, "Empyrean has very high hull HP");
    } else {
        assertTrue(false, "Empyrean titan found in database");
    }
    
    // Verify multiple ship categories loaded
    auto ids = db.getShipIds();
    bool hasCapital = false, hasBattleship = false, hasFrigate = false;
    bool hasTech2Cruiser = false, hasMiningBarge = false;
    bool hasMarauder = false, hasIndustrial = false;
    bool hasInterdictor = false, hasStealthBomber = false;
    for (const auto& id : ids) {
        if (id == "solarius") hasCapital = true;
        if (id == "gale") hasBattleship = true;
        if (id == "fang") hasFrigate = true;
        if (id == "wanderer") hasTech2Cruiser = true;
        if (id == "ironbore") hasMiningBarge = true;
        if (id == "ironheart") hasMarauder = true;
        if (id == "drifthauler") hasIndustrial = true;
        if (id == "gripshard") hasInterdictor = true;
        if (id == "shadowfang") hasStealthBomber = true;
    }
    assertTrue(hasCapital, "Capital ships loaded");
    assertTrue(hasBattleship, "Battleships loaded");
    assertTrue(hasFrigate, "Frigates loaded");
    assertTrue(hasTech2Cruiser, "Tech II cruisers loaded");
    assertTrue(hasMiningBarge, "Mining barges loaded");
    assertTrue(hasMarauder, "Marauder battleships loaded");
    assertTrue(hasIndustrial, "Industrial ships loaded");
    assertTrue(hasInterdictor, "Interdictor destroyers loaded");
    assertTrue(hasStealthBomber, "Stealth Bomber frigates loaded");
    assertTrue(ids.size() >= 50, "At least 50 ship templates loaded");
}

void testShipDatabaseMarauders() {
    std::cout << "\n=== ShipDatabase Marauder Ships ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    // Verify all 4 Marauders are loaded
    const data::ShipTemplate* ironheart = db.getShip("ironheart");
    if (ironheart) {
        assertTrue(ironheart->name == "Ironheart", "Ironheart name correct");
        assertTrue(ironheart->ship_class == "Marauder", "Ironheart class is Marauder");
        assertTrue(ironheart->race == "Keldari", "Ironheart race is Keldari");
        assertTrue(ironheart->hull_hp > 8000.0f, "Ironheart has high hull HP");
        assertTrue(ironheart->shield_hp > 10000.0f, "Ironheart has high shield HP");
        assertTrue(ironheart->max_locked_targets >= 10, "Ironheart has 10 locked targets");
    } else {
        assertTrue(false, "Ironheart marauder found in database");
    }
    
    const data::ShipTemplate* monolith = db.getShip("monolith");
    assertTrue(monolith != nullptr, "Monolith marauder found in database");
    if (monolith) {
        assertTrue(monolith->race == "Veyren", "Monolith race is Veyren");
    }
    
    const data::ShipTemplate* majeste = db.getShip("majeste");
    assertTrue(majeste != nullptr, "Majeste marauder found in database");
    if (majeste) {
        assertTrue(majeste->race == "Aurelian", "Majeste race is Aurelian");
    }
    
    const data::ShipTemplate* solarius_prime = db.getShip("solarius_prime");
    assertTrue(solarius_prime != nullptr, "Solarius Prime marauder found in database");
    if (solarius_prime) {
        assertTrue(solarius_prime->race == "Solari", "Solarius Prime race is Solari");
    }
}

void testShipDatabaseInterdictors() {
    std::cout << "\n=== ShipDatabase Interdictor Ships ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    // Verify all 4 Interdictors are loaded
    const data::ShipTemplate* gripshard = db.getShip("gripshard");
    if (gripshard) {
        assertTrue(gripshard->name == "Gripshard", "Gripshard name correct");
        assertTrue(gripshard->ship_class == "Interdictor", "Gripshard class is Interdictor");
        assertTrue(gripshard->race == "Keldari", "Gripshard race is Keldari");
        assertTrue(gripshard->hull_hp > 700.0f, "Gripshard has destroyer-class hull HP");
        assertTrue(gripshard->max_locked_targets >= 7, "Gripshard has 7 locked targets");
    } else {
        assertTrue(false, "Gripshard interdictor found in database");
    }
    
    const data::ShipTemplate* nettvar = db.getShip("nettvar");
    assertTrue(nettvar != nullptr, "Nettvar interdictor found in database");
    if (nettvar) {
        assertTrue(nettvar->race == "Veyren", "Nettvar race is Veyren");
    }
    
    const data::ShipTemplate* barricade = db.getShip("barricade");
    assertTrue(barricade != nullptr, "Barricade interdictor found in database");
    if (barricade) {
        assertTrue(barricade->race == "Aurelian", "Barricade race is Aurelian");
    }
    
    const data::ShipTemplate* denouncer = db.getShip("denouncer");
    assertTrue(denouncer != nullptr, "Denouncer interdictor found in database");
    if (denouncer) {
        assertTrue(denouncer->race == "Solari", "Denouncer race is Solari");
    }
}

void testShipDatabaseStealthBombers() {
    std::cout << "\n=== ShipDatabase Stealth Bomber Ships ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    // Verify all 4 Stealth Bombers are loaded
    const data::ShipTemplate* shadowfang = db.getShip("shadowfang");
    if (shadowfang) {
        assertTrue(shadowfang->name == "Shadowfang", "Shadowfang name correct");
        assertTrue(shadowfang->ship_class == "Stealth Bomber", "Shadowfang class is Stealth Bomber");
        assertTrue(shadowfang->race == "Keldari", "Shadowfang race is Keldari");
        assertTrue(shadowfang->max_targeting_range >= 45000.0f, "Shadowfang has long targeting range");
    } else {
        assertTrue(false, "Shadowfang stealth bomber found in database");
    }
    
    const data::ShipTemplate* frostbane = db.getShip("frostbane");
    assertTrue(frostbane != nullptr, "Frostbane stealth bomber found in database");
    if (frostbane) {
        assertTrue(frostbane->race == "Veyren", "Frostbane race is Veyren");
    }
    
    const data::ShipTemplate* vengeresse = db.getShip("vengeresse");
    assertTrue(vengeresse != nullptr, "Vengeresse stealth bomber found in database");
    if (vengeresse) {
        assertTrue(vengeresse->race == "Aurelian", "Vengeresse race is Aurelian");
    }
    
    const data::ShipTemplate* sanctifier = db.getShip("sanctifier");
    assertTrue(sanctifier != nullptr, "Sanctifier stealth bomber found in database");
    if (sanctifier) {
        assertTrue(sanctifier->race == "Solari", "Sanctifier race is Solari");
    }
}

void testShipDatabaseSecondHACs() {
    std::cout << "\n=== ShipDatabase Second HAC Variants ===" << std::endl;
    
    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }
    
    // Verify all 4 second HAC variants are loaded
    const data::ShipTemplate* gunnolf = db.getShip("gunnolf");
    if (gunnolf) {
        assertTrue(gunnolf->name == "Gunnolf", "Gunnolf name correct");
        assertTrue(gunnolf->ship_class == "Heavy Assault Cruiser", "Gunnolf class is HAC");
        assertTrue(gunnolf->race == "Keldari", "Gunnolf race is Keldari");
        assertTrue(gunnolf->max_targeting_range >= 70000.0f, "Gunnolf has long targeting range");
    } else {
        assertTrue(false, "Gunnolf HAC found in database");
    }
    
    const data::ShipTemplate* valdris = db.getShip("valdris");
    if (valdris) {
        assertTrue(valdris->name == "Valdris", "Valdris name correct");
        assertTrue(valdris->ship_class == "Heavy Assault Cruiser", "Valdris class is HAC");
        assertTrue(valdris->race == "Veyren", "Valdris race is Veyren");
        assertTrue(valdris->shield_hp >= 3000.0f, "Valdris has strong shields");
    } else {
        assertTrue(false, "Valdris HAC found in database");
    }
    
    const data::ShipTemplate* cavalier = db.getShip("cavalier");
    if (cavalier) {
        assertTrue(cavalier->name == "Cavalier", "Cavalier name correct");
        assertTrue(cavalier->ship_class == "Heavy Assault Cruiser", "Cavalier class is HAC");
        assertTrue(cavalier->race == "Aurelian", "Cavalier race is Aurelian");
        assertTrue(cavalier->armor_hp >= 2000.0f, "Cavalier has strong armor");
    } else {
        assertTrue(false, "Cavalier HAC found in database");
    }
    
    const data::ShipTemplate* inquisitor = db.getShip("inquisitor");
    if (inquisitor) {
        assertTrue(inquisitor->name == "Inquisitor", "Inquisitor name correct");
        assertTrue(inquisitor->ship_class == "Heavy Assault Cruiser", "Inquisitor class is HAC");
        assertTrue(inquisitor->race == "Solari", "Inquisitor race is Solari");
        assertTrue(inquisitor->armor_hp >= 2500.0f, "Inquisitor has heavy armor");
        assertTrue(inquisitor->capacitor >= 1400.0f, "Inquisitor has strong capacitor");
    } else {
        assertTrue(false, "Inquisitor HAC found in database");
    }
}

// ==================== WormholeDatabase Tests ====================

void testWormholeDatabaseLoad() {
    std::cout << "\n=== WormholeDatabase Load ===" << std::endl;
    
    data::WormholeDatabase db;
    int count = db.loadFromDirectory("../data");
    if (count == 0) count = db.loadFromDirectory("data");
    if (count == 0) count = db.loadFromDirectory("../../data");
    
    assertTrue(db.getClassCount() == 6, "Loaded all 6 wormhole classes (C1-C6)");
    assertTrue(db.getEffectCount() > 0, "Loaded at least 1 wormhole effect");
}

void testWormholeDatabaseGetClass() {
    std::cout << "\n=== WormholeDatabase Get Class ===" << std::endl;
    
    data::WormholeDatabase db;
    if (db.loadFromDirectory("../data") == 0)
        if (db.loadFromDirectory("data") == 0)
            db.loadFromDirectory("../../data");
    
    const data::WormholeClassTemplate* c1 = db.getWormholeClass("c1");
    if (c1) {
        assertTrue(c1->wormhole_class == 1, "C1 wormhole class is 1");
        assertTrue(c1->difficulty == "easy", "C1 difficulty is easy");
        assertTrue(c1->max_ship_class == "Battlecruiser", "C1 max ship is Battlecruiser");
        assertTrue(!c1->dormant_spawns.empty(), "C1 has dormant spawns");
        assertTrue(c1->salvage_value_multiplier > 0.0f, "C1 has salvage multiplier");
    } else {
        assertTrue(false, "C1 wormhole class found");
    }
    
    const data::WormholeClassTemplate* c6 = db.getWormholeClass("c6");
    if (c6) {
        assertTrue(c6->wormhole_class == 6, "C6 wormhole class is 6");
        assertTrue(c6->difficulty == "extreme", "C6 difficulty is extreme");
        assertTrue(c6->blue_loot_isk > c1->blue_loot_isk, "C6 loot > C1 loot");
    } else {
        assertTrue(false, "C6 wormhole class found");
    }
    
    assertTrue(db.getWormholeClass("nonexistent") == nullptr, "Nonexistent class returns nullptr");
}

void testWormholeDatabaseEffects() {
    std::cout << "\n=== WormholeDatabase Effects ===" << std::endl;
    
    data::WormholeDatabase db;
    if (db.loadFromDirectory("../data") == 0)
        if (db.loadFromDirectory("data") == 0)
            db.loadFromDirectory("../../data");
    
    const data::WormholeEffect* magnetar = db.getEffect("magnetar");
    if (magnetar) {
        assertTrue(magnetar->name == "Magnetar", "Magnetar name correct");
        assertTrue(!magnetar->modifiers.empty(), "Magnetar has modifiers");
        auto it = magnetar->modifiers.find("damage_multiplier");
        assertTrue(it != magnetar->modifiers.end(), "Magnetar has damage_multiplier");
        if (it != magnetar->modifiers.end()) {
            assertTrue(approxEqual(it->second, 1.86f), "Magnetar damage_multiplier is 1.86");
        }
    } else {
        assertTrue(false, "Magnetar effect found");
    }
    
    assertTrue(db.getEffect("nonexistent") == nullptr, "Nonexistent effect returns nullptr");
}

void testWormholeDatabaseClassIds() {
    std::cout << "\n=== WormholeDatabase Class IDs ===" << std::endl;
    
    data::WormholeDatabase db;
    if (db.loadFromDirectory("../data") == 0)
        if (db.loadFromDirectory("data") == 0)
            db.loadFromDirectory("../../data");
    
    auto ids = db.getClassIds();
    assertTrue(ids.size() == 6, "getClassIds returns 6 classes");
    
    auto effect_ids = db.getEffectIds();
    assertTrue(effect_ids.size() == 6, "getEffectIds returns 6 effects");
}

// ==================== WormholeSystem Tests ====================

void testWormholeLifetimeDecay() {
    std::cout << "\n=== Wormhole Lifetime Decay ===" << std::endl;
    
    ecs::World world;
    systems::WormholeSystem whSys(&world);
    
    auto* wh_entity = world.createEntity("wh_1");
    auto* wh = addComp<components::WormholeConnection>(wh_entity);
    wh->wormhole_id = "wh_1";
    wh->max_mass = 500000000.0;
    wh->remaining_mass = 500000000.0;
    wh->max_jump_mass = 20000000.0;
    wh->max_lifetime_hours = 24.0f;
    wh->elapsed_hours = 0.0f;
    
    assertTrue(whSys.isWormholeStable("wh_1"), "Wormhole starts stable");
    
    // Simulate 12 hours (43200 seconds)
    whSys.update(43200.0f);
    assertTrue(whSys.isWormholeStable("wh_1"), "Wormhole stable at 12 hours");
    assertTrue(approxEqual(whSys.getRemainingLifetimeFraction("wh_1"), 0.5f),
               "50% lifetime remaining at 12h");
    
    // Simulate another 13 hours to exceed lifetime
    whSys.update(46800.0f);
    assertTrue(!whSys.isWormholeStable("wh_1"), "Wormhole collapsed after 25 hours");
}

void testWormholeJumpMass() {
    std::cout << "\n=== Wormhole Jump Mass ===" << std::endl;
    
    ecs::World world;
    systems::WormholeSystem whSys(&world);
    
    auto* wh_entity = world.createEntity("wh_2");
    auto* wh = addComp<components::WormholeConnection>(wh_entity);
    wh->max_mass = 100000000.0;
    wh->remaining_mass = 100000000.0;
    wh->max_jump_mass = 20000000.0;
    wh->max_lifetime_hours = 24.0f;
    
    // Ship too heavy for single jump
    bool result = whSys.jumpThroughWormhole("wh_2", 30000000.0);
    assertTrue(!result, "Ship too heavy for wormhole rejected");
    assertTrue(approxEqual(whSys.getRemainingMassFraction("wh_2"), 1.0f),
               "Mass unchanged on rejected jump");
    
    // Valid jump
    result = whSys.jumpThroughWormhole("wh_2", 15000000.0);
    assertTrue(result, "Valid ship mass jump succeeds");
    assertTrue(approxEqual(whSys.getRemainingMassFraction("wh_2"), 0.85f),
               "Mass reduced by ship mass");
}

void testWormholeMassCollapse() {
    std::cout << "\n=== Wormhole Mass Collapse ===" << std::endl;
    
    ecs::World world;
    systems::WormholeSystem whSys(&world);
    
    auto* wh_entity = world.createEntity("wh_3");
    auto* wh = addComp<components::WormholeConnection>(wh_entity);
    wh->max_mass = 30000000.0;
    wh->remaining_mass = 30000000.0;
    wh->max_jump_mass = 20000000.0;
    wh->max_lifetime_hours = 24.0f;
    
    // First jump takes most of the mass
    bool result = whSys.jumpThroughWormhole("wh_3", 18000000.0);
    assertTrue(result, "First jump succeeds");
    assertTrue(whSys.isWormholeStable("wh_3"), "Still stable after first jump");
    
    // Second jump depletes remaining mass and collapses
    result = whSys.jumpThroughWormhole("wh_3", 15000000.0);
    assertTrue(!result, "Second jump fails (not enough remaining mass)");
    
    // A jump that exactly uses remaining mass
    result = whSys.jumpThroughWormhole("wh_3", 12000000.0);
    assertTrue(result, "Exact remaining mass jump succeeds");
    assertTrue(!whSys.isWormholeStable("wh_3"), "Wormhole collapsed after mass depleted");
}

void testWormholeNonexistent() {
    std::cout << "\n=== Wormhole Nonexistent ===" << std::endl;
    
    ecs::World world;
    systems::WormholeSystem whSys(&world);
    
    assertTrue(!whSys.isWormholeStable("ghost"), "Nonexistent wormhole is not stable");
    assertTrue(whSys.getRemainingMassFraction("ghost") < 0.0f, "Nonexistent returns -1 mass fraction");
    assertTrue(whSys.getRemainingLifetimeFraction("ghost") < 0.0f, "Nonexistent returns -1 lifetime fraction");
    assertTrue(!whSys.jumpThroughWormhole("ghost", 1000.0), "Jump through nonexistent fails");
}

void testSolarSystemComponent() {
    std::cout << "\n=== SolarSystem Component ===" << std::endl;
    
    ecs::World world;
    
    auto* sys_entity = world.createEntity("j123456");
    auto* solar = addComp<components::SolarSystem>(sys_entity);
    solar->system_id = "j123456";
    solar->system_name = "J123456";
    solar->wormhole_class = 3;
    solar->effect_name = "magnetar";
    solar->dormants_spawned = false;
    
    assertTrue(solar->wormhole_class == 3, "SolarSystem wormhole class set correctly");
    assertTrue(solar->effect_name == "magnetar", "SolarSystem effect set correctly");
    assertTrue(!solar->dormants_spawned, "Dormants not yet spawned");
    
    solar->dormants_spawned = true;
    assertTrue(solar->dormants_spawned, "Dormants marked as spawned");
}

// ==================== FleetSystem Tests ====================

void testFleetCreateAndDisband() {
    std::cout << "\n=== Fleet Create and Disband ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* player1 = world.createEntity("player_1");
    addComp<components::Player>(player1)->character_name = "Commander";
    
    // Create fleet
    std::string fleet_id = fleetSys.createFleet("player_1", "Alpha Fleet");
    assertTrue(!fleet_id.empty(), "Fleet created successfully");
    assertTrue(fleetSys.getFleetCount() == 1, "Fleet count is 1");
    assertTrue(fleetSys.getMemberCount(fleet_id) == 1, "Fleet has 1 member (FC)");
    
    const systems::Fleet* fleet = fleetSys.getFleet(fleet_id);
    assertTrue(fleet != nullptr, "Fleet retrievable");
    assertTrue(fleet->fleet_name == "Alpha Fleet", "Fleet name correct");
    assertTrue(fleet->commander_entity_id == "player_1", "Commander is player_1");
    
    // FC has FleetMembership component
    auto* fm = player1->getComponent<components::FleetMembership>();
    assertTrue(fm != nullptr, "FC has FleetMembership component");
    assertTrue(fm->role == "FleetCommander", "FC role is FleetCommander");
    
    // Cannot create another fleet while in one
    std::string fleet2 = fleetSys.createFleet("player_1", "Beta Fleet");
    assertTrue(fleet2.empty(), "Cannot create fleet while already in one");
    
    // Disband
    assertTrue(fleetSys.disbandFleet(fleet_id, "player_1"), "FC can disband fleet");
    assertTrue(fleetSys.getFleetCount() == 0, "No fleets after disband");
    assertTrue(player1->getComponent<components::FleetMembership>() == nullptr,
               "FleetMembership removed after disband");
}

void testFleetAddRemoveMembers() {
    std::cout << "\n=== Fleet Add/Remove Members ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* p2 = world.createEntity("pilot_2");
    addComp<components::Player>(p2)->character_name = "Wing1";
    auto* p3 = world.createEntity("pilot_3");
    addComp<components::Player>(p3)->character_name = "Wing2";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Test Fleet");
    
    // Add members
    assertTrue(fleetSys.addMember(fleet_id, "pilot_2"), "Add pilot_2 succeeds");
    assertTrue(fleetSys.addMember(fleet_id, "pilot_3"), "Add pilot_3 succeeds");
    assertTrue(fleetSys.getMemberCount(fleet_id) == 3, "Fleet has 3 members");
    
    // Cannot add same member twice
    assertTrue(!fleetSys.addMember(fleet_id, "pilot_2"), "Cannot add duplicate member");
    
    // Cannot add nonexistent entity
    assertTrue(!fleetSys.addMember(fleet_id, "ghost"), "Cannot add nonexistent entity");
    
    // Entity fleet lookup
    assertTrue(fleetSys.getFleetForEntity("pilot_2") == fleet_id, "pilot_2 fleet lookup correct");
    assertTrue(fleetSys.getFleetForEntity("ghost").empty(), "Nonexistent entity has no fleet");
    
    // Remove member
    assertTrue(fleetSys.removeMember(fleet_id, "pilot_2"), "Remove pilot_2 succeeds");
    assertTrue(fleetSys.getMemberCount(fleet_id) == 2, "Fleet has 2 members after remove");
    assertTrue(fleetSys.getFleetForEntity("pilot_2").empty(), "Removed member has no fleet");
    assertTrue(p2->getComponent<components::FleetMembership>() == nullptr,
               "Removed member has no FleetMembership component");
}

void testFleetFCLeavePromotes() {
    std::cout << "\n=== Fleet FC Leave Auto-Promotes ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* p2 = world.createEntity("pilot_2");
    addComp<components::Player>(p2)->character_name = "Pilot2";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Test Fleet");
    fleetSys.addMember(fleet_id, "pilot_2");
    
    // FC leaves
    fleetSys.removeMember(fleet_id, "fc");
    assertTrue(fleetSys.getFleetCount() == 1, "Fleet still exists after FC leave");
    
    const systems::Fleet* fleet = fleetSys.getFleet(fleet_id);
    assertTrue(fleet != nullptr, "Fleet still retrievable");
    assertTrue(fleet->commander_entity_id == "pilot_2", "pilot_2 auto-promoted to FC");
    
    auto* fm = p2->getComponent<components::FleetMembership>();
    assertTrue(fm != nullptr && fm->role == "FleetCommander", "Promoted member has FC role");
}

void testFleetDisbandOnEmpty() {
    std::cout << "\n=== Fleet Disbands When Empty ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Solo Fleet");
    assertTrue(fleetSys.getFleetCount() == 1, "Fleet exists");
    
    fleetSys.removeMember(fleet_id, "fc");
    assertTrue(fleetSys.getFleetCount() == 0, "Fleet auto-disbanded when last member leaves");
}

void testFleetPromoteMember() {
    std::cout << "\n=== Fleet Promote Member ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* p2 = world.createEntity("pilot_2");
    addComp<components::Player>(p2)->character_name = "Pilot2";
    auto* p3 = world.createEntity("pilot_3");
    addComp<components::Player>(p3)->character_name = "Pilot3";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Test Fleet");
    fleetSys.addMember(fleet_id, "pilot_2");
    fleetSys.addMember(fleet_id, "pilot_3");
    
    // Promote to WingCommander
    assertTrue(fleetSys.promoteMember(fleet_id, "fc", "pilot_2", "WingCommander"),
               "Promote pilot_2 to WingCommander succeeds");
    auto* fm2 = p2->getComponent<components::FleetMembership>();
    assertTrue(fm2 != nullptr && fm2->role == "WingCommander", "pilot_2 role updated");
    
    // Promote to SquadCommander
    assertTrue(fleetSys.promoteMember(fleet_id, "fc", "pilot_3", "SquadCommander"),
               "Promote pilot_3 to SquadCommander succeeds");
    
    // Non-FC cannot promote
    assertTrue(!fleetSys.promoteMember(fleet_id, "pilot_2", "pilot_3", "Member"),
               "Non-FC cannot promote");
    
    // Invalid role
    assertTrue(!fleetSys.promoteMember(fleet_id, "fc", "pilot_2", "Admiral"),
               "Invalid role rejected");
    
    // Promote to FC transfers command
    assertTrue(fleetSys.promoteMember(fleet_id, "fc", "pilot_2", "FleetCommander"),
               "Transfer FC to pilot_2 succeeds");
    const systems::Fleet* fleet = fleetSys.getFleet(fleet_id);
    assertTrue(fleet->commander_entity_id == "pilot_2", "pilot_2 is now FC");
    auto* fm_fc = fc->getComponent<components::FleetMembership>();
    assertTrue(fm_fc->role == "Member", "Old FC demoted to Member");
}

void testFleetSquadAndWingOrganization() {
    std::cout << "\n=== Fleet Squad and Wing Organization ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* p2 = world.createEntity("p2");
    addComp<components::Player>(p2)->character_name = "P2";
    auto* p3 = world.createEntity("p3");
    addComp<components::Player>(p3)->character_name = "P3";
    auto* p4 = world.createEntity("p4");
    addComp<components::Player>(p4)->character_name = "P4";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Organized Fleet");
    fleetSys.addMember(fleet_id, "p2");
    fleetSys.addMember(fleet_id, "p3");
    fleetSys.addMember(fleet_id, "p4");
    
    // Assign to squads
    assertTrue(fleetSys.assignToSquad(fleet_id, "p2", "squad_alpha"),
               "Assign p2 to squad_alpha");
    assertTrue(fleetSys.assignToSquad(fleet_id, "p3", "squad_alpha"),
               "Assign p3 to squad_alpha");
    assertTrue(fleetSys.assignToSquad(fleet_id, "p4", "squad_bravo"),
               "Assign p4 to squad_bravo");
    
    // Check squad membership
    auto* fm2 = p2->getComponent<components::FleetMembership>();
    assertTrue(fm2->squad_id == "squad_alpha", "p2 squad_id is squad_alpha");
    
    const systems::Fleet* fleet = fleetSys.getFleet(fleet_id);
    assertTrue(fleet->squads.at("squad_alpha").size() == 2, "squad_alpha has 2 members");
    assertTrue(fleet->squads.at("squad_bravo").size() == 1, "squad_bravo has 1 member");
    
    // Assign squads to wings
    assertTrue(fleetSys.assignSquadToWing(fleet_id, "squad_alpha", "wing_1"),
               "Assign squad_alpha to wing_1");
    assertTrue(fleetSys.assignSquadToWing(fleet_id, "squad_bravo", "wing_1"),
               "Assign squad_bravo to wing_1");
    
    assertTrue(fleet->wings.at("wing_1").size() == 2, "wing_1 has 2 squads");
    
    // Nonexistent squad cannot be assigned
    assertTrue(!fleetSys.assignSquadToWing(fleet_id, "ghost_squad", "wing_2"),
               "Cannot assign nonexistent squad to wing");
    
    // Non-member cannot be assigned to squad
    assertTrue(!fleetSys.assignToSquad(fleet_id, "ghost", "squad_alpha"),
               "Cannot assign non-member to squad");
}

void testFleetBonuses() {
    std::cout << "\n=== Fleet Bonuses ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* p2 = world.createEntity("booster");
    addComp<components::Player>(p2)->character_name = "Booster";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Bonus Fleet");
    fleetSys.addMember(fleet_id, "booster");
    
    // Set booster
    assertTrue(fleetSys.setBooster(fleet_id, "armor", "booster"), "Set armor booster");
    assertTrue(fleetSys.setBooster(fleet_id, "shield", "booster"), "Set shield booster");
    
    // Invalid booster type
    assertTrue(!fleetSys.setBooster(fleet_id, "invalid", "booster"), "Invalid booster type rejected");
    
    // Non-member cannot be booster
    assertTrue(!fleetSys.setBooster(fleet_id, "armor", "ghost"), "Non-member cannot be booster");
    
    // Check bonus definitions
    auto armor_bonuses = fleetSys.getBonusesForType("armor");
    assertTrue(armor_bonuses.size() == 2, "Armor has 2 bonuses");
    assertTrue(approxEqual(armor_bonuses[0].value, 0.10f), "Armor HP bonus is 10%");
    assertTrue(approxEqual(armor_bonuses[1].value, 0.05f), "Armor resist bonus is 5%");
    
    auto skirmish_bonuses = fleetSys.getBonusesForType("skirmish");
    assertTrue(skirmish_bonuses.size() == 2, "Skirmish has 2 bonuses");
    assertTrue(approxEqual(skirmish_bonuses[0].value, 0.15f), "Skirmish speed bonus is 15%");
    
    auto info_bonuses = fleetSys.getBonusesForType("information");
    assertTrue(info_bonuses.size() == 2, "Information has 2 bonuses");
    assertTrue(approxEqual(info_bonuses[0].value, 0.20f), "Info targeting range bonus is 20%");
    
    // Update applies bonuses to FleetMembership components
    fleetSys.update(1.0f);
    auto* fm_fc = fc->getComponent<components::FleetMembership>();
    assertTrue(!fm_fc->active_bonuses.empty(), "FC has active bonuses after update");
    assertTrue(fm_fc->active_bonuses.find("armor_hp_bonus") != fm_fc->active_bonuses.end(),
               "FC has armor_hp_bonus");
}

void testFleetBroadcastTarget() {
    std::cout << "\n=== Fleet Broadcast Target ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    addComp<components::Target>(fc);
    addComp<components::Ship>(fc);
    
    auto* p2 = world.createEntity("pilot_2");
    addComp<components::Player>(p2)->character_name = "Pilot2";
    addComp<components::Target>(p2);
    addComp<components::Ship>(p2);
    
    auto* enemy = world.createEntity("enemy_1");
    addComp<components::Health>(enemy);
    
    std::string fleet_id = fleetSys.createFleet("fc", "Combat Fleet");
    fleetSys.addMember(fleet_id, "pilot_2");
    
    // Broadcast target
    int notified = fleetSys.broadcastTarget(fleet_id, "fc", "enemy_1");
    assertTrue(notified == 2, "2 fleet members notified of target");
    
    // Both FC and pilot_2 should be locking
    auto* fc_target = fc->getComponent<components::Target>();
    assertTrue(fc_target->locking_targets.find("enemy_1") != fc_target->locking_targets.end(),
               "FC started locking broadcast target");
    
    auto* p2_target = p2->getComponent<components::Target>();
    assertTrue(p2_target->locking_targets.find("enemy_1") != p2_target->locking_targets.end(),
               "pilot_2 started locking broadcast target");
    
    // Broadcasting nonexistent target returns 0
    int none = fleetSys.broadcastTarget(fleet_id, "fc", "nonexistent");
    assertTrue(none == 0, "Broadcast nonexistent target returns 0");
}

void testFleetWarp() {
    std::cout << "\n=== Fleet Warp ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* fc_pos = addComp<components::Position>(fc);
    fc_pos->x = 0; fc_pos->y = 0; fc_pos->z = 0;
    auto* fc_vel = addComp<components::Velocity>(fc);
    fc_vel->max_speed = 1000.0f;
    
    auto* p2 = world.createEntity("pilot_2");
    addComp<components::Player>(p2)->character_name = "Pilot2";
    auto* p2_pos = addComp<components::Position>(p2);
    p2_pos->x = 100; p2_pos->y = 0; p2_pos->z = 0;
    auto* p2_vel = addComp<components::Velocity>(p2);
    p2_vel->max_speed = 800.0f;
    
    std::string fleet_id = fleetSys.createFleet("fc", "Warp Fleet");
    fleetSys.addMember(fleet_id, "pilot_2");
    
    // FC can fleet warp
    int warped = fleetSys.fleetWarp(fleet_id, "fc", 10000.0f, 0.0f, 0.0f);
    assertTrue(warped == 2, "2 fleet members initiated warp");
    assertTrue(fc_vel->vx > 0.0f, "FC velocity set toward destination");
    assertTrue(p2_vel->vx > 0.0f, "pilot_2 velocity set toward destination");
    
    // Regular member cannot fleet warp
    int no_warp = fleetSys.fleetWarp(fleet_id, "pilot_2", 20000.0f, 0.0f, 0.0f);
    assertTrue(no_warp == 0, "Regular member cannot fleet warp");
}

void testFleetDisbandPermission() {
    std::cout << "\n=== Fleet Disband Permission ===" << std::endl;
    
    ecs::World world;
    systems::FleetSystem fleetSys(&world);
    
    auto* fc = world.createEntity("fc");
    addComp<components::Player>(fc)->character_name = "FC";
    auto* p2 = world.createEntity("pilot_2");
    addComp<components::Player>(p2)->character_name = "Pilot2";
    
    std::string fleet_id = fleetSys.createFleet("fc", "Test Fleet");
    fleetSys.addMember(fleet_id, "pilot_2");
    
    // Non-FC cannot disband
    assertTrue(!fleetSys.disbandFleet(fleet_id, "pilot_2"), "Non-FC cannot disband fleet");
    assertTrue(fleetSys.getFleetCount() == 1, "Fleet still exists");
    
    // Nonexistent fleet
    assertTrue(!fleetSys.disbandFleet("ghost_fleet", "fc"), "Cannot disband nonexistent fleet");
}

void testFleetMembershipComponent() {
    std::cout << "\n=== FleetMembership Component ===" << std::endl;
    
    ecs::World world;
    
    auto* entity = world.createEntity("test_pilot");
    auto* fm = addComp<components::FleetMembership>(entity);
    fm->fleet_id = "fleet_1";
    fm->role = "Member";
    fm->squad_id = "squad_alpha";
    fm->wing_id = "wing_1";
    fm->active_bonuses["armor_hp_bonus"] = 0.10f;
    
    assertTrue(fm->fleet_id == "fleet_1", "FleetMembership fleet_id correct");
    assertTrue(fm->role == "Member", "FleetMembership role correct");
    assertTrue(fm->squad_id == "squad_alpha", "FleetMembership squad_id correct");
    assertTrue(fm->wing_id == "wing_1", "FleetMembership wing_id correct");
    assertTrue(approxEqual(fm->active_bonuses["armor_hp_bonus"], 0.10f),
               "FleetMembership bonus value correct");
}

// ==================== WorldPersistence Tests ====================

void testSerializeDeserializeBasicEntity() {
    std::cout << "\n=== Serialize/Deserialize Basic Entity ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("ship_1");

    auto pos = std::make_unique<components::Position>();
    pos->x = 100.0f; pos->y = 200.0f; pos->z = 300.0f; pos->rotation = 1.5f;
    entity->addComponent(std::move(pos));

    auto vel = std::make_unique<components::Velocity>();
    vel->vx = 10.0f; vel->vy = 20.0f; vel->vz = 30.0f; vel->max_speed = 500.0f;
    entity->addComponent(std::move(vel));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    assertTrue(!json.empty(), "Serialized JSON is not empty");
    assertTrue(json.find("ship_1") != std::string::npos, "JSON contains entity id");

    // Deserialize into a new world
    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialize succeeds");
    assertTrue(world2.getEntityCount() == 1, "Loaded world has 1 entity");

    auto* loaded = world2.getEntity("ship_1");
    assertTrue(loaded != nullptr, "Loaded entity found by id");

    auto* lpos = loaded->getComponent<components::Position>();
    assertTrue(lpos != nullptr, "Loaded entity has Position");
    assertTrue(approxEqual(lpos->x, 100.0f), "Position.x preserved");
    assertTrue(approxEqual(lpos->y, 200.0f), "Position.y preserved");
    assertTrue(approxEqual(lpos->z, 300.0f), "Position.z preserved");
    assertTrue(approxEqual(lpos->rotation, 1.5f), "Position.rotation preserved");

    auto* lvel = loaded->getComponent<components::Velocity>();
    assertTrue(lvel != nullptr, "Loaded entity has Velocity");
    assertTrue(approxEqual(lvel->vx, 10.0f), "Velocity.vx preserved");
    assertTrue(approxEqual(lvel->max_speed, 500.0f), "Velocity.max_speed preserved");
}

void testSerializeDeserializeHealthCapacitor() {
    std::cout << "\n=== Serialize/Deserialize Health & Capacitor ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("tanker");

    auto hp = std::make_unique<components::Health>();
    hp->shield_hp = 450.0f; hp->shield_max = 500.0f;
    hp->armor_hp = 300.0f; hp->armor_max = 400.0f;
    hp->hull_hp = 200.0f; hp->hull_max = 250.0f;
    hp->shield_recharge_rate = 5.0f;
    hp->shield_em_resist = 0.1f;
    hp->armor_thermal_resist = 0.35f;
    entity->addComponent(std::move(hp));

    auto cap = std::make_unique<components::Capacitor>();
    cap->capacitor = 180.0f; cap->capacitor_max = 250.0f; cap->recharge_rate = 4.0f;
    entity->addComponent(std::move(cap));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    auto* loaded = world2.getEntity("tanker");
    assertTrue(loaded != nullptr, "Entity loaded");

    auto* lhp = loaded->getComponent<components::Health>();
    assertTrue(lhp != nullptr, "Health component loaded");
    assertTrue(approxEqual(lhp->shield_hp, 450.0f), "Shield HP preserved");
    assertTrue(approxEqual(lhp->shield_max, 500.0f), "Shield max preserved");
    assertTrue(approxEqual(lhp->armor_hp, 300.0f), "Armor HP preserved");
    assertTrue(approxEqual(lhp->hull_hp, 200.0f), "Hull HP preserved");
    assertTrue(approxEqual(lhp->shield_recharge_rate, 5.0f), "Shield recharge rate preserved");
    assertTrue(approxEqual(lhp->shield_em_resist, 0.1f), "Shield EM resist preserved");
    assertTrue(approxEqual(lhp->armor_thermal_resist, 0.35f), "Armor thermal resist preserved");

    auto* lcap = loaded->getComponent<components::Capacitor>();
    assertTrue(lcap != nullptr, "Capacitor component loaded");
    assertTrue(approxEqual(lcap->capacitor, 180.0f), "Capacitor current preserved");
    assertTrue(approxEqual(lcap->capacitor_max, 250.0f), "Capacitor max preserved");
    assertTrue(approxEqual(lcap->recharge_rate, 4.0f), "Capacitor recharge rate preserved");
}

void testSerializeDeserializeShipAndFaction() {
    std::cout << "\n=== Serialize/Deserialize Ship & Faction ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("player_ship");

    auto ship = std::make_unique<components::Ship>();
    ship->ship_type = "Cruiser";
    ship->ship_class = "Cruiser";
    ship->ship_name = "Caracal";
    ship->race = "Veyren";
    ship->cpu_max = 350.0f;
    ship->powergrid_max = 200.0f;
    ship->signature_radius = 140.0f;
    ship->scan_resolution = 250.0f;
    ship->max_locked_targets = 6;
    ship->max_targeting_range = 55000.0f;
    entity->addComponent(std::move(ship));

    auto fac = std::make_unique<components::Faction>();
    fac->faction_name = "Veyren";
    entity->addComponent(std::move(fac));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    auto* loaded = world2.getEntity("player_ship");
    assertTrue(loaded != nullptr, "Entity loaded");

    auto* lship = loaded->getComponent<components::Ship>();
    assertTrue(lship != nullptr, "Ship component loaded");
    assertTrue(lship->ship_name == "Caracal", "Ship name preserved");
    assertTrue(lship->race == "Veyren", "Ship race preserved");
    assertTrue(lship->ship_class == "Cruiser", "Ship class preserved");
    assertTrue(approxEqual(lship->cpu_max, 350.0f), "CPU max preserved");
    assertTrue(lship->max_locked_targets == 6, "Max locked targets preserved");
    assertTrue(approxEqual(lship->max_targeting_range, 55000.0f), "Max targeting range preserved");

    auto* lfac = loaded->getComponent<components::Faction>();
    assertTrue(lfac != nullptr, "Faction component loaded");
    assertTrue(lfac->faction_name == "Veyren", "Faction name preserved");
}

void testSerializeDeserializeStandings() {
    std::cout << "\n=== Serialize/Deserialize Standings ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("player_1");

    // Add Standings component with test data
    auto standings = std::make_unique<components::Standings>();
    standings->personal_standings["npc_pirate_001"] = -5.0f;
    standings->personal_standings["player_friend"] = 8.5f;
    standings->corporation_standings["Republic Fleet"] = 3.0f;
    standings->corporation_standings["Venom Syndicate"] = -7.5f;
    standings->faction_standings["Keldari"] = 2.5f;
    standings->faction_standings["Solari"] = -1.5f;
    entity->addComponent(std::move(standings));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    auto* loaded = world2.getEntity("player_1");
    assertTrue(loaded != nullptr, "Entity loaded");

    auto* lstandings = loaded->getComponent<components::Standings>();
    assertTrue(lstandings != nullptr, "Standings component loaded");
    
    // Check personal standings
    assertTrue(lstandings->personal_standings.size() == 2, "Personal standings count preserved");
    assertTrue(approxEqual(lstandings->personal_standings["npc_pirate_001"], -5.0f), "Personal standing (pirate) preserved");
    assertTrue(approxEqual(lstandings->personal_standings["player_friend"], 8.5f), "Personal standing (friend) preserved");
    
    // Check corporation standings
    assertTrue(lstandings->corporation_standings.size() == 2, "Corporation standings count preserved");
    assertTrue(approxEqual(lstandings->corporation_standings["Republic Fleet"], 3.0f), "Corporation standing (Republic Fleet) preserved");
    assertTrue(approxEqual(lstandings->corporation_standings["Venom Syndicate"], -7.5f), "Corporation standing (Venom Syndicate) preserved");
    
    // Check faction standings
    assertTrue(lstandings->faction_standings.size() == 2, "Faction standings count preserved");
    assertTrue(approxEqual(lstandings->faction_standings["Keldari"], 2.5f), "Faction standing (Keldari) preserved");
    assertTrue(approxEqual(lstandings->faction_standings["Solari"], -1.5f), "Faction standing (Solari) preserved");
}

void testStandingsGetStanding() {
    std::cout << "\n=== Standings getStandingWith ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("player_1");

    auto standings = std::make_unique<components::Standings>();
    standings->personal_standings["npc_001"] = -5.0f;
    standings->corporation_standings["TestCorp"] = 3.0f;
    standings->faction_standings["Veyren"] = 7.0f;
    entity->addComponent(std::move(standings));

    auto* comp = entity->getComponent<components::Standings>();
    
    // Personal standing has highest priority
    float standing1 = comp->getStandingWith("npc_001", "", "");
    assertTrue(approxEqual(standing1, -5.0f), "Personal standing returned");
    
    // Corporation standing used when no personal standing
    float standing2 = comp->getStandingWith("npc_002", "TestCorp", "");
    assertTrue(approxEqual(standing2, 3.0f), "Corporation standing returned");
    
    // Faction standing used when no personal or corp standing
    float standing3 = comp->getStandingWith("npc_003", "OtherCorp", "Veyren");
    assertTrue(approxEqual(standing3, 7.0f), "Faction standing returned");
    
    // Neutral (0) when no standing exists
    float standing4 = comp->getStandingWith("unknown", "UnknownCorp", "UnknownFaction");
    assertTrue(approxEqual(standing4, 0.0f), "Neutral standing for unknown entity");
    
    // Personal standing overrides corporation
    comp->personal_standings["npc_004"] = 9.0f;
    float standing5 = comp->getStandingWith("npc_004", "TestCorp", "");
    assertTrue(approxEqual(standing5, 9.0f), "Personal standing overrides corporation");
}

void testStandingsModify() {
    std::cout << "\n=== Standings modifyStanding ===" << std::endl;

    std::map<std::string, float> test_standings;
    
    // Start with no standing (implicit 0)
    components::Standings::modifyStanding(test_standings, "entity1", 2.5f);
    assertTrue(approxEqual(test_standings["entity1"], 2.5f), "Standing increased from 0 to 2.5");
    
    // Increase existing standing
    components::Standings::modifyStanding(test_standings, "entity1", 3.0f);
    assertTrue(approxEqual(test_standings["entity1"], 5.5f), "Standing increased to 5.5");
    
    // Decrease standing
    components::Standings::modifyStanding(test_standings, "entity1", -2.0f);
    assertTrue(approxEqual(test_standings["entity1"], 3.5f), "Standing decreased to 3.5");
    
    // Clamp at maximum (10.0)
    components::Standings::modifyStanding(test_standings, "entity1", 15.0f);
    assertTrue(approxEqual(test_standings["entity1"], 10.0f), "Standing clamped at max (10.0)");
    
    // Clamp at minimum (-10.0)
    components::Standings::modifyStanding(test_standings, "entity2", -20.0f);
    assertTrue(approxEqual(test_standings["entity2"], -10.0f), "Standing clamped at min (-10.0)");
    
    // Negative adjustment from positive
    test_standings["entity3"] = 5.0f;
    components::Standings::modifyStanding(test_standings, "entity3", -8.0f);
    assertTrue(approxEqual(test_standings["entity3"], -3.0f), "Standing went from +5 to -3");
}

void testSerializeDeserializeAIAndWeapon() {
    std::cout << "\n=== Serialize/Deserialize AI & Weapon ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("npc_1");

    auto ai = std::make_unique<components::AI>();
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state = components::AI::State::Attacking;
    ai->target_entity_id = "player_1";
    ai->orbit_distance = 2500.0f;
    ai->awareness_range = 60000.0f;
    entity->addComponent(std::move(ai));

    auto weapon = std::make_unique<components::Weapon>();
    weapon->weapon_type = "Missile";
    weapon->damage_type = "kinetic";
    weapon->damage = 75.0f;
    weapon->optimal_range = 20000.0f;
    weapon->rate_of_fire = 8.0f;
    weapon->capacitor_cost = 15.0f;
    weapon->ammo_type = "Scourge";
    weapon->ammo_count = 50;
    entity->addComponent(std::move(weapon));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    auto* loaded = world2.getEntity("npc_1");
    assertTrue(loaded != nullptr, "NPC entity loaded");

    auto* lai = loaded->getComponent<components::AI>();
    assertTrue(lai != nullptr, "AI component loaded");
    assertTrue(lai->behavior == components::AI::Behavior::Aggressive, "AI behavior preserved");
    assertTrue(lai->state == components::AI::State::Attacking, "AI state preserved");
    assertTrue(lai->target_entity_id == "player_1", "AI target preserved");
    assertTrue(approxEqual(lai->orbit_distance, 2500.0f), "AI orbit distance preserved");

    auto* lwep = loaded->getComponent<components::Weapon>();
    assertTrue(lwep != nullptr, "Weapon component loaded");
    assertTrue(lwep->weapon_type == "Missile", "Weapon type preserved");
    assertTrue(lwep->damage_type == "kinetic", "Damage type preserved");
    assertTrue(approxEqual(lwep->damage, 75.0f), "Weapon damage preserved");
    assertTrue(lwep->ammo_type == "Scourge", "Ammo type preserved");
    assertTrue(lwep->ammo_count == 50, "Ammo count preserved");
}

void testSerializeDeserializePlayerComponent() {
    std::cout << "\n=== Serialize/Deserialize Player Component ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("player_42");

    auto player = std::make_unique<components::Player>();
    player->player_id = "steam_12345";
    player->character_name = "TestPilot";
    player->isk = 5000000.0;
    player->corporation = "Test Corp";
    entity->addComponent(std::move(player));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    auto* loaded = world2.getEntity("player_42");
    assertTrue(loaded != nullptr, "Player entity loaded");

    auto* lp = loaded->getComponent<components::Player>();
    assertTrue(lp != nullptr, "Player component loaded");
    assertTrue(lp->player_id == "steam_12345", "Player ID preserved");
    assertTrue(lp->character_name == "TestPilot", "Character name preserved");
    assertTrue(lp->isk > 4999999.0 && lp->isk < 5000001.0, "ISK preserved");
    assertTrue(lp->corporation == "Test Corp", "Corporation preserved");
}

void testSerializeDeserializeMultipleEntities() {
    std::cout << "\n=== Serialize/Deserialize Multiple Entities ===" << std::endl;

    ecs::World world;

    // Create 3 entities with different component combinations
    auto* e1 = world.createEntity("ship_a");
    auto p1 = std::make_unique<components::Position>();
    p1->x = 10.0f;
    e1->addComponent(std::move(p1));

    auto* e2 = world.createEntity("ship_b");
    auto p2 = std::make_unique<components::Position>();
    p2->x = 20.0f;
    e2->addComponent(std::move(p2));
    auto h2 = std::make_unique<components::Health>();
    h2->shield_hp = 999.0f;
    e2->addComponent(std::move(h2));

    auto* e3 = world.createEntity("ship_c");
    auto p3 = std::make_unique<components::Position>();
    p3->x = 30.0f;
    e3->addComponent(std::move(p3));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    assertTrue(world2.getEntityCount() == 3, "All 3 entities loaded");
    assertTrue(world2.getEntity("ship_a") != nullptr, "ship_a loaded");
    assertTrue(world2.getEntity("ship_b") != nullptr, "ship_b loaded");
    assertTrue(world2.getEntity("ship_c") != nullptr, "ship_c loaded");

    auto* lb = world2.getEntity("ship_b");
    auto* lhp = lb->getComponent<components::Health>();
    assertTrue(lhp != nullptr, "ship_b has Health component");
    assertTrue(approxEqual(lhp->shield_hp, 999.0f), "ship_b shield HP preserved");
}

void testSaveLoadFile() {
    std::cout << "\n=== Save/Load World File ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("file_test");
    auto pos = std::make_unique<components::Position>();
    pos->x = 42.0f; pos->y = 84.0f;
    entity->addComponent(std::move(pos));

    data::WorldPersistence persistence;
    std::string filepath = "/tmp/eve_test_world.json";

    bool saved = persistence.saveWorld(&world, filepath);
    assertTrue(saved, "World saved to file");

    // Verify file exists
    std::ifstream check(filepath);
    assertTrue(check.good(), "Save file exists on disk");
    check.close();

    ecs::World world2;
    bool loaded = persistence.loadWorld(&world2, filepath);
    assertTrue(loaded, "World loaded from file");
    assertTrue(world2.getEntityCount() == 1, "Loaded world has 1 entity");

    auto* le = world2.getEntity("file_test");
    assertTrue(le != nullptr, "Entity loaded from file");
    auto* lpos = le->getComponent<components::Position>();
    assertTrue(lpos != nullptr, "Position loaded from file");
    assertTrue(approxEqual(lpos->x, 42.0f), "Position.x loaded from file");
    assertTrue(approxEqual(lpos->y, 84.0f), "Position.y loaded from file");

    // Clean up
    std::remove(filepath.c_str());
}

void testLoadNonexistentFile() {
    std::cout << "\n=== Load Nonexistent File ===" << std::endl;

    ecs::World world;
    data::WorldPersistence persistence;
    bool loaded = persistence.loadWorld(&world, "/tmp/does_not_exist_12345.json");
    assertTrue(!loaded, "Loading nonexistent file returns false");
    assertTrue(world.getEntityCount() == 0, "World unchanged on failed load");
}

void testSerializeDeserializeWormholeAndSolarSystem() {
    std::cout << "\n=== Serialize/Deserialize Wormhole & SolarSystem ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("wh_j123456");

    auto ss = std::make_unique<components::SolarSystem>();
    ss->system_id = "j123456";
    ss->system_name = "J123456";
    ss->wormhole_class = 3;
    ss->effect_name = "magnetar";
    ss->dormants_spawned = true;
    entity->addComponent(std::move(ss));

    auto* wh_entity = world.createEntity("wh_conn_1");
    auto wh = std::make_unique<components::WormholeConnection>();
    wh->wormhole_id = "wh_001";
    wh->source_system = "j123456";
    wh->destination_system = "jita";
    wh->max_mass = 1000000000.0;
    wh->remaining_mass = 750000000.0;
    wh->max_jump_mass = 300000000.0;
    wh->max_lifetime_hours = 16.0f;
    wh->elapsed_hours = 4.5f;
    wh->collapsed = false;
    wh_entity->addComponent(std::move(wh));

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    persistence.deserializeWorld(&world2, json);

    auto* lss_entity = world2.getEntity("wh_j123456");
    assertTrue(lss_entity != nullptr, "SolarSystem entity loaded");
    auto* lss = lss_entity->getComponent<components::SolarSystem>();
    assertTrue(lss != nullptr, "SolarSystem component loaded");
    assertTrue(lss->system_id == "j123456", "System ID preserved");
    assertTrue(lss->wormhole_class == 3, "Wormhole class preserved");
    assertTrue(lss->effect_name == "magnetar", "Effect name preserved");
    assertTrue(lss->dormants_spawned == true, "Dormants spawned preserved");

    auto* lwh_entity = world2.getEntity("wh_conn_1");
    assertTrue(lwh_entity != nullptr, "WormholeConnection entity loaded");
    auto* lwh = lwh_entity->getComponent<components::WormholeConnection>();
    assertTrue(lwh != nullptr, "WormholeConnection component loaded");
    assertTrue(lwh->wormhole_id == "wh_001", "Wormhole ID preserved");
    assertTrue(lwh->remaining_mass > 749999999.0 && lwh->remaining_mass < 750000001.0,
               "Remaining mass preserved");
    assertTrue(approxEqual(lwh->elapsed_hours, 4.5f), "Elapsed hours preserved");
    assertTrue(lwh->collapsed == false, "Collapsed state preserved");
}

void testEmptyWorldSerialize() {
    std::cout << "\n=== Empty World Serialize ===" << std::endl;

    ecs::World world;
    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    assertTrue(!json.empty(), "Empty world produces valid JSON");
    assertTrue(json.find("entities") != std::string::npos, "JSON has entities key");

    ecs::World world2;
    bool ok = persistence.deserializeWorld(&world2, json);
    assertTrue(ok, "Deserialize empty world succeeds");
    assertTrue(world2.getEntityCount() == 0, "Empty world has 0 entities");
}

// ==================== Movement System & Collision Tests ====================

void testMovementBasicUpdate() {
    std::cout << "\n=== Movement Basic Update ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    auto* entity = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = 100.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    assertTrue(approxEqual(pos->x, 100.0f), "Position updated by velocity * dt");
    assertTrue(approxEqual(pos->y, 0.0f), "Y unchanged when vy = 0");
}

void testMovementSpeedLimit() {
    std::cout << "\n=== Movement Speed Limit ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    auto* entity = world.createEntity("ship2");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    pos->x = 0.0f;
    vel->vx = 500.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    float speed = std::sqrt(vel->vx * vel->vx + vel->vy * vel->vy + vel->vz * vel->vz);
    assertTrue(speed <= vel->max_speed + 0.01f, "Speed clamped to max_speed");
}

void testMovementCollisionZonePush() {
    std::cout << "\n=== Movement Collision Zone Push ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    // Set up a collision zone at origin (like a sun)
    std::vector<systems::MovementSystem::CollisionZone> zones;
    zones.push_back({0.0f, 0.0f, 0.0f, 500000.0f});  // 500km radius sun
    moveSys.setCollisionZones(zones);
    
    auto* entity = world.createEntity("ship3");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    // Place ship inside the sun's collision zone
    pos->x = 100000.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = -100.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    
    // After update, ship should be pushed outside the collision zone
    float dist = std::sqrt(pos->x * pos->x + pos->y * pos->y + pos->z * pos->z);
    assertTrue(dist >= 500000.0f, "Ship pushed outside collision zone (sun)");
}

void testMovementCollisionZoneVelocityKilled() {
    std::cout << "\n=== Movement Collision Zone Velocity Killed ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    std::vector<systems::MovementSystem::CollisionZone> zones;
    zones.push_back({0.0f, 0.0f, 0.0f, 500000.0f});
    moveSys.setCollisionZones(zones);
    
    auto* entity = world.createEntity("ship4");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    // Ship inside zone moving toward center
    pos->x = 100000.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = -200.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 300.0f;
    
    moveSys.update(1.0f);
    
    // Velocity toward the celestial should be killed
    assertTrue(vel->vx >= 0.0f, "Velocity toward celestial killed (bounce effect)");
}

void testMovementOutsideCollisionZoneUnaffected() {
    std::cout << "\n=== Movement Outside Collision Zone Unaffected ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    std::vector<systems::MovementSystem::CollisionZone> zones;
    zones.push_back({0.0f, 0.0f, 0.0f, 500000.0f});
    moveSys.setCollisionZones(zones);
    
    auto* entity = world.createEntity("ship5");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    // Ship well outside the collision zone
    pos->x = 1000000.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = 100.0f; vel->vy = 50.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    
    // Position should be updated normally (not pushed)
    assertTrue(approxEqual(pos->x, 1000100.0f), "Ship outside zone moves normally in X");
    assertTrue(approxEqual(pos->y, 50.0f), "Ship outside zone moves normally in Y");
}

void testMovementMultipleCollisionZones() {
    std::cout << "\n=== Movement Multiple Collision Zones ===" << std::endl;
    
    ecs::World world;
    systems::MovementSystem moveSys(&world);
    
    // Sun at origin, planet at 1M meters
    std::vector<systems::MovementSystem::CollisionZone> zones;
    zones.push_back({0.0f, 0.0f, 0.0f, 500000.0f});        // Sun
    zones.push_back({1000000.0f, 0.0f, 0.0f, 6000.0f});     // Planet
    moveSys.setCollisionZones(zones);
    
    auto* entity = world.createEntity("ship6");
    auto* pos = addComp<components::Position>(entity);
    auto* vel = addComp<components::Velocity>(entity);
    
    // Ship inside planet's collision zone
    pos->x = 999000.0f; pos->y = 0.0f; pos->z = 0.0f;
    vel->vx = 100.0f; vel->vy = 0.0f; vel->vz = 0.0f;
    vel->max_speed = 200.0f;
    
    moveSys.update(1.0f);
    
    // Ship should be pushed out of planet's collision zone
    float distToPlanet = std::sqrt((pos->x - 1000000.0f) * (pos->x - 1000000.0f) + 
                                    pos->y * pos->y + pos->z * pos->z);
    assertTrue(distToPlanet >= 6000.0f, "Ship pushed outside planet collision zone");
}

// ==================== Logger Tests ====================

void testLoggerLevels() {
    std::cout << "\n=== Logger Levels ===" << std::endl;
    
    auto& log = utils::Logger::instance();
    
    // Disable console output so tests don't clutter the terminal
    log.setConsoleOutput(false);
    
    log.setLevel(utils::LogLevel::INFO);
    assertTrue(log.getLevel() == utils::LogLevel::INFO, "Log level set to INFO");
    
    log.setLevel(utils::LogLevel::DEBUG);
    assertTrue(log.getLevel() == utils::LogLevel::DEBUG, "Log level set to DEBUG");
    
    log.setLevel(utils::LogLevel::ERROR);
    assertTrue(log.getLevel() == utils::LogLevel::ERROR, "Log level set to ERROR");
    
    log.setLevel(utils::LogLevel::WARN);
    assertTrue(log.getLevel() == utils::LogLevel::WARN, "Log level set to WARN");
    
    log.setLevel(utils::LogLevel::FATAL);
    assertTrue(log.getLevel() == utils::LogLevel::FATAL, "Log level set to FATAL");
    
    // Re-enable console output
    log.setConsoleOutput(true);
    // Reset to INFO for other tests
    log.setLevel(utils::LogLevel::INFO);
}

void testLoggerFileOutput() {
    std::cout << "\n=== Logger File Output ===" << std::endl;
    
    auto& log = utils::Logger::instance();
    log.setConsoleOutput(false);
    
    // Shut down any previously opened file
    log.shutdown();
    assertTrue(!log.isFileOpen(), "No file open after shutdown");
    
    // Init with a temp directory
    bool ok = log.init("/tmp/eve_test_logs");
    assertTrue(ok, "Logger init succeeds");
    assertTrue(log.isFileOpen(), "Log file is open after init");
    
    // Write some log entries
    log.setLevel(utils::LogLevel::DEBUG);
    log.debug("test debug message");
    log.info("test info message");
    log.warn("test warn message");
    log.error("test error message");
    
    log.shutdown();
    assertTrue(!log.isFileOpen(), "Log file closed after shutdown");
    
    // Verify the file was actually written
    std::ifstream f("/tmp/eve_test_logs/server.log");
    assertTrue(f.is_open(), "Log file exists on disk");
    
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assertTrue(content.find("[DEBUG]") != std::string::npos, "Log contains DEBUG entry");
    assertTrue(content.find("[INFO]") != std::string::npos, "Log contains INFO entry");
    assertTrue(content.find("[WARN]") != std::string::npos, "Log contains WARN entry");
    assertTrue(content.find("[ERROR]") != std::string::npos, "Log contains ERROR entry");
    assertTrue(content.find("test debug message") != std::string::npos, "Log contains debug text");
    assertTrue(content.find("test info message") != std::string::npos, "Log contains info text");
    f.close();
    
    // Clean up
    std::remove("/tmp/eve_test_logs/server.log");
    
    // Re-enable console
    log.setConsoleOutput(true);
    log.setLevel(utils::LogLevel::INFO);
}

void testLoggerLevelFiltering() {
    std::cout << "\n=== Logger Level Filtering ===" << std::endl;
    
    auto& log = utils::Logger::instance();
    log.setConsoleOutput(false);
    log.shutdown();
    
    bool ok = log.init("/tmp/eve_test_logs", "filter_test.log");
    assertTrue(ok, "Logger init for filter test succeeds");
    
    // Set level to WARN — DEBUG and INFO should be filtered out
    log.setLevel(utils::LogLevel::WARN);
    log.debug("should_not_appear_debug");
    log.info("should_not_appear_info");
    log.warn("should_appear_warn");
    log.error("should_appear_error");
    
    log.shutdown();
    
    std::ifstream f("/tmp/eve_test_logs/filter_test.log");
    assertTrue(f.is_open(), "Filter test log file exists");
    
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    assertTrue(content.find("should_not_appear_debug") == std::string::npos,
               "DEBUG filtered out at WARN level");
    assertTrue(content.find("should_not_appear_info") == std::string::npos,
               "INFO filtered out at WARN level");
    assertTrue(content.find("should_appear_warn") != std::string::npos,
               "WARN passes at WARN level");
    assertTrue(content.find("should_appear_error") != std::string::npos,
               "ERROR passes at WARN level");
    f.close();
    
    std::remove("/tmp/eve_test_logs/filter_test.log");
    log.setConsoleOutput(true);
    log.setLevel(utils::LogLevel::INFO);
}

// ==================== ServerMetrics Tests ====================

void testMetricsTickTiming() {
    std::cout << "\n=== Metrics Tick Timing ===" << std::endl;
    
    utils::ServerMetrics metrics;
    
    assertTrue(metrics.getTotalTicks() == 0, "No ticks recorded initially");
    assertTrue(metrics.getAvgTickMs() == 0.0, "Avg tick 0 with no data");
    assertTrue(metrics.getMaxTickMs() == 0.0, "Max tick 0 with no data");
    assertTrue(metrics.getMinTickMs() == 0.0, "Min tick 0 with no data");
    
    // Record a few ticks with a known sleep
    for (int i = 0; i < 5; ++i) {
        metrics.recordTickStart();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        metrics.recordTickEnd();
    }
    
    assertTrue(metrics.getTotalTicks() == 5, "5 ticks recorded");
    assertTrue(metrics.getAvgTickMs() >= 1.0, "Average tick >= 1ms");
    assertTrue(metrics.getMaxTickMs() >= 1.0, "Max tick >= 1ms");
    assertTrue(metrics.getMinTickMs() >= 1.0, "Min tick >= 1ms");
    assertTrue(metrics.getMaxTickMs() >= metrics.getMinTickMs(), "Max >= Min");
}

void testMetricsCounters() {
    std::cout << "\n=== Metrics Counters ===" << std::endl;
    
    utils::ServerMetrics metrics;
    
    assertTrue(metrics.getEntityCount() == 0, "Entity count starts at 0");
    assertTrue(metrics.getPlayerCount() == 0, "Player count starts at 0");
    
    metrics.setEntityCount(42);
    metrics.setPlayerCount(3);
    
    assertTrue(metrics.getEntityCount() == 42, "Entity count set to 42");
    assertTrue(metrics.getPlayerCount() == 3, "Player count set to 3");
}

void testMetricsUptime() {
    std::cout << "\n=== Metrics Uptime ===" << std::endl;
    
    utils::ServerMetrics metrics;
    
    assertTrue(metrics.getUptimeSeconds() >= 0.0, "Uptime is non-negative");
    
    std::string uptime = metrics.getUptimeString();
    assertTrue(!uptime.empty(), "Uptime string is not empty");
    assertTrue(uptime.find("d") != std::string::npos, "Uptime contains 'd'");
    assertTrue(uptime.find("h") != std::string::npos, "Uptime contains 'h'");
    assertTrue(uptime.find("m") != std::string::npos, "Uptime contains 'm'");
    assertTrue(uptime.find("s") != std::string::npos, "Uptime contains 's'");
}

void testMetricsSummary() {
    std::cout << "\n=== Metrics Summary ===" << std::endl;
    
    utils::ServerMetrics metrics;
    metrics.setEntityCount(10);
    metrics.setPlayerCount(2);
    
    metrics.recordTickStart();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    metrics.recordTickEnd();
    
    std::string s = metrics.summary();
    assertTrue(!s.empty(), "Summary is not empty");
    assertTrue(s.find("[Metrics]") != std::string::npos, "Summary contains [Metrics]");
    assertTrue(s.find("entities=10") != std::string::npos, "Summary contains entity count");
    assertTrue(s.find("players=2") != std::string::npos, "Summary contains player count");
    assertTrue(s.find("uptime") != std::string::npos, "Summary contains uptime");
    assertTrue(s.find("ticks=") != std::string::npos, "Summary contains tick count");
}

void testMetricsResetWindow() {
    std::cout << "\n=== Metrics Reset Window ===" << std::endl;
    
    utils::ServerMetrics metrics;
    
    // Record some ticks
    for (int i = 0; i < 3; ++i) {
        metrics.recordTickStart();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        metrics.recordTickEnd();
    }
    
    assertTrue(metrics.getTotalTicks() == 3, "3 ticks before reset");
    assertTrue(metrics.getAvgTickMs() > 0.0, "Avg > 0 before reset");
    
    metrics.resetWindow();
    
    // Total ticks should remain, but window stats reset
    assertTrue(metrics.getTotalTicks() == 3, "Total ticks preserved after reset");
    assertTrue(metrics.getAvgTickMs() == 0.0, "Avg reset to 0 after window reset");
    assertTrue(metrics.getMaxTickMs() == 0.0, "Max reset to 0 after window reset");
    assertTrue(metrics.getMinTickMs() == 0.0, "Min reset to 0 after window reset");
}

// ==================== Mission System Tests ====================

void testMissionAcceptAndComplete() {
    std::cout << "\n=== Mission Accept & Complete ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missionSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);
    auto* playerComp = addComp<components::Player>(player);
    playerComp->isk = 0.0;
    auto* standings = addComp<components::Standings>(player);

    // Accept a mission
    bool accepted = missionSys.acceptMission("player1", "mission_001",
        "Destroy Pirates", 1, "combat", "Veyren", 100000.0, 0.5f);
    assertTrue(accepted, "Mission accepted successfully");

    auto* tracker = player->getComponent<components::MissionTracker>();
    assertTrue(tracker->active_missions.size() == 1, "One active mission");

    // Add objective
    components::MissionTracker::Objective obj;
    obj.type = "destroy";
    obj.target = "pirate_frigate";
    obj.required = 3;
    obj.completed = 0;
    tracker->active_missions[0].objectives.push_back(obj);

    // Record partial progress
    missionSys.recordProgress("player1", "mission_001", "destroy", "pirate_frigate", 2);
    assertTrue(tracker->active_missions[0].objectives[0].completed == 2,
               "Partial progress recorded (2/3)");

    // Complete the objective
    missionSys.recordProgress("player1", "mission_001", "destroy", "pirate_frigate", 1);
    assertTrue(tracker->active_missions[0].objectives[0].done(),
               "Objective completed (3/3)");

    // Update should process completion
    missionSys.update(0.0f);
    assertTrue(approxEqual(static_cast<float>(playerComp->isk), 100000.0f, 1.0f),
               "ISK reward applied");
    assertTrue(tracker->completed_mission_ids.size() == 1,
               "Mission recorded as completed");
    assertTrue(tracker->active_missions.empty(),
               "Active missions cleared after completion");

    // Check standing was applied
    float standing = standings->faction_standings["Veyren"];
    assertTrue(approxEqual(standing, 0.5f), "Standing reward applied");
}

void testMissionTimeout() {
    std::cout << "\n=== Mission Timeout ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missionSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);
    addComp<components::Player>(player);

    // Accept a timed mission (30 second limit)
    missionSys.acceptMission("player1", "timed_001",
        "Timed Mission", 1, "combat", "Veyren", 50000.0, 0.1f, 30.0f);

    auto* tracker = player->getComponent<components::MissionTracker>();

    // Add an incomplete objective
    components::MissionTracker::Objective obj;
    obj.type = "destroy";
    obj.target = "enemy";
    obj.required = 5;
    tracker->active_missions[0].objectives.push_back(obj);

    // Update for 25 seconds (should still be active)
    missionSys.update(25.0f);
    assertTrue(tracker->active_missions.size() == 1, "Mission still active at 25s");

    // Update past the time limit
    missionSys.update(10.0f);
    assertTrue(tracker->active_missions.empty(), "Timed-out mission removed");
}

void testMissionAbandon() {
    std::cout << "\n=== Mission Abandon ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missionSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);

    missionSys.acceptMission("player1", "abandon_001",
        "Will Abandon", 1, "combat", "Faction", 10000.0, 0.1f);

    auto* tracker = player->getComponent<components::MissionTracker>();
    assertTrue(tracker->active_missions.size() == 1, "Mission active before abandon");

    missionSys.abandonMission("player1", "abandon_001");
    assertTrue(tracker->active_missions.empty(), "Mission removed after abandon");
}

void testMissionDuplicatePrevention() {
    std::cout << "\n=== Mission Duplicate Prevention ===" << std::endl;

    ecs::World world;
    systems::MissionSystem missionSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::MissionTracker>(player);

    bool first = missionSys.acceptMission("player1", "dup_001",
        "First", 1, "combat", "Faction", 10000.0, 0.1f);
    bool second = missionSys.acceptMission("player1", "dup_001",
        "Duplicate", 1, "combat", "Faction", 10000.0, 0.1f);

    assertTrue(first, "First accept succeeds");
    assertTrue(!second, "Duplicate accept rejected");
}

// ==================== Skill System Tests ====================

void testSkillTraining() {
    std::cout << "\n=== Skill Training ===" << std::endl;

    ecs::World world;
    systems::SkillSystem skillSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::SkillSet>(player);

    // Queue skill training
    bool queued = skillSys.queueSkillTraining("player1", "gunnery_001",
        "Small Projectile Turret", 1, 60.0f);
    assertTrue(queued, "Skill training queued");

    auto* skillset = player->getComponent<components::SkillSet>();
    assertTrue(skillset->training_queue.size() == 1, "One skill in queue");

    // Partially train
    skillSys.update(30.0f);
    assertTrue(skillSys.getSkillLevel("player1", "gunnery_001") == 0,
               "Skill not yet complete after 30s");

    // Complete training
    skillSys.update(35.0f);
    assertTrue(skillSys.getSkillLevel("player1", "gunnery_001") == 1,
               "Skill trained to level 1 after 65s");
    assertTrue(skillset->training_queue.empty(), "Queue empty after completion");
    assertTrue(skillset->total_sp > 0.0, "SP awarded");
}

void testSkillInstantTrain() {
    std::cout << "\n=== Skill Instant Train ===" << std::endl;

    ecs::World world;
    systems::SkillSystem skillSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::SkillSet>(player);

    bool trained = skillSys.trainSkillInstant("player1", "nav_001",
        "Navigation", 3);
    assertTrue(trained, "Instant train succeeds");
    assertTrue(skillSys.getSkillLevel("player1", "nav_001") == 3,
               "Skill is level 3");
}

void testSkillQueueMultiple() {
    std::cout << "\n=== Skill Queue Multiple ===" << std::endl;

    ecs::World world;
    systems::SkillSystem skillSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::SkillSet>(player);

    skillSys.queueSkillTraining("player1", "skill_a", "Skill A", 1, 10.0f);
    skillSys.queueSkillTraining("player1", "skill_b", "Skill B", 1, 20.0f);

    auto* skillset = player->getComponent<components::SkillSet>();
    assertTrue(skillset->training_queue.size() == 2, "Two skills in queue");

    // Complete first
    skillSys.update(12.0f);
    assertTrue(skillSys.getSkillLevel("player1", "skill_a") == 1, "First skill complete");
    assertTrue(skillset->training_queue.size() == 1, "One skill remaining");

    // Complete second
    skillSys.update(20.0f);
    assertTrue(skillSys.getSkillLevel("player1", "skill_b") == 1, "Second skill complete");
    assertTrue(skillset->training_queue.empty(), "Queue empty");
}

void testSkillInvalidLevel() {
    std::cout << "\n=== Skill Invalid Level ===" << std::endl;

    ecs::World world;
    systems::SkillSystem skillSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::SkillSet>(player);

    bool result = skillSys.queueSkillTraining("player1", "test", "Test", 6, 10.0f);
    assertTrue(!result, "Level 6 rejected (max is 5)");

    result = skillSys.queueSkillTraining("player1", "test", "Test", 0, 10.0f);
    assertTrue(!result, "Level 0 rejected (min is 1)");
}

// ==================== Module System Tests ====================

void testModuleActivation() {
    std::cout << "\n=== Module Activation ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* rack = addComp<components::ModuleRack>(ship);
    auto* cap = addComp<components::Capacitor>(ship);
    cap->capacitor = 100.0f;
    cap->capacitor_max = 100.0f;

    // Add a module to high slot
    components::ModuleRack::FittedModule gun;
    gun.module_id = "gun_001";
    gun.name = "125mm Autocannon";
    gun.slot_type = "high";
    gun.slot_index = 0;
    gun.cycle_time = 5.0f;
    gun.capacitor_cost = 10.0f;
    rack->high_slots.push_back(gun);

    // Activate
    bool activated = modSys.activateModule("ship1", "high", 0);
    assertTrue(activated, "Module activated");
    assertTrue(rack->high_slots[0].active, "Module is active");

    // Can't activate again
    bool double_activate = modSys.activateModule("ship1", "high", 0);
    assertTrue(!double_activate, "Can't activate already active module");
}

void testModuleCycling() {
    std::cout << "\n=== Module Cycling ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* rack = addComp<components::ModuleRack>(ship);
    auto* cap = addComp<components::Capacitor>(ship);
    cap->capacitor = 100.0f;
    cap->capacitor_max = 100.0f;

    components::ModuleRack::FittedModule repper;
    repper.module_id = "rep_001";
    repper.name = "Small Armor Repairer";
    repper.slot_type = "low";
    repper.slot_index = 0;
    repper.cycle_time = 4.0f;
    repper.capacitor_cost = 20.0f;
    rack->low_slots.push_back(repper);

    modSys.activateModule("ship1", "low", 0);

    // Partially cycle
    modSys.update(2.0f);
    assertTrue(approxEqual(rack->low_slots[0].cycle_progress, 0.5f),
               "Half cycle after 2s (4s cycle time)");

    // Complete cycle — should consume cap
    modSys.update(3.0f);
    assertTrue(approxEqual(cap->capacitor, 80.0f, 1.0f),
               "Capacitor consumed after cycle completion");
}

void testModuleCapDrain() {
    std::cout << "\n=== Module Capacitor Drain ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* rack = addComp<components::ModuleRack>(ship);
    auto* cap = addComp<components::Capacitor>(ship);
    cap->capacitor = 15.0f;  // Just enough for one cycle
    cap->capacitor_max = 100.0f;

    components::ModuleRack::FittedModule mod;
    mod.cycle_time = 1.0f;
    mod.capacitor_cost = 10.0f;
    rack->high_slots.push_back(mod);

    modSys.activateModule("ship1", "high", 0);

    // First cycle completes
    modSys.update(1.5f);
    assertTrue(rack->high_slots[0].active, "Module still active after first cycle");

    // Second cycle — not enough cap
    modSys.update(1.5f);
    assertTrue(!rack->high_slots[0].active,
               "Module deactivated when capacitor exhausted");
}

void testModuleFittingValidation() {
    std::cout << "\n=== Module Fitting Validation ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->cpu_max = 100.0f;
    shipComp->powergrid_max = 50.0f;
    auto* rack = addComp<components::ModuleRack>(ship);

    // Fit a module within limits
    components::ModuleRack::FittedModule mod1;
    mod1.cpu_usage = 30.0f;
    mod1.powergrid_usage = 20.0f;
    rack->high_slots.push_back(mod1);

    assertTrue(modSys.validateFitting("ship1"), "Fitting within limits");

    // Exceed CPU
    components::ModuleRack::FittedModule mod2;
    mod2.cpu_usage = 80.0f;
    mod2.powergrid_usage = 10.0f;
    rack->mid_slots.push_back(mod2);

    assertTrue(!modSys.validateFitting("ship1"), "Fitting exceeds CPU");
}

void testModuleToggle() {
    std::cout << "\n=== Module Toggle ===" << std::endl;

    ecs::World world;
    systems::ModuleSystem modSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* rack = addComp<components::ModuleRack>(ship);
    auto* cap = addComp<components::Capacitor>(ship);
    cap->capacitor = 100.0f;

    components::ModuleRack::FittedModule mod;
    mod.capacitor_cost = 5.0f;
    rack->mid_slots.push_back(mod);

    // Toggle on
    modSys.toggleModule("ship1", "mid", 0);
    assertTrue(rack->mid_slots[0].active, "Module toggled on");

    // Toggle off
    modSys.toggleModule("ship1", "mid", 0);
    assertTrue(!rack->mid_slots[0].active, "Module toggled off");
}

// ==================== Movement Command Tests ====================

void testMovementOrbitCommand() {
    std::cout << "\n=== Movement Orbit Command ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(ship);
    vel->max_speed = 200.0f;

    auto* target = world.createEntity("target1");
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 1000.0f; tpos->y = 0.0f; tpos->z = 0.0f;
    addComp<components::Velocity>(target);

    moveSys.commandOrbit("ship1", "target1", 500.0f);
    moveSys.update(1.0f);

    // Ship should be moving (velocity non-zero)
    float speed = std::sqrt(vel->vx * vel->vx + vel->vy * vel->vy + vel->vz * vel->vz);
    assertTrue(speed > 0.0f, "Ship has velocity after orbit command");
}

void testMovementApproachCommand() {
    std::cout << "\n=== Movement Approach Command ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(ship);
    vel->max_speed = 200.0f;

    auto* target = world.createEntity("target1");
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 1000.0f; tpos->y = 0.0f; tpos->z = 0.0f;
    addComp<components::Velocity>(target);

    moveSys.commandApproach("ship1", "target1");
    moveSys.update(1.0f);

    // Ship should be moving toward target (positive vx)
    assertTrue(vel->vx > 0.0f, "Ship moving toward target (positive X)");
    assertTrue(pos->x > 0.0f, "Ship position moved toward target");
}

void testMovementStopCommand() {
    std::cout << "\n=== Movement Stop Command ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    addComp<components::Position>(ship);
    auto* vel = addComp<components::Velocity>(ship);
    vel->vx = 100.0f;
    vel->vy = 50.0f;
    vel->max_speed = 200.0f;

    moveSys.commandStop("ship1");
    assertTrue(vel->vx == 0.0f && vel->vy == 0.0f && vel->vz == 0.0f,
               "Velocity zeroed after stop command");
}

void testMovementWarpDistance() {
    std::cout << "\n=== Movement Warp Distance Check ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);

    // Try to warp too close (< 150km)
    bool warped = moveSys.commandWarp("ship1", 100.0f, 0.0f, 0.0f);
    assertTrue(!warped, "Warp rejected (destination too close)");

    // Warp to valid distance
    warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(warped, "Warp accepted (>150km)");
}

// ==================== Inventory System Tests ====================

void testInventoryAddItem() {
    std::cout << "\n=== Inventory Add Item ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 100.0f;

    bool added = invSys.addItem("ship1", "tritanium", "Tritanium", "ore", 10, 1.0f);
    assertTrue(added, "Item added successfully");
    assertTrue(inv->items.size() == 1, "One item stack in inventory");
    assertTrue(inv->items[0].quantity == 10, "Quantity is 10");
    assertTrue(approxEqual(inv->usedCapacity(), 10.0f), "Used capacity is 10 m3");

    // Stack with existing
    added = invSys.addItem("ship1", "tritanium", "Tritanium", "ore", 5, 1.0f);
    assertTrue(added, "Stacked item added");
    assertTrue(inv->items.size() == 1, "Still one stack after stacking");
    assertTrue(inv->items[0].quantity == 15, "Quantity is 15 after stacking");
}

void testInventoryCapacityLimit() {
    std::cout << "\n=== Inventory Capacity Limit ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 50.0f;

    bool added = invSys.addItem("ship1", "ore", "Veldspar", "ore", 40, 1.0f);
    assertTrue(added, "40 m3 fits in 50 m3 hold");

    added = invSys.addItem("ship1", "big_item", "Big Module", "module", 1, 20.0f);
    assertTrue(!added, "20 m3 item rejected (only 10 m3 free)");
    assertTrue(approxEqual(inv->freeCapacity(), 10.0f), "Free capacity is 10 m3");
}

void testInventoryRemoveItem() {
    std::cout << "\n=== Inventory Remove Item ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 400.0f;

    invSys.addItem("ship1", "ammo", "Hybrid Charges", "ammo", 100, 0.01f);

    int removed = invSys.removeItem("ship1", "ammo", 30);
    assertTrue(removed == 30, "Removed 30 units");
    assertTrue(invSys.getItemCount("ship1", "ammo") == 70, "70 remaining");

    removed = invSys.removeItem("ship1", "ammo", 200);
    assertTrue(removed == 70, "Removed only 70 (all available)");
    assertTrue(inv->items.empty(), "Item stack removed when depleted");
}

void testInventoryTransfer() {
    std::cout << "\n=== Inventory Transfer ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship1 = world.createEntity("ship1");
    auto* inv1 = addComp<components::Inventory>(ship1);
    inv1->max_capacity = 400.0f;

    auto* ship2 = world.createEntity("ship2");
    auto* inv2 = addComp<components::Inventory>(ship2);
    inv2->max_capacity = 400.0f;

    invSys.addItem("ship1", "salvage", "Armor Plates", "salvage", 20, 2.0f);

    bool transferred = invSys.transferItem("ship1", "ship2", "salvage", 10);
    assertTrue(transferred, "Transfer succeeded");
    assertTrue(invSys.getItemCount("ship1", "salvage") == 10, "Source has 10 left");
    assertTrue(invSys.getItemCount("ship2", "salvage") == 10, "Destination has 10");

    // Transfer fails if source lacks quantity
    transferred = invSys.transferItem("ship1", "ship2", "nonexistent", 5);
    assertTrue(!transferred, "Transfer fails for missing item");
}

void testInventoryHasItem() {
    std::cout << "\n=== Inventory HasItem ===" << std::endl;

    ecs::World world;
    systems::InventorySystem invSys(&world);

    auto* ship = world.createEntity("ship1");
    addComp<components::Inventory>(ship);

    invSys.addItem("ship1", "dogtag", "Pirate Dogtag", "commodity", 5, 0.1f);

    assertTrue(invSys.hasItem("ship1", "dogtag", 3), "Has 3 dogtags (has 5)");
    assertTrue(invSys.hasItem("ship1", "dogtag", 5), "Has exactly 5 dogtags");
    assertTrue(!invSys.hasItem("ship1", "dogtag", 6), "Does not have 6 dogtags");
    assertTrue(!invSys.hasItem("ship1", "nope"), "Does not have nonexistent item");
}

// ==================== Loot System Tests ====================

void testLootGenerate() {
    std::cout << "\n=== Loot Generate ===" << std::endl;

    ecs::World world;
    systems::LootSystem lootSys(&world);
    lootSys.setRandomSeed(42);

    auto* npc = world.createEntity("pirate1");
    auto* lt = addComp<components::LootTable>(npc);
    lt->isk_drop = 15000.0;

    components::LootTable::LootEntry entry1;
    entry1.item_id     = "scrap_metal";
    entry1.name        = "Scrap Metal";
    entry1.type        = "salvage";
    entry1.drop_chance = 1.0f;  // always drops
    entry1.min_quantity = 1;
    entry1.max_quantity = 5;
    entry1.volume      = 1.0f;
    lt->entries.push_back(entry1);

    components::LootTable::LootEntry entry2;
    entry2.item_id     = "rare_module";
    entry2.name        = "Rare Module";
    entry2.type        = "module";
    entry2.drop_chance = 1.0f;  // always drops for testing
    entry2.min_quantity = 1;
    entry2.max_quantity = 1;
    entry2.volume      = 5.0f;
    lt->entries.push_back(entry2);

    std::string wreck_id = lootSys.generateLoot("pirate1");
    assertTrue(!wreck_id.empty(), "Wreck entity created");

    auto* wreck = world.getEntity(wreck_id);
    assertTrue(wreck != nullptr, "Wreck entity exists in world");

    auto* wreck_inv = wreck->getComponent<components::Inventory>();
    assertTrue(wreck_inv != nullptr, "Wreck has Inventory component");
    assertTrue(wreck_inv->items.size() >= 1, "Wreck has at least one item");

    auto* wreck_lt = wreck->getComponent<components::LootTable>();
    assertTrue(wreck_lt != nullptr, "Wreck has LootTable for ISK");
    assertTrue(approxEqual(static_cast<float>(wreck_lt->isk_drop), 15000.0f),
               "ISK bounty preserved on wreck");
}

void testLootCollect() {
    std::cout << "\n=== Loot Collect ===" << std::endl;

    ecs::World world;
    systems::LootSystem lootSys(&world);
    lootSys.setRandomSeed(42);

    // Create NPC with loot
    auto* npc = world.createEntity("pirate2");
    auto* lt = addComp<components::LootTable>(npc);
    lt->isk_drop = 25000.0;

    components::LootTable::LootEntry entry;
    entry.item_id     = "hybrid_charges";
    entry.name        = "Hybrid Charges";
    entry.type        = "ammo";
    entry.drop_chance = 1.0f;
    entry.min_quantity = 10;
    entry.max_quantity = 10;
    entry.volume      = 0.01f;
    lt->entries.push_back(entry);

    std::string wreck_id = lootSys.generateLoot("pirate2");

    // Create player
    auto* player = world.createEntity("player1");
    auto* player_inv = addComp<components::Inventory>(player);
    player_inv->max_capacity = 400.0f;
    auto* player_comp = addComp<components::Player>(player);
    player_comp->isk = 100000.0;

    bool collected = lootSys.collectLoot(wreck_id, "player1");
    assertTrue(collected, "Loot collected successfully");
    assertTrue(player_inv->items.size() >= 1, "Player received items");
    assertTrue(approxEqual(static_cast<float>(player_comp->isk), 125000.0f),
               "Player ISK increased by bounty");
}

void testLootEmptyTable() {
    std::cout << "\n=== Loot Empty Table ===" << std::endl;

    ecs::World world;
    systems::LootSystem lootSys(&world);
    lootSys.setRandomSeed(99);

    auto* npc = world.createEntity("pirate3");
    auto* lt = addComp<components::LootTable>(npc);
    lt->isk_drop = 0.0;
    // No entries

    std::string wreck_id = lootSys.generateLoot("pirate3");
    assertTrue(!wreck_id.empty(), "Wreck created even with empty loot table");

    auto* wreck = world.getEntity(wreck_id);
    auto* wreck_inv = wreck->getComponent<components::Inventory>();
    assertTrue(wreck_inv->items.empty(), "Wreck has no items from empty table");
}

// ==================== NpcDatabase Tests ====================

void testNpcDatabaseLoad() {
    std::cout << "\n=== NpcDatabase Load ===" << std::endl;

    data::NpcDatabase npcDb;

    // Try multiple paths (same strategy as ShipDatabase tests)
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    assertTrue(loaded > 0, "NpcDatabase loaded NPCs from directory");
    assertTrue(npcDb.getNpcCount() >= 32, "At least 32 NPC templates loaded");
}

void testNpcDatabaseGetNpc() {
    std::cout << "\n=== NpcDatabase GetNpc ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    const data::NpcTemplate* scout = npcDb.getNpc("venom_syndicate_scout");
    assertTrue(scout != nullptr, "venom_syndicate_scout found");
    assertTrue(scout->name == "Venom Syndicate Scout", "NPC name correct");
    assertTrue(scout->type == "frigate", "NPC type correct");
    assertTrue(scout->faction == "Venom Syndicate", "NPC faction correct");
}

void testNpcDatabaseHpValues() {
    std::cout << "\n=== NpcDatabase HP Values ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    const data::NpcTemplate* scout = npcDb.getNpc("venom_syndicate_scout");
    assertTrue(scout != nullptr, "Scout found for HP test");
    assertTrue(approxEqual(scout->hull_hp, 300.0f), "Hull HP is 300");
    assertTrue(approxEqual(scout->armor_hp, 250.0f), "Armor HP is 250");
    assertTrue(approxEqual(scout->shield_hp, 350.0f), "Shield HP is 350");
    assertTrue(approxEqual(static_cast<float>(scout->bounty), 12500.0f), "Bounty is 12500");
}

void testNpcDatabaseWeapons() {
    std::cout << "\n=== NpcDatabase Weapons ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    const data::NpcTemplate* scout = npcDb.getNpc("venom_syndicate_scout");
    assertTrue(scout != nullptr, "Scout found for weapons test");
    assertTrue(!scout->weapons.empty(), "Scout has weapons");
    assertTrue(scout->weapons[0].type == "small_hybrid", "Weapon type is small_hybrid");
    assertTrue(approxEqual(scout->weapons[0].damage, 28.0f), "Weapon damage is 28");
    assertTrue(scout->weapons[0].damage_type == "kinetic", "Weapon damage type is kinetic");
    assertTrue(approxEqual(scout->weapons[0].rate_of_fire, 4.5f), "Rate of fire is 4.5");
}

void testNpcDatabaseResistances() {
    std::cout << "\n=== NpcDatabase Resistances ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    const data::NpcTemplate* scout = npcDb.getNpc("venom_syndicate_scout");
    assertTrue(scout != nullptr, "Scout found for resistances test");

    // Shield: em=0, thermal=60, kinetic=85, explosive=50 -> /100
    assertTrue(approxEqual(scout->shield_resists.em, 0.0f), "Shield EM resist is 0.0");
    assertTrue(approxEqual(scout->shield_resists.thermal, 0.60f), "Shield thermal resist is 0.60");
    assertTrue(approxEqual(scout->shield_resists.kinetic, 0.85f), "Shield kinetic resist is 0.85");
    assertTrue(approxEqual(scout->shield_resists.explosive, 0.50f), "Shield explosive resist is 0.50");

    // Armor: em=10, thermal=35, kinetic=25, explosive=45 -> /100
    assertTrue(approxEqual(scout->armor_resists.em, 0.10f), "Armor EM resist is 0.10");
    assertTrue(approxEqual(scout->armor_resists.kinetic, 0.25f), "Armor kinetic resist is 0.25");
}

void testNpcDatabaseIds() {
    std::cout << "\n=== NpcDatabase IDs ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    auto ids = npcDb.getNpcIds();
    assertTrue(!ids.empty(), "getNpcIds returns non-empty list");
    assertTrue(ids.size() == npcDb.getNpcCount(), "IDs count matches getNpcCount");
}

void testNpcDatabaseNonexistent() {
    std::cout << "\n=== NpcDatabase Nonexistent ===" << std::endl;

    data::NpcDatabase npcDb;
    npcDb.loadFromDirectory("../data");

    const data::NpcTemplate* result = npcDb.getNpc("totally_fake_npc");
    assertTrue(result == nullptr, "Nonexistent NPC returns nullptr");
}

// ==================== DroneSystem Tests ====================

void testDroneLaunch() {
    std::cout << "\n=== Drone Launch ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);
    bay->bay_capacity = 25.0f;
    bay->max_bandwidth = 25;

    components::DroneBay::DroneInfo d;
    d.drone_id = "hobgoblin"; d.name = "Hobgoblin I";
    d.type = "light_combat_drone"; d.damage_type = "thermal";
    d.damage = 25.0f; d.rate_of_fire = 3.0f; d.optimal_range = 5000.0f;
    d.hitpoints = 45.0f; d.current_hp = 45.0f; d.bandwidth_use = 5; d.volume = 5.0f;
    bay->stored_drones.push_back(d);

    assertTrue(droneSys.launchDrone("player_ship", "hobgoblin"),
               "Drone launched successfully");
    assertTrue(bay->deployed_drones.size() == 1, "One drone deployed");
    assertTrue(bay->stored_drones.empty(), "Bay empty after launch");
    assertTrue(droneSys.getDeployedCount("player_ship") == 1, "getDeployedCount returns 1");
}

void testDroneRecall() {
    std::cout << "\n=== Drone Recall ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);

    components::DroneBay::DroneInfo d;
    d.drone_id = "warrior"; d.name = "Warrior I";
    d.type = "light_combat_drone"; d.damage_type = "explosive";
    d.damage = 22.0f; d.bandwidth_use = 5; d.volume = 5.0f;
    d.hitpoints = 38.0f; d.current_hp = 38.0f;
    bay->stored_drones.push_back(d);

    droneSys.launchDrone("player_ship", "warrior");
    assertTrue(bay->deployed_drones.size() == 1, "Drone deployed before recall");

    assertTrue(droneSys.recallDrone("player_ship", "warrior"),
               "Drone recalled successfully");
    assertTrue(bay->deployed_drones.empty(), "No deployed drones after recall");
    assertTrue(bay->stored_drones.size() == 1, "Drone back in bay");
}

void testDroneRecallAll() {
    std::cout << "\n=== Drone Recall All ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);
    bay->max_bandwidth = 25;

    // Add 3 drones
    for (int i = 0; i < 3; ++i) {
        components::DroneBay::DroneInfo d;
        d.drone_id = "drone_" + std::to_string(i);
        d.name = "Test Drone " + std::to_string(i);
        d.type = "light_combat_drone"; d.damage_type = "thermal";
        d.damage = 10.0f; d.bandwidth_use = 5; d.volume = 5.0f;
        d.hitpoints = 40.0f; d.current_hp = 40.0f;
        bay->stored_drones.push_back(d);
    }

    // Launch all 3
    droneSys.launchDrone("player_ship", "drone_0");
    droneSys.launchDrone("player_ship", "drone_1");
    droneSys.launchDrone("player_ship", "drone_2");
    assertTrue(bay->deployed_drones.size() == 3, "3 drones deployed");

    int recalled = droneSys.recallAll("player_ship");
    assertTrue(recalled == 3, "recallAll returns 3");
    assertTrue(bay->deployed_drones.empty(), "No deployed drones after recallAll");
    assertTrue(bay->stored_drones.size() == 3, "All drones back in bay");
}

void testDroneBandwidthLimit() {
    std::cout << "\n=== Drone Bandwidth Limit ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);
    bay->max_bandwidth = 10;  // Only 10 Mbit/s

    // Add two drones each using 5 bandwidth (exactly max), then a third
    for (int i = 0; i < 3; ++i) {
        components::DroneBay::DroneInfo d;
        d.drone_id = "drone_" + std::to_string(i);
        d.name = "Test Drone " + std::to_string(i);
        d.type = "light_combat_drone"; d.damage_type = "kinetic";
        d.damage = 10.0f; d.bandwidth_use = 5; d.volume = 5.0f;
        d.hitpoints = 40.0f; d.current_hp = 40.0f;
        bay->stored_drones.push_back(d);
    }

    assertTrue(droneSys.launchDrone("player_ship", "drone_0"), "First drone fits bandwidth");
    assertTrue(droneSys.launchDrone("player_ship", "drone_1"), "Second drone fits bandwidth");
    assertTrue(!droneSys.launchDrone("player_ship", "drone_2"),
               "Third drone exceeds bandwidth limit");
    assertTrue(bay->deployed_drones.size() == 2, "Only 2 drones deployed");
    assertTrue(bay->stored_drones.size() == 1, "One drone remains in bay");
}

void testDroneCombatUpdate() {
    std::cout << "\n=== Drone Combat Update ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    // Create player ship with drone
    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);
    auto* target_comp = addComp<components::Target>(ship);

    components::DroneBay::DroneInfo d;
    d.drone_id = "hobgoblin"; d.name = "Hobgoblin I";
    d.type = "light_combat_drone"; d.damage_type = "thermal";
    d.damage = 25.0f; d.rate_of_fire = 3.0f; d.optimal_range = 5000.0f;
    d.hitpoints = 45.0f; d.current_hp = 45.0f; d.bandwidth_use = 5;
    bay->stored_drones.push_back(d);
    droneSys.launchDrone("player_ship", "hobgoblin");

    // Create target NPC
    auto* npc = world.createEntity("npc_target");
    auto* hp = addComp<components::Health>(npc);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 100.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;

    // Lock the target
    target_comp->locked_targets.push_back("npc_target");

    // First tick: drone fires (cooldown == 0 initially)
    droneSys.update(0.1f);
    assertTrue(hp->shield_hp < 100.0f, "Drone dealt damage to shields");
    float shield_after = hp->shield_hp;

    // Second tick: drone is on cooldown, no additional damage
    droneSys.update(0.1f);
    assertTrue(approxEqual(hp->shield_hp, shield_after),
               "Drone on cooldown, no additional damage");

    // Wait out the cooldown (3.0 seconds)
    droneSys.update(3.0f);
    // Cooldown just expired this tick; drone fires on next update
    droneSys.update(0.01f);
    assertTrue(hp->shield_hp < shield_after, "Drone fires again after cooldown");
}

void testDroneDestroyedRemoval() {
    std::cout << "\n=== Drone Destroyed Removal ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);

    auto* ship = world.createEntity("player_ship");
    auto* bay = addComp<components::DroneBay>(ship);

    components::DroneBay::DroneInfo d;
    d.drone_id = "hobgoblin"; d.name = "Hobgoblin I";
    d.type = "light_combat_drone"; d.damage_type = "thermal";
    d.damage = 25.0f; d.bandwidth_use = 5; d.volume = 5.0f;
    d.hitpoints = 45.0f; d.current_hp = 45.0f;
    bay->stored_drones.push_back(d);
    droneSys.launchDrone("player_ship", "hobgoblin");
    assertTrue(bay->deployed_drones.size() == 1, "Drone deployed");

    // Simulate drone being destroyed
    bay->deployed_drones[0].current_hp = 0.0f;

    droneSys.update(1.0f);
    assertTrue(bay->deployed_drones.empty(), "Destroyed drone removed from deployed list");
}

void testSerializeDeserializeDroneBay() {
    std::cout << "\n=== Serialize/Deserialize DroneBay ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("drone_ship");
    auto* bay = addComp<components::DroneBay>(entity);
    bay->bay_capacity = 50.0f;
    bay->max_bandwidth = 50;

    // Add stored drone
    components::DroneBay::DroneInfo stored;
    stored.drone_id = "ogre"; stored.name = "Ogre I";
    stored.type = "heavy_combat_drone"; stored.damage_type = "thermal";
    stored.damage = 55.0f; stored.rate_of_fire = 6.0f;
    stored.optimal_range = 3000.0f; stored.hitpoints = 120.0f;
    stored.current_hp = 120.0f; stored.bandwidth_use = 25; stored.volume = 25.0f;
    bay->stored_drones.push_back(stored);

    // Add deployed drone
    components::DroneBay::DroneInfo deployed;
    deployed.drone_id = "hobgoblin"; deployed.name = "Hobgoblin I";
    deployed.type = "light_combat_drone"; deployed.damage_type = "thermal";
    deployed.damage = 25.0f; deployed.rate_of_fire = 3.0f;
    deployed.optimal_range = 5000.0f; deployed.hitpoints = 45.0f;
    deployed.current_hp = 30.0f; deployed.bandwidth_use = 5; deployed.volume = 5.0f;
    bay->deployed_drones.push_back(deployed);

    // Serialize
    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    // Deserialize into new world
    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "DroneBay deserialization succeeds");

    auto* e2 = world2.getEntity("drone_ship");
    assertTrue(e2 != nullptr, "Entity recreated");

    auto* bay2 = e2->getComponent<components::DroneBay>();
    assertTrue(bay2 != nullptr, "DroneBay component recreated");
    assertTrue(approxEqual(bay2->bay_capacity, 50.0f), "bay_capacity preserved");
    assertTrue(bay2->max_bandwidth == 50, "max_bandwidth preserved");
    assertTrue(bay2->stored_drones.size() == 1, "One stored drone");
    assertTrue(bay2->stored_drones[0].drone_id == "ogre", "Stored drone id preserved");
    assertTrue(approxEqual(bay2->stored_drones[0].damage, 55.0f), "Stored drone damage preserved");
    assertTrue(bay2->deployed_drones.size() == 1, "One deployed drone");
    assertTrue(bay2->deployed_drones[0].drone_id == "hobgoblin", "Deployed drone id preserved");
    assertTrue(approxEqual(bay2->deployed_drones[0].current_hp, 30.0f), "Deployed drone current_hp preserved");
}

// ==================== Insurance System Tests ====================

void testInsurancePurchase() {
    std::cout << "\n=== Insurance Purchase ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->isk = 1000000.0;

    assertTrue(insSys.purchaseInsurance("player_ship", "basic", 500000.0),
               "Basic insurance purchased");
    auto* policy = ship->getComponent<components::InsurancePolicy>();
    assertTrue(policy != nullptr, "InsurancePolicy component created");
    assertTrue(policy->tier == "basic", "Policy tier is basic");
    assertTrue(approxEqual(static_cast<float>(policy->coverage_fraction), 0.5f), "Basic coverage is 50%");
    assertTrue(approxEqual(static_cast<float>(policy->payout_value), 250000.0f), "Payout is 50% of ship value");
    assertTrue(player->isk < 1000000.0, "Premium deducted from ISK");
    assertTrue(policy->active, "Policy is active");
}

void testInsuranceClaim() {
    std::cout << "\n=== Insurance Claim ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->isk = 1000000.0;

    insSys.purchaseInsurance("player_ship", "standard", 500000.0);
    double isk_after_purchase = player->isk;

    double payout = insSys.claimInsurance("player_ship");
    assertTrue(payout > 0.0, "Claim returns positive payout");
    assertTrue(approxEqual(static_cast<float>(payout), 350000.0f), "Standard pays 70% of ship value");
    assertTrue(approxEqual(static_cast<float>(player->isk), static_cast<float>(isk_after_purchase + payout)),
               "ISK increased by payout");

    auto* policy = ship->getComponent<components::InsurancePolicy>();
    assertTrue(policy->claimed, "Policy marked as claimed");

    double second_claim = insSys.claimInsurance("player_ship");
    assertTrue(approxEqual(static_cast<float>(second_claim), 0.0f), "Double claim returns 0");
}

void testInsurancePlatinum() {
    std::cout << "\n=== Insurance Platinum ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->isk = 1000000.0;

    assertTrue(insSys.purchaseInsurance("player_ship", "platinum", 500000.0),
               "Platinum insurance purchased");
    auto* policy = ship->getComponent<components::InsurancePolicy>();
    assertTrue(approxEqual(static_cast<float>(policy->coverage_fraction), 1.0f), "Platinum coverage is 100%");
    assertTrue(approxEqual(static_cast<float>(policy->payout_value), 500000.0f), "Platinum payout is full value");
}

void testInsuranceExpiry() {
    std::cout << "\n=== Insurance Expiry ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->isk = 1000000.0;

    insSys.purchaseInsurance("player_ship", "basic", 500000.0);
    auto* policy = ship->getComponent<components::InsurancePolicy>();
    policy->duration_remaining = 10.0f; // 10 seconds

    insSys.update(5.0f);
    assertTrue(policy->active, "Policy still active at 5s");
    assertTrue(insSys.hasActivePolicy("player_ship"), "hasActivePolicy returns true");

    insSys.update(6.0f);
    assertTrue(!policy->active, "Policy expired after 11s");
    assertTrue(!insSys.hasActivePolicy("player_ship"), "hasActivePolicy returns false after expiry");
}

void testInsuranceInsufficientFunds() {
    std::cout << "\n=== Insurance Insufficient Funds ===" << std::endl;
    ecs::World world;
    systems::InsuranceSystem insSys(&world);
    auto* ship = world.createEntity("player_ship");
    auto* player = addComp<components::Player>(ship);
    player->isk = 100.0; // Not enough

    assertTrue(!insSys.purchaseInsurance("player_ship", "basic", 500000.0),
               "Insurance rejected with insufficient funds");
    assertTrue(ship->getComponent<components::InsurancePolicy>() == nullptr,
               "No policy created on failure");
}

// ==================== BountySystem Tests ====================

void testBountyProcessKill() {
    std::cout << "\n=== Bounty Process Kill ===" << std::endl;
    ecs::World world;
    systems::BountySystem bountySys(&world);
    
    auto* player = world.createEntity("player_1");
    auto* pc = addComp<components::Player>(player);
    pc->isk = 100000.0;
    
    double bounty = bountySys.processKill("player_1", "npc_pirate_1", "Venom Scout", 12500.0, "Venom Syndicate");
    assertTrue(approxEqual(static_cast<float>(bounty), 12500.0f), "Bounty returned correctly");
    assertTrue(approxEqual(static_cast<float>(pc->isk), 112500.0f), "ISK increased by bounty");
    assertTrue(bountySys.getTotalKills("player_1") == 1, "Kill count is 1");
    assertTrue(approxEqual(static_cast<float>(bountySys.getTotalBounty("player_1")), 12500.0f), "Total bounty correct");
}

void testBountyMultipleKills() {
    std::cout << "\n=== Bounty Multiple Kills ===" << std::endl;
    ecs::World world;
    systems::BountySystem bountySys(&world);
    
    auto* player = world.createEntity("player_1");
    auto* pc = addComp<components::Player>(player);
    pc->isk = 0.0;
    
    bountySys.processKill("player_1", "npc_1", "Scout", 10000.0);
    bountySys.processKill("player_1", "npc_2", "Cruiser", 50000.0);
    bountySys.processKill("player_1", "npc_3", "Battleship", 150000.0);
    
    assertTrue(bountySys.getTotalKills("player_1") == 3, "3 kills recorded");
    assertTrue(approxEqual(static_cast<float>(bountySys.getTotalBounty("player_1")), 210000.0f), "Total bounty is 210K");
    assertTrue(approxEqual(static_cast<float>(pc->isk), 210000.0f), "ISK matches total bounty");
}

void testBountyLedgerRecordLimit() {
    std::cout << "\n=== Bounty Ledger Record Limit ===" << std::endl;
    ecs::World world;
    systems::BountySystem bountySys(&world);
    
    auto* player = world.createEntity("player_1");
    addComp<components::Player>(player);
    
    for (int i = 0; i < 60; ++i) {
        bountySys.processKill("player_1", "npc_" + std::to_string(i), "NPC " + std::to_string(i), 1000.0);
    }
    
    auto* ledger = player->getComponent<components::BountyLedger>();
    assertTrue(ledger != nullptr, "Ledger exists");
    assertTrue(static_cast<int>(ledger->recent_kills.size()) <= components::BountyLedger::MAX_RECENT,
               "Recent kills capped at MAX_RECENT");
    assertTrue(ledger->total_kills == 60, "Total kills tracks all 60");
}

void testBountyNonexistentPlayer() {
    std::cout << "\n=== Bounty Nonexistent Player ===" << std::endl;
    ecs::World world;
    systems::BountySystem bountySys(&world);
    
    double bounty = bountySys.processKill("fake_player", "npc_1", "Scout", 10000.0);
    assertTrue(approxEqual(static_cast<float>(bounty), 0.0f), "No bounty for nonexistent player");
    assertTrue(bountySys.getTotalKills("fake_player") == 0, "Zero kills for nonexistent");
    assertTrue(approxEqual(static_cast<float>(bountySys.getTotalBounty("fake_player")), 0.0f), "Zero bounty for nonexistent");
}

// ==================== MarketSystem Tests ====================

void testMarketPlaceSellOrder() {
    std::cout << "\n=== Market Place Sell Order ===" << std::endl;
    ecs::World world;
    systems::MarketSystem marketSys(&world);

    auto* station = world.createEntity("station_1");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "station_1";

    auto* seller = world.createEntity("seller_1");
    auto* pc = addComp<components::Player>(seller);
    pc->isk = 100000.0;

    std::string oid = marketSys.placeSellOrder("station_1", "seller_1", "tritanium", "Tritanium", 100, 5.0);
    assertTrue(!oid.empty(), "Sell order created");
    assertTrue(marketSys.getOrderCount("station_1") == 1, "One order on station");
    assertTrue(pc->isk < 100000.0, "Broker fee deducted from seller");
}

void testMarketBuyFromMarket() {
    std::cout << "\n=== Market Buy From Market ===" << std::endl;
    ecs::World world;
    systems::MarketSystem marketSys(&world);

    auto* station = world.createEntity("station_1");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "station_1";

    auto* seller = world.createEntity("seller_1");
    auto* seller_pc = addComp<components::Player>(seller);
    seller_pc->isk = 100000.0;

    auto* buyer = world.createEntity("buyer_1");
    auto* buyer_pc = addComp<components::Player>(buyer);
    buyer_pc->isk = 100000.0;

    marketSys.placeSellOrder("station_1", "seller_1", "tritanium", "Tritanium", 100, 5.0);

    int bought = marketSys.buyFromMarket("station_1", "buyer_1", "tritanium", 50);
    assertTrue(bought == 50, "Bought 50 units");
    assertTrue(buyer_pc->isk < 100000.0, "Buyer ISK decreased");
    assertTrue(seller_pc->isk > 100000.0 - 100000.0 * 0.02, "Seller ISK increased from sale");
}

void testMarketPriceQueries() {
    std::cout << "\n=== Market Price Queries ===" << std::endl;
    ecs::World world;
    systems::MarketSystem marketSys(&world);

    auto* station = world.createEntity("station_1");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "station_1";

    auto* seller1 = world.createEntity("seller_1");
    auto* pc1 = addComp<components::Player>(seller1);
    pc1->isk = 1000000.0;

    auto* seller2 = world.createEntity("seller_2");
    auto* pc2 = addComp<components::Player>(seller2);
    pc2->isk = 1000000.0;

    auto* buyer1 = world.createEntity("buyer_1");
    auto* bpc = addComp<components::Player>(buyer1);
    bpc->isk = 1000000.0;

    marketSys.placeSellOrder("station_1", "seller_1", "tritanium", "Tritanium", 100, 5.0);
    marketSys.placeSellOrder("station_1", "seller_2", "tritanium", "Tritanium", 50, 4.5);
    marketSys.placeBuyOrder("station_1", "buyer_1", "tritanium", "Tritanium", 200, 4.0);

    double lowest = marketSys.getLowestSellPrice("station_1", "tritanium");
    assertTrue(approxEqual(static_cast<float>(lowest), 4.5f), "Lowest sell is 4.5");

    double highest = marketSys.getHighestBuyPrice("station_1", "tritanium");
    assertTrue(approxEqual(static_cast<float>(highest), 4.0f), "Highest buy is 4.0");

    double no_item = marketSys.getLowestSellPrice("station_1", "nonexistent");
    assertTrue(no_item < 0, "No sell price for nonexistent item");
}

void testMarketOrderExpiry() {
    std::cout << "\n=== Market Order Expiry ===" << std::endl;
    ecs::World world;
    systems::MarketSystem marketSys(&world);

    auto* station = world.createEntity("station_1");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "station_1";

    auto* seller = world.createEntity("seller_1");
    auto* pc = addComp<components::Player>(seller);
    pc->isk = 1000000.0;

    marketSys.placeSellOrder("station_1", "seller_1", "tritanium", "Tritanium", 100, 5.0);
    assertTrue(marketSys.getOrderCount("station_1") == 1, "One active order");

    // Set order duration
    hub->orders[0].duration_remaining = 5.0f;

    marketSys.update(6.0f);
    assertTrue(marketSys.getOrderCount("station_1") == 0, "Order expired and removed");
}

// ==================== Corporation System Tests ====================

void testCorpCreate() {
    std::cout << "\n=== Corporation Create ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* player = world.createEntity("player1");
    auto* pc = addComp<components::Player>(player);
    pc->player_id = "player1";
    pc->character_name = "TestPilot";

    assertTrue(corpSys.createCorporation("player1", "Test Corp", "TSTC"),
               "Corporation created");

    auto* corp_entity = world.getEntity("corp_test_corp");
    assertTrue(corp_entity != nullptr, "Corp entity exists");

    auto* corp = corp_entity->getComponent<components::Corporation>();
    assertTrue(corp != nullptr, "Corporation component exists");
    assertTrue(corp->ceo_id == "player1", "CEO is the creator");
    assertTrue(corp->corp_name == "Test Corp", "Corp name set");
    assertTrue(corp->ticker == "TSTC", "Ticker set");
    assertTrue(corpSys.getMemberCount("corp_test_corp") == 1, "One member after creation");
    assertTrue(pc->corporation == "Test Corp", "Player corporation updated");
}

void testCorpJoin() {
    std::cout << "\n=== Corporation Join ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    auto* p2 = world.createEntity("player2");
    auto* pc2 = addComp<components::Player>(p2);
    pc2->player_id = "player2";

    corpSys.createCorporation("player1", "Join Corp", "JNCO");

    assertTrue(corpSys.joinCorporation("player2", "corp_join_corp"),
               "Player2 joins corp");
    assertTrue(corpSys.getMemberCount("corp_join_corp") == 2, "Two members after join");
    assertTrue(pc2->corporation == "Join Corp", "Player2 corporation updated");
    assertTrue(!corpSys.joinCorporation("player2", "corp_join_corp"),
               "Duplicate join rejected");
}

void testCorpLeave() {
    std::cout << "\n=== Corporation Leave ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    auto* p2 = world.createEntity("player2");
    auto* pc2 = addComp<components::Player>(p2);
    pc2->player_id = "player2";

    corpSys.createCorporation("player1", "Leave Corp", "LVCO");
    corpSys.joinCorporation("player2", "corp_leave_corp");

    assertTrue(corpSys.leaveCorporation("player2", "corp_leave_corp"),
               "Player2 leaves corp");
    assertTrue(corpSys.getMemberCount("corp_leave_corp") == 1, "One member after leave");
    assertTrue(pc2->corporation == "NPC Corp", "Player2 corporation reset");
}

void testCorpCeoCannotLeave() {
    std::cout << "\n=== Corporation CEO Cannot Leave ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    corpSys.createCorporation("player1", "CEO Corp", "CEOC");

    assertTrue(!corpSys.leaveCorporation("player1", "corp_ceo_corp"),
               "CEO cannot leave corporation");
    assertTrue(corpSys.getMemberCount("corp_ceo_corp") == 1, "Member count unchanged");
}

void testCorpTaxRate() {
    std::cout << "\n=== Corporation Tax Rate ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    auto* p2 = world.createEntity("player2");
    auto* pc2 = addComp<components::Player>(p2);
    pc2->player_id = "player2";

    corpSys.createCorporation("player1", "Tax Corp", "TAXC");
    corpSys.joinCorporation("player2", "corp_tax_corp");

    assertTrue(corpSys.setTaxRate("corp_tax_corp", "player1", 0.10f),
               "CEO can set tax rate");
    auto* corp = world.getEntity("corp_tax_corp")->getComponent<components::Corporation>();
    assertTrue(approxEqual(corp->tax_rate, 0.10f), "Tax rate updated to 10%");

    assertTrue(!corpSys.setTaxRate("corp_tax_corp", "player2", 0.20f),
               "Non-CEO cannot set tax rate");
    assertTrue(approxEqual(corp->tax_rate, 0.10f), "Tax rate unchanged");
}

void testCorpApplyTax() {
    std::cout << "\n=== Corporation Apply Tax ===" << std::endl;
    ecs::World world;
    systems::CorporationSystem corpSys(&world);

    auto* p1 = world.createEntity("player1");
    auto* pc1 = addComp<components::Player>(p1);
    pc1->player_id = "player1";

    corpSys.createCorporation("player1", "Wallet Corp", "WLTC");
    corpSys.setTaxRate("corp_wallet_corp", "player1", 0.10f);

    double remaining = corpSys.applyTax("corp_wallet_corp", 1000.0);
    assertTrue(approxEqual(static_cast<float>(remaining), 900.0f), "Remaining ISK after 10% tax");

    auto* corp = world.getEntity("corp_wallet_corp")->getComponent<components::Corporation>();
    assertTrue(approxEqual(static_cast<float>(corp->corp_wallet), 100.0f), "Corp wallet received tax");
}

void testSerializeDeserializeCorporation() {
    std::cout << "\n=== Serialize/Deserialize Corporation ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("corp_test");
    auto* corp = addComp<components::Corporation>(entity);
    corp->corp_id = "corp_test";
    corp->corp_name = "Serialize Corp";
    corp->ticker = "SRLZ";
    corp->ceo_id = "player1";
    corp->tax_rate = 0.15f;
    corp->corp_wallet = 50000.0;
    corp->member_ids.push_back("player1");
    corp->member_ids.push_back("player2");

    components::Corporation::CorpHangarItem item;
    item.item_id = "tritanium"; item.name = "Tritanium";
    item.type = "ore"; item.quantity = 1000; item.volume = 0.01f;
    corp->hangar_items.push_back(item);

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "Corporation deserialization succeeds");

    auto* e2 = world2.getEntity("corp_test");
    assertTrue(e2 != nullptr, "Corp entity recreated");

    auto* corp2 = e2->getComponent<components::Corporation>();
    assertTrue(corp2 != nullptr, "Corporation component recreated");
    assertTrue(corp2->corp_name == "Serialize Corp", "corp_name preserved");
    assertTrue(corp2->ticker == "SRLZ", "ticker preserved");
    assertTrue(corp2->ceo_id == "player1", "ceo_id preserved");
    assertTrue(approxEqual(corp2->tax_rate, 0.15f), "tax_rate preserved");
    assertTrue(approxEqual(static_cast<float>(corp2->corp_wallet), 50000.0f), "corp_wallet preserved");
    assertTrue(corp2->member_ids.size() == 2, "member_ids count preserved");
    assertTrue(corp2->member_ids[0] == "player1", "member_ids[0] preserved");
    assertTrue(corp2->member_ids[1] == "player2", "member_ids[1] preserved");
    assertTrue(corp2->hangar_items.size() == 1, "hangar_items count preserved");
    assertTrue(corp2->hangar_items[0].item_id == "tritanium", "hangar item_id preserved");
    assertTrue(corp2->hangar_items[0].quantity == 1000, "hangar item quantity preserved");
}

// ==================== ContractSystem Tests ====================

void testContractCreate() {
    std::cout << "\n=== Contract Create ===" << std::endl;
    ecs::World world;
    systems::ContractSystem contractSys(&world);
    auto* station = world.createEntity("station_1");
    addComp<components::ContractBoard>(station);

    assertTrue(contractSys.createContract("station_1", "player_1", "item_exchange", 50000.0, 3600.0f),
               "Contract created successfully");
    assertTrue(contractSys.getActiveContractCount("station_1") == 1, "Active contract count is 1");
    assertTrue(contractSys.getContractsByStatus("station_1", "outstanding") == 1,
               "Outstanding contract count is 1");
}

void testContractAccept() {
    std::cout << "\n=== Contract Accept ===" << std::endl;
    ecs::World world;
    systems::ContractSystem contractSys(&world);
    auto* station = world.createEntity("station_1");
    addComp<components::ContractBoard>(station);

    contractSys.createContract("station_1", "player_1", "courier", 100000.0, -1.0f);
    auto* board = station->getComponent<components::ContractBoard>();
    std::string cid = board->contracts[0].contract_id;

    assertTrue(contractSys.acceptContract("station_1", cid, "player_2"),
               "Contract accepted");
    assertTrue(board->contracts[0].status == "in_progress", "Status changed to in_progress");
    assertTrue(board->contracts[0].assignee_id == "player_2", "Assignee set correctly");
    assertTrue(contractSys.getContractsByStatus("station_1", "outstanding") == 0,
               "No outstanding contracts after accept");
    assertTrue(contractSys.getContractsByStatus("station_1", "in_progress") == 1,
               "One in_progress contract after accept");
}

void testContractComplete() {
    std::cout << "\n=== Contract Complete ===" << std::endl;
    ecs::World world;
    systems::ContractSystem contractSys(&world);
    auto* station = world.createEntity("station_1");
    addComp<components::ContractBoard>(station);

    auto* acceptor = world.createEntity("player_2");
    auto* player = addComp<components::Player>(acceptor);
    player->isk = 10000.0;

    contractSys.createContract("station_1", "player_1", "item_exchange", 75000.0, -1.0f);
    auto* board = station->getComponent<components::ContractBoard>();
    std::string cid = board->contracts[0].contract_id;

    contractSys.acceptContract("station_1", cid, "player_2");
    assertTrue(contractSys.completeContract("station_1", cid), "Contract completed");
    assertTrue(board->contracts[0].status == "completed", "Status is completed");
    assertTrue(approxEqual(static_cast<float>(player->isk), 85000.0f), "ISK reward paid to acceptor");
}

void testContractExpiry() {
    std::cout << "\n=== Contract Expiry ===" << std::endl;
    ecs::World world;
    systems::ContractSystem contractSys(&world);
    auto* station = world.createEntity("station_1");
    addComp<components::ContractBoard>(station);

    contractSys.createContract("station_1", "player_1", "auction", 0.0, 10.0f);

    contractSys.update(5.0f);
    assertTrue(contractSys.getContractsByStatus("station_1", "outstanding") == 1,
               "Contract still outstanding at 5s");

    contractSys.update(6.0f);
    assertTrue(contractSys.getContractsByStatus("station_1", "outstanding") == 0,
               "No outstanding contracts after 11s");
    assertTrue(contractSys.getContractsByStatus("station_1", "expired") == 1,
               "Contract expired after 11s");
}

void testContractStatusQuery() {
    std::cout << "\n=== Contract Status Query ===" << std::endl;
    ecs::World world;
    systems::ContractSystem contractSys(&world);
    auto* station = world.createEntity("station_1");
    addComp<components::ContractBoard>(station);

    contractSys.createContract("station_1", "p1", "item_exchange", 1000.0, -1.0f);
    contractSys.createContract("station_1", "p2", "courier", 2000.0, 5.0f);
    contractSys.createContract("station_1", "p3", "auction", 3000.0, -1.0f);

    auto* board = station->getComponent<components::ContractBoard>();
    contractSys.acceptContract("station_1", board->contracts[0].contract_id, "buyer_1");
    contractSys.completeContract("station_1", board->contracts[0].contract_id);

    contractSys.update(6.0f); // expire the second contract

    assertTrue(contractSys.getContractsByStatus("station_1", "completed") == 1,
               "1 completed contract");
    assertTrue(contractSys.getContractsByStatus("station_1", "expired") == 1,
               "1 expired contract");
    assertTrue(contractSys.getContractsByStatus("station_1", "outstanding") == 1,
               "1 outstanding contract");
    assertTrue(contractSys.getActiveContractCount("station_1") == 1,
               "1 active contract (outstanding only)");
}

void testSerializeDeserializeContractBoard() {
    std::cout << "\n=== Serialize/Deserialize ContractBoard ===" << std::endl;

    ecs::World world;
    auto* entity = world.createEntity("board_test");
    auto* board = addComp<components::ContractBoard>(entity);

    components::ContractBoard::Contract c;
    c.contract_id = "contract_p1_0";
    c.issuer_id = "p1";
    c.assignee_id = "p2";
    c.type = "courier";
    c.status = "in_progress";
    c.isk_reward = 50000.0;
    c.isk_collateral = 10000.0;
    c.duration_remaining = 100.0f;
    c.days_to_complete = 7.0f;

    components::ContractBoard::ContractItem offered;
    offered.item_id = "trit"; offered.name = "Tritanium";
    offered.quantity = 500; offered.volume = 0.01f;
    c.items_offered.push_back(offered);

    components::ContractBoard::ContractItem requested;
    requested.item_id = "pye"; requested.name = "Pyerite";
    requested.quantity = 100; requested.volume = 0.01f;
    c.items_requested.push_back(requested);

    board->contracts.push_back(c);

    data::WorldPersistence persistence;
    std::string json = persistence.serializeWorld(&world);

    ecs::World world2;
    assertTrue(persistence.deserializeWorld(&world2, json),
               "ContractBoard deserialization succeeds");

    auto* e2 = world2.getEntity("board_test");
    assertTrue(e2 != nullptr, "Board entity recreated");

    auto* board2 = e2->getComponent<components::ContractBoard>();
    assertTrue(board2 != nullptr, "ContractBoard component recreated");
    assertTrue(board2->contracts.size() == 1, "Contract count preserved");
    assertTrue(board2->contracts[0].contract_id == "contract_p1_0", "contract_id preserved");
    assertTrue(board2->contracts[0].issuer_id == "p1", "issuer_id preserved");
    assertTrue(board2->contracts[0].assignee_id == "p2", "assignee_id preserved");
    assertTrue(board2->contracts[0].type == "courier", "type preserved");
    assertTrue(board2->contracts[0].status == "in_progress", "status preserved");
    assertTrue(approxEqual(static_cast<float>(board2->contracts[0].isk_reward), 50000.0f), "isk_reward preserved");
    assertTrue(approxEqual(static_cast<float>(board2->contracts[0].isk_collateral), 10000.0f), "isk_collateral preserved");
    assertTrue(approxEqual(board2->contracts[0].duration_remaining, 100.0f), "duration_remaining preserved");
    assertTrue(approxEqual(board2->contracts[0].days_to_complete, 7.0f), "days_to_complete preserved");
    assertTrue(board2->contracts[0].items_offered.size() == 1, "items_offered count preserved");
    assertTrue(board2->contracts[0].items_offered[0].item_id == "trit", "offered item_id preserved");
    assertTrue(board2->contracts[0].items_offered[0].quantity == 500, "offered quantity preserved");
    assertTrue(board2->contracts[0].items_requested.size() == 1, "items_requested count preserved");
    assertTrue(board2->contracts[0].items_requested[0].item_id == "pye", "requested item_id preserved");
}

// ==================== PISystem Tests ====================

void testPIInstallExtractor() {
    std::cout << "\n=== PI Install Extractor ===" << std::endl;

    ecs::World world;
    systems::PISystem piSys(&world);

    auto* entity = world.createEntity("colony1");
    auto* colony = addComp<components::PlanetaryColony>(entity);
    colony->colony_id = "col_1";
    colony->owner_id = "player1";
    colony->planet_type = "barren";
    colony->cpu_max = 1675.0f;
    colony->powergrid_max = 6000.0f;

    bool ok = piSys.installExtractor("colony1", "base_metals", 100);
    assertTrue(ok, "Extractor installed successfully");
    assertTrue(piSys.getExtractorCount("colony1") == 1, "1 extractor present");
    assertTrue(colony->extractors[0].resource_type == "base_metals", "Extractor resource type correct");
    assertTrue(colony->extractors[0].quantity_per_cycle == 100, "Extractor quantity correct");
}

void testPIInstallProcessor() {
    std::cout << "\n=== PI Install Processor ===" << std::endl;

    ecs::World world;
    systems::PISystem piSys(&world);

    auto* entity = world.createEntity("colony2");
    auto* colony = addComp<components::PlanetaryColony>(entity);
    colony->colony_id = "col_2";
    colony->owner_id = "player1";
    colony->planet_type = "temperate";
    colony->cpu_max = 1675.0f;
    colony->powergrid_max = 6000.0f;

    bool ok = piSys.installProcessor("colony2", "base_metals", "refined_metals", 40, 5);
    assertTrue(ok, "Processor installed successfully");
    assertTrue(piSys.getProcessorCount("colony2") == 1, "1 processor present");
    assertTrue(colony->processors[0].input_type == "base_metals", "Processor input type correct");
    assertTrue(colony->processors[0].output_type == "refined_metals", "Processor output type correct");
}

void testPIExtractionCycle() {
    std::cout << "\n=== PI Extraction Cycle ===" << std::endl;

    ecs::World world;
    systems::PISystem piSys(&world);

    auto* entity = world.createEntity("colony3");
    auto* colony = addComp<components::PlanetaryColony>(entity);
    colony->colony_id = "col_3";
    colony->owner_id = "player1";
    colony->planet_type = "lava";
    colony->cpu_max = 1675.0f;
    colony->powergrid_max = 6000.0f;
    colony->storage_capacity = 10000.0f;

    piSys.installExtractor("colony3", "heavy_metals", 50);
    // Set short cycle time for testing
    colony->extractors[0].cycle_time = 10.0f;

    assertTrue(piSys.getTotalStored("colony3") == 0, "Storage starts empty");

    // Tick for one full cycle
    piSys.update(10.0f);
    assertTrue(piSys.getStoredResource("colony3", "heavy_metals") == 50,
               "50 heavy_metals extracted after 1 cycle");

    // Tick for another cycle
    piSys.update(10.0f);
    assertTrue(piSys.getStoredResource("colony3", "heavy_metals") == 100,
               "100 heavy_metals after 2 cycles");
}

void testPIProcessingCycle() {
    std::cout << "\n=== PI Processing Cycle ===" << std::endl;

    ecs::World world;
    systems::PISystem piSys(&world);

    auto* entity = world.createEntity("colony4");
    auto* colony = addComp<components::PlanetaryColony>(entity);
    colony->colony_id = "col_4";
    colony->owner_id = "player1";
    colony->planet_type = "oceanic";
    colony->cpu_max = 1675.0f;
    colony->powergrid_max = 6000.0f;
    colony->storage_capacity = 10000.0f;

    // Pre-load raw materials
    components::PlanetaryColony::StoredResource sr;
    sr.resource_type = "aqueous_liquids";
    sr.quantity = 200;
    colony->storage.push_back(sr);

    piSys.installProcessor("colony4", "aqueous_liquids", "water", 40, 5);
    colony->processors[0].cycle_time = 10.0f;

    piSys.update(10.0f);
    assertTrue(piSys.getStoredResource("colony4", "aqueous_liquids") == 160,
               "40 aqueous_liquids consumed");
    assertTrue(piSys.getStoredResource("colony4", "water") == 5,
               "5 water produced");
}

void testPICpuPowergridLimit() {
    std::cout << "\n=== PI CPU/PG Limit ===" << std::endl;

    ecs::World world;
    systems::PISystem piSys(&world);

    auto* entity = world.createEntity("colony5");
    auto* colony = addComp<components::PlanetaryColony>(entity);
    colony->colony_id = "col_5";
    colony->owner_id = "player1";
    colony->planet_type = "gas";
    colony->cpu_max = 100.0f;    // Very limited
    colony->powergrid_max = 600.0f;

    bool ok1 = piSys.installExtractor("colony5", "noble_gas", 50);
    assertTrue(ok1, "First extractor fits");

    // Second extractor should fail (cpu 45+45=90 fits, but pg 550+550=1100 > 600)
    bool ok2 = piSys.installExtractor("colony5", "reactive_gas", 30);
    assertTrue(!ok2, "Second extractor rejected (PG exceeded)");
    assertTrue(piSys.getExtractorCount("colony5") == 1, "Still only 1 extractor");
}

void testPIStorageCapacityLimit() {
    std::cout << "\n=== PI Storage Capacity Limit ===" << std::endl;

    ecs::World world;
    systems::PISystem piSys(&world);

    auto* entity = world.createEntity("colony6");
    auto* colony = addComp<components::PlanetaryColony>(entity);
    colony->colony_id = "col_6";
    colony->owner_id = "player1";
    colony->planet_type = "barren";
    colony->cpu_max = 1675.0f;
    colony->powergrid_max = 6000.0f;
    colony->storage_capacity = 100.0f;

    piSys.installExtractor("colony6", "base_metals", 60);
    colony->extractors[0].cycle_time = 10.0f;

    // First cycle: 60 extracted (< 100 capacity)
    piSys.update(10.0f);
    assertTrue(piSys.getStoredResource("colony6", "base_metals") == 60,
               "60 extracted (under capacity)");

    // Second cycle: 60 + 60 = 120 > 100 capacity, should not extract
    piSys.update(10.0f);
    assertTrue(piSys.getStoredResource("colony6", "base_metals") == 60,
               "Still 60 (storage full, extraction skipped)");
}

// ==================== ManufacturingSystem Tests ====================

void testManufacturingStartJob() {
    std::cout << "\n=== Manufacturing Start Job ===" << std::endl;

    ecs::World world;
    systems::ManufacturingSystem mfgSys(&world);

    auto* station = world.createEntity("station1");
    auto* facility = addComp<components::ManufacturingFacility>(station);
    facility->facility_id = "fac_1";
    facility->station_id = "station1";
    facility->max_jobs = 2;

    auto* player = world.createEntity("player1");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "player1";
    pcomp->isk = 100000.0;

    std::string job_id = mfgSys.startJob("station1", "player1", "fang_blueprint",
                                           "fang", "Fang Frigate", 1, 3600.0f, 1000.0);
    assertTrue(!job_id.empty(), "Job started successfully");
    assertTrue(mfgSys.getActiveJobCount("station1") == 1, "1 active job");
    assertTrue(approxEqual(static_cast<float>(pcomp->isk), 99000.0f), "Install cost deducted");
}

void testManufacturingJobCompletion() {
    std::cout << "\n=== Manufacturing Job Completion ===" << std::endl;

    ecs::World world;
    systems::ManufacturingSystem mfgSys(&world);

    auto* station = world.createEntity("station2");
    auto* facility = addComp<components::ManufacturingFacility>(station);
    facility->facility_id = "fac_2";
    facility->max_jobs = 1;

    auto* player = world.createEntity("player2");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "player2";
    pcomp->isk = 100000.0;

    mfgSys.startJob("station2", "player2", "autocannon_bp",
                     "autocannon_i", "150mm Autocannon I", 1, 100.0f, 500.0);

    assertTrue(mfgSys.getActiveJobCount("station2") == 1, "Job is active");
    assertTrue(mfgSys.getCompletedJobCount("station2") == 0, "No completed jobs yet");

    // Tick to completion
    mfgSys.update(100.0f);
    assertTrue(mfgSys.getActiveJobCount("station2") == 0, "No active jobs after completion");
    assertTrue(mfgSys.getCompletedJobCount("station2") == 1, "1 completed job");
    assertTrue(mfgSys.getTotalRunsCompleted("station2") == 1, "1 run completed");
}

void testManufacturingMultipleRuns() {
    std::cout << "\n=== Manufacturing Multiple Runs ===" << std::endl;

    ecs::World world;
    systems::ManufacturingSystem mfgSys(&world);

    auto* station = world.createEntity("station3");
    auto* facility = addComp<components::ManufacturingFacility>(station);
    facility->facility_id = "fac_3";
    facility->max_jobs = 1;

    auto* player = world.createEntity("player3");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "player3";
    pcomp->isk = 100000.0;

    mfgSys.startJob("station3", "player3", "drone_bp",
                     "hobgoblin_i", "Hobgoblin I", 3, 50.0f, 200.0);

    // First run
    mfgSys.update(50.0f);
    assertTrue(mfgSys.getTotalRunsCompleted("station3") == 1, "1 run after 50s");
    assertTrue(mfgSys.getActiveJobCount("station3") == 1, "Job still active (more runs)");

    // Second run
    mfgSys.update(50.0f);
    assertTrue(mfgSys.getTotalRunsCompleted("station3") == 2, "2 runs after 100s");

    // Third run (final)
    mfgSys.update(50.0f);
    assertTrue(mfgSys.getTotalRunsCompleted("station3") == 3, "3 runs after 150s");
    assertTrue(mfgSys.getCompletedJobCount("station3") == 1, "Job completed");
    assertTrue(mfgSys.getActiveJobCount("station3") == 0, "No active jobs");
}

void testManufacturingJobSlotLimit() {
    std::cout << "\n=== Manufacturing Job Slot Limit ===" << std::endl;

    ecs::World world;
    systems::ManufacturingSystem mfgSys(&world);

    auto* station = world.createEntity("station4");
    auto* facility = addComp<components::ManufacturingFacility>(station);
    facility->facility_id = "fac_4";
    facility->max_jobs = 1;

    auto* player = world.createEntity("player4");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "player4";
    pcomp->isk = 100000.0;

    std::string job1 = mfgSys.startJob("station4", "player4", "bp1",
                                         "item1", "Item 1", 1, 3600.0f, 100.0);
    assertTrue(!job1.empty(), "First job started");

    std::string job2 = mfgSys.startJob("station4", "player4", "bp2",
                                         "item2", "Item 2", 1, 3600.0f, 100.0);
    assertTrue(job2.empty(), "Second job rejected (slot full)");
    assertTrue(mfgSys.getActiveJobCount("station4") == 1, "Still 1 active job");
}

void testManufacturingCancelJob() {
    std::cout << "\n=== Manufacturing Cancel Job ===" << std::endl;

    ecs::World world;
    systems::ManufacturingSystem mfgSys(&world);

    auto* station = world.createEntity("station5");
    auto* facility = addComp<components::ManufacturingFacility>(station);
    facility->facility_id = "fac_5";
    facility->max_jobs = 2;

    auto* player = world.createEntity("player5");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "player5";
    pcomp->isk = 100000.0;

    std::string job_id = mfgSys.startJob("station5", "player5", "bp_test",
                                           "item_test", "Test Item", 1, 3600.0f, 100.0);
    assertTrue(mfgSys.getActiveJobCount("station5") == 1, "1 active job");

    bool cancelled = mfgSys.cancelJob("station5", job_id);
    assertTrue(cancelled, "Job cancelled successfully");
    assertTrue(mfgSys.getActiveJobCount("station5") == 0, "No active jobs after cancel");
}

void testManufacturingInsufficientFunds() {
    std::cout << "\n=== Manufacturing Insufficient Funds ===" << std::endl;

    ecs::World world;
    systems::ManufacturingSystem mfgSys(&world);

    auto* station = world.createEntity("station6");
    auto* facility = addComp<components::ManufacturingFacility>(station);
    facility->facility_id = "fac_6";
    facility->max_jobs = 1;

    auto* player = world.createEntity("player6");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "player6";
    pcomp->isk = 50.0;  // Not enough

    std::string job_id = mfgSys.startJob("station6", "player6", "bp_expensive",
                                           "item_expensive", "Expensive Item", 1, 3600.0f, 1000.0);
    assertTrue(job_id.empty(), "Job rejected (insufficient funds)");
    assertTrue(mfgSys.getActiveJobCount("station6") == 0, "No active jobs");
    assertTrue(approxEqual(static_cast<float>(pcomp->isk), 50.0f), "ISK unchanged");
}

// ==================== ResearchSystem Tests ====================

void testResearchME() {
    std::cout << "\n=== Research ME ===" << std::endl;

    ecs::World world;
    systems::ResearchSystem resSys(&world);

    auto* station = world.createEntity("lab1");
    auto* lab = addComp<components::ResearchLab>(station);
    lab->lab_id = "lab_1";
    lab->max_jobs = 1;

    auto* player = world.createEntity("researcher1");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "researcher1";
    pcomp->isk = 100000.0;

    std::string job_id = resSys.startMEResearch("lab1", "researcher1", "fang_blueprint",
                                                  5, 100.0f, 500.0);
    assertTrue(!job_id.empty(), "ME research started");
    assertTrue(resSys.getActiveJobCount("lab1") == 1, "1 active job");
    assertTrue(approxEqual(static_cast<float>(pcomp->isk), 99500.0f), "Install cost deducted");

    // Complete
    resSys.update(100.0f);
    assertTrue(resSys.getActiveJobCount("lab1") == 0, "No active jobs");
    assertTrue(resSys.getCompletedJobCount("lab1") == 1, "1 completed job");
}

void testResearchTE() {
    std::cout << "\n=== Research TE ===" << std::endl;

    ecs::World world;
    systems::ResearchSystem resSys(&world);

    auto* station = world.createEntity("lab2");
    auto* lab = addComp<components::ResearchLab>(station);
    lab->lab_id = "lab_2";
    lab->max_jobs = 1;

    auto* player = world.createEntity("researcher2");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "researcher2";
    pcomp->isk = 100000.0;

    std::string job_id = resSys.startTEResearch("lab2", "researcher2", "autocannon_bp",
                                                  10, 200.0f, 300.0);
    assertTrue(!job_id.empty(), "TE research started");
    assertTrue(resSys.getActiveJobCount("lab2") == 1, "1 active job");

    resSys.update(200.0f);
    assertTrue(resSys.getCompletedJobCount("lab2") == 1, "TE research completed");
}

void testResearchInvention() {
    std::cout << "\n=== Research Invention ===" << std::endl;

    ecs::World world;
    systems::ResearchSystem resSys(&world);

    auto* station = world.createEntity("lab3");
    auto* lab = addComp<components::ResearchLab>(station);
    lab->lab_id = "lab_3";
    lab->max_jobs = 1;

    auto* player = world.createEntity("researcher3");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "researcher3";
    pcomp->isk = 100000.0;

    std::string job_id = resSys.startInvention("lab3", "researcher3",
                                                "fang_blueprint", "fang_ii_blueprint",
                                                "datacore_mechanical_engineering",
                                                "datacore_electronic_engineering",
                                                1.0f, // 100% success for testing
                                                50.0f, 1000.0);
    assertTrue(!job_id.empty(), "Invention started");
    assertTrue(resSys.getActiveJobCount("lab3") == 1, "1 active job");

    resSys.update(50.0f);
    // With 100% success chance, it should complete
    assertTrue(resSys.getCompletedJobCount("lab3") == 1, "Invention succeeded");
    assertTrue(resSys.getFailedJobCount("lab3") == 0, "No failed jobs");
}

void testResearchInventionFailure() {
    std::cout << "\n=== Research Invention Failure ===" << std::endl;

    ecs::World world;
    systems::ResearchSystem resSys(&world);

    auto* station = world.createEntity("lab4");
    auto* lab = addComp<components::ResearchLab>(station);
    lab->lab_id = "lab_4";
    lab->max_jobs = 1;

    auto* player = world.createEntity("researcher4");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "researcher4";
    pcomp->isk = 100000.0;

    std::string job_id = resSys.startInvention("lab4", "researcher4",
                                                "fang_blueprint", "fang_ii_blueprint",
                                                "datacore_mechanical_engineering",
                                                "datacore_electronic_engineering",
                                                0.0f, // 0% success = guaranteed fail
                                                50.0f, 500.0);
    assertTrue(!job_id.empty(), "Invention job started");

    resSys.update(50.0f);
    assertTrue(resSys.getFailedJobCount("lab4") == 1, "Invention failed (0% chance)");
    assertTrue(resSys.getCompletedJobCount("lab4") == 0, "No completed jobs");
}

void testResearchJobSlotLimit() {
    std::cout << "\n=== Research Job Slot Limit ===" << std::endl;

    ecs::World world;
    systems::ResearchSystem resSys(&world);

    auto* station = world.createEntity("lab5");
    auto* lab = addComp<components::ResearchLab>(station);
    lab->lab_id = "lab_5";
    lab->max_jobs = 1;

    auto* player = world.createEntity("researcher5");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "researcher5";
    pcomp->isk = 100000.0;

    std::string job1 = resSys.startMEResearch("lab5", "researcher5", "bp1",
                                                5, 1000.0f, 100.0);
    assertTrue(!job1.empty(), "First research job started");

    std::string job2 = resSys.startTEResearch("lab5", "researcher5", "bp2",
                                                10, 1000.0f, 100.0);
    assertTrue(job2.empty(), "Second job rejected (slot full)");
    assertTrue(resSys.getActiveJobCount("lab5") == 1, "Still 1 active job");
}

void testResearchInsufficientFunds() {
    std::cout << "\n=== Research Insufficient Funds ===" << std::endl;

    ecs::World world;
    systems::ResearchSystem resSys(&world);

    auto* station = world.createEntity("lab6");
    auto* lab = addComp<components::ResearchLab>(station);
    lab->lab_id = "lab_6";
    lab->max_jobs = 1;

    auto* player = world.createEntity("researcher6");
    auto* pcomp = addComp<components::Player>(player);
    pcomp->player_id = "researcher6";
    pcomp->isk = 10.0;  // Not enough

    std::string job_id = resSys.startMEResearch("lab6", "researcher6", "bp_expensive",
                                                  5, 1000.0f, 500.0);
    assertTrue(job_id.empty(), "Job rejected (insufficient funds)");
    assertTrue(resSys.getActiveJobCount("lab6") == 0, "No active jobs");
    assertTrue(approxEqual(static_cast<float>(pcomp->isk), 10.0f), "ISK unchanged");
}

// ==================== Chat System Tests ====================

void testChatJoinChannel() {
    std::cout << "\n=== Chat Join Channel ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);
    channel->channel_name = "local";

    assertTrue(chatSys.joinChannel("chat_channel_1", "player_1", "Alice"),
               "Player 1 joins channel");
    assertTrue(chatSys.joinChannel("chat_channel_1", "player_2", "Bob"),
               "Player 2 joins channel");
    assertTrue(chatSys.getMemberCount("chat_channel_1") == 2,
               "Member count is 2");
    // 2 join system messages
    assertTrue(chatSys.getMessageCount("chat_channel_1") >= 2,
               "System join messages sent");
}

void testChatLeaveChannel() {
    std::cout << "\n=== Chat Leave Channel ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_1", "Alice");
    assertTrue(chatSys.getMemberCount("chat_channel_1") == 1,
               "Member count is 1 after join");

    assertTrue(chatSys.leaveChannel("chat_channel_1", "player_1"),
               "Player leaves channel");
    assertTrue(chatSys.getMemberCount("chat_channel_1") == 0,
               "Member count is 0 after leave");
    // 1 join + 1 leave system message
    bool hasLeaveMsg = false;
    auto* ch = entity->getComponent<components::ChatChannel>();
    for (const auto& m : ch->messages) {
        if (m.content.find("has left the channel") != std::string::npos)
            hasLeaveMsg = true;
    }
    assertTrue(hasLeaveMsg, "Leave system message exists");
}

void testChatSendMessage() {
    std::cout << "\n=== Chat Send Message ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_1", "Alice");
    int baseCount = chatSys.getMessageCount("chat_channel_1");

    assertTrue(chatSys.sendMessage("chat_channel_1", "player_1", "Alice", "Hello!"),
               "First message sent");
    assertTrue(chatSys.sendMessage("chat_channel_1", "player_1", "Alice", "World!"),
               "Second message sent");
    assertTrue(chatSys.getMessageCount("chat_channel_1") == baseCount + 2,
               "Message count increased by 2");
}

void testChatMutePlayer() {
    std::cout << "\n=== Chat Mute Player ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_mod", "Moderator");
    chatSys.joinChannel("chat_channel_1", "player_2", "Bob");

    // Set moderator role
    for (auto& m : channel->members) {
        if (m.player_id == "player_mod") m.role = "moderator";
    }

    assertTrue(chatSys.mutePlayer("chat_channel_1", "player_mod", "player_2"),
               "Moderator mutes player");
    assertTrue(!chatSys.sendMessage("chat_channel_1", "player_2", "Bob", "test"),
               "Muted player cannot send message");
}

void testChatUnmutePlayer() {
    std::cout << "\n=== Chat Unmute Player ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_mod", "Moderator");
    chatSys.joinChannel("chat_channel_1", "player_2", "Bob");

    for (auto& m : channel->members) {
        if (m.player_id == "player_mod") m.role = "moderator";
    }

    chatSys.mutePlayer("chat_channel_1", "player_mod", "player_2");
    assertTrue(!chatSys.sendMessage("chat_channel_1", "player_2", "Bob", "blocked"),
               "Muted player cannot send");

    assertTrue(chatSys.unmutePlayer("chat_channel_1", "player_mod", "player_2"),
               "Moderator unmutes player");
    assertTrue(chatSys.sendMessage("chat_channel_1", "player_2", "Bob", "free!"),
               "Unmuted player can send again");
}

void testChatSetMotd() {
    std::cout << "\n=== Chat Set MOTD ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_owner", "Owner");
    chatSys.joinChannel("chat_channel_1", "player_2", "Bob");

    // Set owner role
    for (auto& m : channel->members) {
        if (m.player_id == "player_owner") m.role = "owner";
    }

    assertTrue(chatSys.setMotd("chat_channel_1", "player_owner", "Welcome!"),
               "Owner sets MOTD");
    assertTrue(channel->motd == "Welcome!", "MOTD was set correctly");

    assertTrue(!chatSys.setMotd("chat_channel_1", "player_2", "Hacked!"),
               "Regular member cannot set MOTD");
    assertTrue(channel->motd == "Welcome!", "MOTD unchanged after failed attempt");
}

void testChatMaxMembers() {
    std::cout << "\n=== Chat Max Members ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);
    channel->max_members = 2;

    assertTrue(chatSys.joinChannel("chat_channel_1", "player_1", "Alice"),
               "Player 1 joins (1/2)");
    assertTrue(chatSys.joinChannel("chat_channel_1", "player_2", "Bob"),
               "Player 2 joins (2/2)");
    assertTrue(!chatSys.joinChannel("chat_channel_1", "player_3", "Charlie"),
               "Player 3 cannot join (channel full)");
    assertTrue(chatSys.getMemberCount("chat_channel_1") == 2,
               "Member count stays at 2");
}

void testChatMessageHistory() {
    std::cout << "\n=== Chat Message History ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);
    channel->max_history = 5;

    chatSys.joinChannel("chat_channel_1", "player_1", "Alice");
    // join message = 1, then send 8 more = 9 total
    for (int i = 0; i < 8; ++i) {
        chatSys.sendMessage("chat_channel_1", "player_1", "Alice",
                            "Message " + std::to_string(i));
    }
    assertTrue(static_cast<int>(channel->messages.size()) > 5,
               "Messages exceed max_history before trim");

    chatSys.update(0.0f);
    assertTrue(static_cast<int>(channel->messages.size()) <= 5,
               "Messages trimmed to max_history after update");
}

void testChatMutedPlayerCannotSend() {
    std::cout << "\n=== Chat Muted Player Cannot Send ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    auto* channel = addComp<components::ChatChannel>(entity);

    chatSys.joinChannel("chat_channel_1", "player_1", "Alice");

    // Directly mute via component
    for (auto& m : channel->members) {
        if (m.player_id == "player_1") m.is_muted = true;
    }

    assertTrue(!chatSys.sendMessage("chat_channel_1", "player_1", "Alice", "test"),
               "Directly muted player cannot send");
}

void testChatNonMemberCannotSend() {
    std::cout << "\n=== Chat Non-Member Cannot Send ===" << std::endl;
    ecs::World world;
    systems::ChatSystem chatSys(&world);

    auto* entity = world.createEntity("chat_channel_1");
    addComp<components::ChatChannel>(entity);

    assertTrue(!chatSys.sendMessage("chat_channel_1", "player_1", "Alice", "test"),
               "Non-member cannot send message");
}

// ==================== CharacterCreationSystem Tests ====================

void testCharacterCreate() {
    std::cout << "\n=== Character Create ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);

    bool result = charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");
    assertTrue(result, "createCharacter returns true for valid race");

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->character_name == "TestPilot", "Character name is set correctly");
    assertTrue(sheet->intelligence == 23 && sheet->memory == 21, "Caldari starting attributes are correct");
}

void testCharacterInvalidRace() {
    std::cout << "\n=== Character Invalid Race ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);

    bool result = charSys.createCharacter("pilot_1", "TestPilot", "Jove", "Unknown", "Unknown", "male");
    assertTrue(!result, "createCharacter returns false for invalid race Jove");
}

void testCharacterInstallImplant() {
    std::cout << "\n=== Character Install Implant ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    bool result = charSys.installImplant("pilot_1", "imp_1", "Neural Boost", 1, "intelligence", 3);
    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->implants.size() == 1, "Implant added to implants vector");
    assertTrue(charSys.getEffectiveAttribute("pilot_1", "intelligence") == 23 + 3, "Effective attribute includes implant bonus");
}

void testCharacterImplantSlotOccupied() {
    std::cout << "\n=== Character Implant Slot Occupied ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    bool first_install_result = charSys.installImplant("pilot_1", "imp_1", "Neural Boost", 1, "intelligence", 3);
    assertTrue(first_install_result, "First implant in slot 1 succeeds");

    bool second_install_result = charSys.installImplant("pilot_1", "imp_2", "Another Boost", 1, "perception", 2);
    assertTrue(!second_install_result, "Second implant in same slot 1 fails");
}

void testCharacterRemoveImplant() {
    std::cout << "\n=== Character Remove Implant ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");
    charSys.installImplant("pilot_1", "imp_1", "Neural Boost", 1, "intelligence", 3);

    bool result = charSys.removeImplant("pilot_1", 1);
    assertTrue(result, "removeImplant returns true for occupied slot");

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->implants.empty(), "Implants vector is empty after removal");
}

void testCharacterCloneGrade() {
    std::cout << "\n=== Character Clone Grade ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    bool result = charSys.setCloneGrade("pilot_1", "omega");
    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(result && sheet->clone_grade == "omega", "Clone grade set to omega");

    bool gamma_result = charSys.setCloneGrade("pilot_1", "gamma");
    assertTrue(!gamma_result, "Invalid clone grade gamma returns false");
}

void testCharacterJumpClone() {
    std::cout << "\n=== Character Jump Clone ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    bool result = charSys.jumpClone("pilot_1");
    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(result && sheet->clone_jump_cooldown > 0, "Jump clone sets cooldown");

    bool second = charSys.jumpClone("pilot_1");
    assertTrue(!second, "Cannot jump clone while on cooldown");
}

void testCharacterCloneCooldownDecay() {
    std::cout << "\n=== Character Clone Cooldown Decay ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    charSys.jumpClone("pilot_1");
    charSys.update(86400.0f);

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->clone_jump_cooldown == 0, "Cooldown decays to 0 after 86400 seconds");

    bool result = charSys.jumpClone("pilot_1");
    assertTrue(result, "Can jump clone again after cooldown expires");
}

void testCharacterSecurityStatus() {
    std::cout << "\n=== Character Security Status ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    charSys.modifySecurityStatus("pilot_1", 5.0f);
    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(approxEqual(sheet->security_status, 5.0f), "Security status increased to 5.0");

    charSys.modifySecurityStatus("pilot_1", 8.0f);
    assertTrue(approxEqual(sheet->security_status, 10.0f), "Security status clamped to 10.0");
}

void testCharacterEmploymentHistory() {
    std::cout << "\n=== Character Employment History ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* entity = world.createEntity("pilot_1");
    addComp<components::CharacterSheet>(entity);
    charSys.createCharacter("pilot_1", "TestPilot", "Caldari", "Deteis", "Scientist", "male");

    charSys.addEmploymentRecord("pilot_1", "corp_1", "Test Corp", 1000.0f);
    charSys.addEmploymentRecord("pilot_1", "corp_2", "Another Corp", 2000.0f);

    auto* sheet = entity->getComponent<components::CharacterSheet>();
    assertTrue(sheet->employment_history.size() == 2, "Two employment records added");
}

void testCharacterRaceAttributes() {
    std::cout << "\n=== Character Race Attributes ===" << std::endl;

    ecs::World world;
    systems::CharacterCreationSystem charSys(&world);

    auto* e1 = world.createEntity("amarr_pilot");
    addComp<components::CharacterSheet>(e1);
    charSys.createCharacter("amarr_pilot", "AmarrPilot", "Amarr", "Khanid", "Cyber Knight", "male");

    auto* e2 = world.createEntity("gallente_pilot");
    addComp<components::CharacterSheet>(e2);
    charSys.createCharacter("gallente_pilot", "GallentePilot", "Gallente", "Intaki", "Diplomat", "female");

    auto* e3 = world.createEntity("minmatar_pilot");
    addComp<components::CharacterSheet>(e3);
    charSys.createCharacter("minmatar_pilot", "MinmatarPilot", "Minmatar", "Brutor", "Warrior", "male");

    auto* e4 = world.createEntity("caldari_pilot");
    addComp<components::CharacterSheet>(e4);
    charSys.createCharacter("caldari_pilot", "CaldariPilot", "Caldari", "Deteis", "Scientist", "male");

    auto* s1 = e1->getComponent<components::CharacterSheet>();
    assertTrue(s1->willpower == 22, "Amarr willpower is 22");

    auto* s2 = e2->getComponent<components::CharacterSheet>();
    assertTrue(s2->charisma == 22, "Gallente charisma is 22");

    auto* s3 = e3->getComponent<components::CharacterSheet>();
    assertTrue(s3->perception == 22, "Minmatar perception is 22");

    auto* s4 = e4->getComponent<components::CharacterSheet>();
    assertTrue(s4->intelligence == 23, "Caldari intelligence is 23");
}

// ==================== TournamentSystem Tests ====================

void testTournamentCreate() {
    std::cout << "\n=== Tournament Create ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    bool created = tourneySys.createTournament("tourney_1", "pvp_tourney_1", "Arena Championship", 8, 10000.0, 300.0f);
    assertTrue(created, "Tournament created");
    assertTrue(tourneySys.getStatus("tourney_1") == "registration", "Status is registration");
    assertTrue(tourneySys.getParticipantCount("tourney_1") == 0, "Zero participants initially");
}

void testTournamentRegister() {
    std::cout << "\n=== Tournament Register ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test Tournament", 4, 5000.0, 300.0f);

    assertTrue(tourneySys.registerPlayer("tourney_1", "player_1", "Alice"), "Player 1 registered");
    assertTrue(tourneySys.registerPlayer("tourney_1", "player_2", "Bob"), "Player 2 registered");
    assertTrue(tourneySys.getParticipantCount("tourney_1") == 2, "Two participants registered");
    assertTrue(approxEqual(static_cast<float>(tourneySys.getPrizePool("tourney_1")), 10000.0f), "Prize pool is 10K");
}

void testTournamentMaxParticipants() {
    std::cout << "\n=== Tournament Max Participants ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Small Tourney", 2, 1000.0, 300.0f);

    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    bool third = tourneySys.registerPlayer("tourney_1", "p3", "Charlie");
    assertTrue(!third, "Third player rejected (tournament full)");
    assertTrue(tourneySys.getParticipantCount("tourney_1") == 2, "Still 2 participants");
}

void testTournamentDuplicateRegister() {
    std::cout << "\n=== Tournament Duplicate Register ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);

    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    bool dup = tourneySys.registerPlayer("tourney_1", "p1", "Alice Again");
    assertTrue(!dup, "Duplicate registration rejected");
    assertTrue(tourneySys.getParticipantCount("tourney_1") == 1, "Still 1 participant");
}

void testTournamentStart() {
    std::cout << "\n=== Tournament Start ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");

    bool started = tourneySys.startTournament("tourney_1");
    assertTrue(started, "Tournament started");
    assertTrue(tourneySys.getStatus("tourney_1") == "active", "Status is active");
    assertTrue(tourneySys.getCurrentRound("tourney_1") == 1, "Round 1 started");
}

void testTournamentEmptyCannotStart() {
    std::cout << "\n=== Tournament Empty Cannot Start ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Empty", 8, 0.0, 300.0f);

    bool started = tourneySys.startTournament("tourney_1");
    assertTrue(!started, "Empty tournament cannot start");
    assertTrue(tourneySys.getStatus("tourney_1") == "registration", "Status stays registration");
}

void testTournamentScoring() {
    std::cout << "\n=== Tournament Scoring ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    tourneySys.startTournament("tourney_1");

    tourneySys.recordKill("tourney_1", "p1", 5);
    tourneySys.recordKill("tourney_1", "p2", 3);
    tourneySys.recordKill("tourney_1", "p1", 2);

    assertTrue(tourneySys.getPlayerScore("tourney_1", "p1") == 7, "Player 1 score is 7");
    assertTrue(tourneySys.getPlayerScore("tourney_1", "p2") == 3, "Player 2 score is 3");
}

void testTournamentElimination() {
    std::cout << "\n=== Tournament Elimination ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    tourneySys.registerPlayer("tourney_1", "p3", "Charlie");
    tourneySys.startTournament("tourney_1");

    assertTrue(tourneySys.getActiveParticipantCount("tourney_1") == 3, "3 active before elimination");
    tourneySys.eliminatePlayer("tourney_1", "p2");
    assertTrue(tourneySys.getActiveParticipantCount("tourney_1") == 2, "2 active after elimination");

    // Eliminated player cannot score
    bool scored = tourneySys.recordKill("tourney_1", "p2", 1);
    assertTrue(!scored, "Eliminated player cannot score");
}

void testTournamentRoundAdvance() {
    std::cout << "\n=== Tournament Round Advance ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 100.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    tourneySys.startTournament("tourney_1");

    tourneySys.recordKill("tourney_1", "p1", 5);
    assertTrue(tourneySys.getCurrentRound("tourney_1") == 1, "Still round 1 before update");

    // Advance past round 1
    tourneySys.update(101.0f);
    assertTrue(tourneySys.getCurrentRound("tourney_1") == 2, "Advanced to round 2");
}

void testTournamentCompletion() {
    std::cout << "\n=== Tournament Completion ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 1000.0, 50.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    tourneySys.startTournament("tourney_1");

    // Advance through all 3 rounds
    tourneySys.update(51.0f);  // end round 1 → start round 2
    tourneySys.update(51.0f);  // end round 2 → start round 3
    tourneySys.update(51.0f);  // end round 3 → completed

    assertTrue(tourneySys.getStatus("tourney_1") == "completed", "Tournament completed after 3 rounds");
}

void testTournamentRegisterAfterStart() {
    std::cout << "\n=== Tournament Register After Start ===" << std::endl;
    ecs::World world;
    systems::TournamentSystem tourneySys(&world);

    world.createEntity("tourney_1");
    tourneySys.createTournament("tourney_1", "t1", "Test", 8, 0.0, 300.0f);
    tourneySys.registerPlayer("tourney_1", "p1", "Alice");
    tourneySys.startTournament("tourney_1");

    bool late = tourneySys.registerPlayer("tourney_1", "p2", "Bob");
    assertTrue(!late, "Cannot register after tournament starts");
}

// ==================== LeaderboardSystem Tests ====================

void testLeaderboardRecordKill() {
    std::cout << "\n=== Leaderboard Record Kill ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordKill("board_1", "p1", "Alice");
    lbSys.recordKill("board_1", "p1", "Alice");
    lbSys.recordKill("board_1", "p1", "Alice");

    assertTrue(lbSys.getPlayerKills("board_1", "p1") == 3, "Player has 3 kills");
    assertTrue(lbSys.getEntryCount("board_1") == 1, "One entry on board");
}

void testLeaderboardMultiplePlayers() {
    std::cout << "\n=== Leaderboard Multiple Players ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordKill("board_1", "p1", "Alice");
    lbSys.recordKill("board_1", "p2", "Bob");
    lbSys.recordKill("board_1", "p1", "Alice");

    assertTrue(lbSys.getEntryCount("board_1") == 2, "Two entries on board");
    assertTrue(lbSys.getPlayerKills("board_1", "p1") == 2, "Alice has 2 kills");
    assertTrue(lbSys.getPlayerKills("board_1", "p2") == 1, "Bob has 1 kill");
}

void testLeaderboardIskTracking() {
    std::cout << "\n=== Leaderboard ISK Tracking ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordIskEarned("board_1", "p1", "Alice", 50000.0);
    lbSys.recordIskEarned("board_1", "p1", "Alice", 25000.0);

    assertTrue(approxEqual(static_cast<float>(lbSys.getPlayerIskEarned("board_1", "p1")), 75000.0f), "ISK earned is 75K");
}

void testLeaderboardMissionTracking() {
    std::cout << "\n=== Leaderboard Mission Tracking ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordMissionComplete("board_1", "p1", "Alice");
    lbSys.recordMissionComplete("board_1", "p1", "Alice");

    assertTrue(lbSys.getPlayerMissions("board_1", "p1") == 2, "Player completed 2 missions");
}

void testLeaderboardRanking() {
    std::cout << "\n=== Leaderboard Ranking ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordKill("board_1", "p1", "Alice");
    for (int i = 0; i < 5; ++i) lbSys.recordKill("board_1", "p2", "Bob");
    for (int i = 0; i < 3; ++i) lbSys.recordKill("board_1", "p3", "Charlie");

    auto ranking = lbSys.getRankingByKills("board_1");
    assertTrue(static_cast<int>(ranking.size()) == 3, "Ranking has 3 entries");
    assertTrue(ranking[0] == "p2", "Bob is rank 1 (5 kills)");
    assertTrue(ranking[1] == "p3", "Charlie is rank 2 (3 kills)");
    assertTrue(ranking[2] == "p1", "Alice is rank 3 (1 kill)");
}

void testLeaderboardAchievementDefine() {
    std::cout << "\n=== Leaderboard Achievement Define ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.defineAchievement("board_1", "first_blood", "First Blood", "Get your first kill", "combat", "total_kills", 1);
    lbSys.defineAchievement("board_1", "veteran", "Veteran", "Reach 100 kills", "combat", "total_kills", 100);

    auto* lb = board->getComponent<components::Leaderboard>();
    assertTrue(static_cast<int>(lb->achievements.size()) == 2, "Two achievements defined");
}

void testLeaderboardAchievementUnlock() {
    std::cout << "\n=== Leaderboard Achievement Unlock ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.defineAchievement("board_1", "first_blood", "First Blood", "Get your first kill", "combat", "total_kills", 1);
    lbSys.defineAchievement("board_1", "veteran", "Veteran", "Reach 100 kills", "combat", "total_kills", 100);

    lbSys.recordKill("board_1", "p1", "Alice");
    int unlocked = lbSys.checkAchievements("board_1", "p1", 1000.0f);

    assertTrue(unlocked == 1, "One achievement unlocked");
    assertTrue(lbSys.hasAchievement("board_1", "p1", "first_blood"), "First Blood unlocked");
    assertTrue(!lbSys.hasAchievement("board_1", "p1", "veteran"), "Veteran not unlocked yet");
}

void testLeaderboardAchievementNoDuplicate() {
    std::cout << "\n=== Leaderboard Achievement No Duplicate ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.defineAchievement("board_1", "first_blood", "First Blood", "Get first kill", "combat", "total_kills", 1);
    lbSys.recordKill("board_1", "p1", "Alice");

    lbSys.checkAchievements("board_1", "p1");
    int second = lbSys.checkAchievements("board_1", "p1");

    assertTrue(second == 0, "No duplicate unlock");
    assertTrue(lbSys.getPlayerAchievementCount("board_1", "p1") == 1, "Still 1 achievement total");
}

void testLeaderboardNonexistentPlayer() {
    std::cout << "\n=== Leaderboard Nonexistent Player ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    assertTrue(lbSys.getPlayerKills("board_1", "fake") == 0, "Zero kills for nonexistent");
    assertTrue(approxEqual(static_cast<float>(lbSys.getPlayerIskEarned("board_1", "fake")), 0.0f), "Zero ISK for nonexistent");
    assertTrue(lbSys.getPlayerMissions("board_1", "fake") == 0, "Zero missions for nonexistent");
}

void testLeaderboardDamageTracking() {
    std::cout << "\n=== Leaderboard Damage Tracking ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordDamageDealt("board_1", "p1", "Alice", 5000.0);
    lbSys.recordDamageDealt("board_1", "p1", "Alice", 3000.0);

    auto* lb = board->getComponent<components::Leaderboard>();
    bool found = false;
    for (const auto& e : lb->entries) {
        if (e.player_id == "p1") {
            found = true;
            assertTrue(approxEqual(static_cast<float>(e.total_damage_dealt), 8000.0f), "Total damage is 8000");
        }
    }
    assertTrue(found, "Player entry found for damage tracking");
}

// ==================== StationSystem Tests ====================

void testStationCreate() {
    std::cout << "\n=== Station Create ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    bool ok = stationSys.createStation("station_1", "Test Hub", 100.0f, 0.0f, 200.0f, 3000.0f, 2.0f);
    assertTrue(ok, "Station created successfully");

    auto* entity = world.getEntity("station_1");
    assertTrue(entity != nullptr, "Station entity exists");

    auto* station = entity->getComponent<components::Station>();
    assertTrue(station != nullptr, "Station component attached");
    assertTrue(station->station_name == "Test Hub", "Station name is correct");
    assertTrue(approxEqual(station->docking_range, 3000.0f), "Docking range is correct");
    assertTrue(approxEqual(station->repair_cost_per_hp, 2.0f), "Repair cost per HP is correct");
}

void testStationDuplicateCreate() {
    std::cout << "\n=== Station Duplicate Create ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub A", 0, 0, 0);
    bool dup = stationSys.createStation("station_1", "Hub B", 0, 0, 0);
    assertTrue(!dup, "Duplicate station creation rejected");
}

void testStationDockInRange() {
    std::cout << "\n=== Station Dock In Range ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 100.0f;
    addComp<components::Velocity>(ship);
    addComp<components::Player>(ship);

    bool ok = stationSys.dockAtStation("player_1", "station_1");
    assertTrue(ok, "Docking succeeds when in range");
    assertTrue(stationSys.isDocked("player_1"), "Player is docked");
    assertTrue(stationSys.getDockedStation("player_1") == "station_1", "Docked at correct station");
}

void testStationDockOutOfRange() {
    std::cout << "\n=== Station Dock Out Of Range ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 500.0f);

    auto* ship = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 9999.0f;
    addComp<components::Velocity>(ship);

    bool ok = stationSys.dockAtStation("player_1", "station_1");
    assertTrue(!ok, "Docking fails when out of range");
    assertTrue(!stationSys.isDocked("player_1"), "Player is not docked");
}

void testStationUndock() {
    std::cout << "\n=== Station Undock ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);

    stationSys.dockAtStation("player_1", "station_1");
    assertTrue(stationSys.isDocked("player_1"), "Docked before undock");

    bool ok = stationSys.undockFromStation("player_1");
    assertTrue(ok, "Undock succeeds");
    assertTrue(!stationSys.isDocked("player_1"), "No longer docked after undock");
}

void testStationUndockNotDocked() {
    std::cout << "\n=== Station Undock Not Docked ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);

    bool ok = stationSys.undockFromStation("player_1");
    assertTrue(!ok, "Undock fails when not docked");
}

void testStationRepair() {
    std::cout << "\n=== Station Repair ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f, 1.0f);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);

    auto* hp = addComp<components::Health>(ship);
    hp->shield_hp = 50.0f;  hp->shield_max = 100.0f;
    hp->armor_hp  = 30.0f;  hp->armor_max  = 100.0f;
    hp->hull_hp   = 80.0f;  hp->hull_max   = 100.0f;

    auto* player = addComp<components::Player>(ship);
    player->isk = 10000.0;

    stationSys.dockAtStation("player_1", "station_1");

    double cost = stationSys.repairShip("player_1");
    // Damage = (100-50) + (100-30) + (100-80) = 50+70+20 = 140 HP, at 1 ISK/hp = 140
    assertTrue(approxEqual(static_cast<float>(cost), 140.0f), "Repair cost is 140 ISK");
    assertTrue(approxEqual(hp->shield_hp, 100.0f), "Shield fully repaired");
    assertTrue(approxEqual(hp->armor_hp, 100.0f), "Armor fully repaired");
    assertTrue(approxEqual(hp->hull_hp, 100.0f), "Hull fully repaired");
    assertTrue(approxEqual(static_cast<float>(player->isk), 9860.0f), "ISK deducted");
}

void testStationRepairNoDamage() {
    std::cout << "\n=== Station Repair No Damage ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);

    auto* hp = addComp<components::Health>(ship);
    hp->shield_hp = hp->shield_max = 100.0f;
    hp->armor_hp  = hp->armor_max  = 100.0f;
    hp->hull_hp   = hp->hull_max   = 100.0f;

    addComp<components::Player>(ship);

    stationSys.dockAtStation("player_1", "station_1");

    double cost = stationSys.repairShip("player_1");
    assertTrue(approxEqual(static_cast<float>(cost), 0.0f), "No cost when no damage");
}

void testStationRepairNotDocked() {
    std::cout << "\n=== Station Repair Not Docked ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    auto* ship = world.createEntity("player_1");
    addComp<components::Position>(ship);
    auto* hp = addComp<components::Health>(ship);
    hp->shield_hp = 50.0f; hp->shield_max = 100.0f;

    double cost = stationSys.repairShip("player_1");
    assertTrue(approxEqual(static_cast<float>(cost), 0.0f), "No repair when not docked");
}

void testStationDockedCount() {
    std::cout << "\n=== Station Docked Count ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* s1 = world.createEntity("p1");
    addComp<components::Position>(s1);
    addComp<components::Velocity>(s1);

    auto* s2 = world.createEntity("p2");
    addComp<components::Position>(s2);
    addComp<components::Velocity>(s2);

    stationSys.dockAtStation("p1", "station_1");
    stationSys.dockAtStation("p2", "station_1");

    auto* station = world.getEntity("station_1")->getComponent<components::Station>();
    assertTrue(station->docked_count == 2, "Two ships docked");

    stationSys.undockFromStation("p1");
    assertTrue(station->docked_count == 1, "One ship after undock");
}

void testStationDoubleDock() {
    std::cout << "\n=== Station Double Dock ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("p1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);

    stationSys.dockAtStation("p1", "station_1");
    bool again = stationSys.dockAtStation("p1", "station_1");
    assertTrue(!again, "Cannot dock when already docked");
}

void testStationMovementStopsOnDock() {
    std::cout << "\n=== Station Movement Stops On Dock ===" << std::endl;
    ecs::World world;
    systems::StationSystem stationSys(&world);

    stationSys.createStation("station_1", "Hub", 0, 0, 0, 5000.0f);

    auto* ship = world.createEntity("p1");
    addComp<components::Position>(ship);
    auto* vel = addComp<components::Velocity>(ship);
    vel->vx = 100.0f;
    vel->vy = 50.0f;
    vel->vz = 200.0f;

    stationSys.dockAtStation("p1", "station_1");
    assertTrue(approxEqual(vel->vx, 0.0f), "Velocity X zeroed on dock");
    assertTrue(approxEqual(vel->vy, 0.0f), "Velocity Y zeroed on dock");
    assertTrue(approxEqual(vel->vz, 0.0f), "Velocity Z zeroed on dock");
}

// ==================== WreckSalvageSystem Tests ====================

void testWreckCreate() {
    std::cout << "\n=== Wreck Create ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("dead_ship_1", 100.0f, 0.0f, 200.0f, 600.0f);
    assertTrue(!wreck_id.empty(), "Wreck created with valid id");

    auto* entity = world.getEntity(wreck_id);
    assertTrue(entity != nullptr, "Wreck entity exists");

    auto* wreck = entity->getComponent<components::Wreck>();
    assertTrue(wreck != nullptr, "Wreck component attached");
    assertTrue(wreck->source_entity_id == "dead_ship_1", "Source entity id correct");
    assertTrue(approxEqual(wreck->lifetime_remaining, 600.0f), "Lifetime is correct");
    assertTrue(!wreck->salvaged, "Not yet salvaged");
}

void testWreckLifetimeDecay() {
    std::cout << "\n=== Wreck Lifetime Decay ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    wreckSys.createWreck("ship1", 0, 0, 0, 10.0f);
    assertTrue(wreckSys.getActiveWreckCount() == 1, "One active wreck");

    wreckSys.update(5.0f);
    assertTrue(wreckSys.getActiveWreckCount() == 1, "Wreck still active after 5s");

    wreckSys.update(6.0f);
    assertTrue(wreckSys.getActiveWreckCount() == 0, "Wreck despawned after expiry");
}

void testSalvageWreckInRange() {
    std::cout << "\n=== Salvage Wreck In Range ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("ship1", 100.0f, 0.0f, 0.0f);

    // Add loot to wreck
    auto* wreck_entity = world.getEntity(wreck_id);
    auto* wreck_inv = wreck_entity->getComponent<components::Inventory>();
    components::Inventory::Item loot;
    loot.item_id = "scrap_1";
    loot.name = "Metal Scraps";
    loot.type = "salvage";
    loot.quantity = 5;
    loot.volume = 1.0f;
    wreck_inv->items.push_back(loot);

    // Create player near the wreck
    auto* player = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(player);
    pos->x = 110.0f;
    auto* inv = addComp<components::Inventory>(player);
    inv->max_capacity = 1000.0f;

    bool ok = wreckSys.salvageWreck("player_1", wreck_id, 2500.0f);
    assertTrue(ok, "Salvage succeeds when in range");

    assertTrue(inv->items.size() == 1, "Player received 1 item stack");
    assertTrue(inv->items[0].name == "Metal Scraps", "Correct item transferred");
    assertTrue(inv->items[0].quantity == 5, "Correct quantity transferred");
}

void testSalvageWreckOutOfRange() {
    std::cout << "\n=== Salvage Wreck Out Of Range ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("ship1", 0, 0, 0);

    auto* player = world.createEntity("player_1");
    auto* pos = addComp<components::Position>(player);
    pos->x = 99999.0f;
    addComp<components::Inventory>(player);

    bool ok = wreckSys.salvageWreck("player_1", wreck_id, 2500.0f);
    assertTrue(!ok, "Salvage fails when out of range");
}

void testSalvageAlreadySalvaged() {
    std::cout << "\n=== Salvage Already Salvaged ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("ship1", 0, 0, 0);

    auto* player = world.createEntity("player_1");
    addComp<components::Position>(player);
    addComp<components::Inventory>(player);

    wreckSys.salvageWreck("player_1", wreck_id, 5000.0f);
    bool again = wreckSys.salvageWreck("player_1", wreck_id, 5000.0f);
    assertTrue(!again, "Cannot salvage same wreck twice");
}

void testWreckActiveCount() {
    std::cout << "\n=== Wreck Active Count ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    wreckSys.createWreck("s1", 0, 0, 0);
    wreckSys.createWreck("s2", 100, 0, 0);
    wreckSys.createWreck("s3", 200, 0, 0);
    assertTrue(wreckSys.getActiveWreckCount() == 3, "Three active wrecks");

    // Salvage one
    auto* player = world.createEntity("player_1");
    addComp<components::Position>(player);
    addComp<components::Inventory>(player);

    auto entities = world.getAllEntities();
    std::string first_wreck;
    for (auto* e : entities) {
        if (e->getComponent<components::Wreck>()) {
            first_wreck = e->getId();
            break;
        }
    }
    wreckSys.salvageWreck("player_1", first_wreck, 999999.0f);
    assertTrue(wreckSys.getActiveWreckCount() == 2, "Two active after one salvaged");
}

void testWreckHasInventory() {
    std::cout << "\n=== Wreck Has Inventory ===" << std::endl;
    ecs::World world;
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("ship1", 0, 0, 0);
    auto* entity = world.getEntity(wreck_id);
    auto* inv = entity->getComponent<components::Inventory>();
    assertTrue(inv != nullptr, "Wreck has Inventory component");
    assertTrue(approxEqual(inv->max_capacity, 500.0f), "Wreck cargo capacity is 500 m3");
}

// ==================== ServerConsole Tests ====================

void testConsoleInit() {
    std::cout << "\n=== Console Init ===" << std::endl;
    ServerConsole console;
    // Pass dummy references — the init only stores a flag
    bool ok = console.init();
    assertTrue(ok, "Console initializes successfully");
    assertTrue(console.getCommandCount() >= 2, "Built-in commands registered (help, status)");
}

void testConsoleHelpCommand() {
    std::cout << "\n=== Console Help Command ===" << std::endl;
    ServerConsole console;
    console.init();

    std::string output = console.executeCommand("help");
    assertTrue(output.find("help") != std::string::npos, "Help output lists 'help' command");
    assertTrue(output.find("status") != std::string::npos, "Help output lists 'status' command");
}

void testConsoleStatusCommand() {
    std::cout << "\n=== Console Status Command ===" << std::endl;
    ServerConsole console;
    console.init();

    std::string output = console.executeCommand("status");
    assertTrue(output.find("Server Status") != std::string::npos, "Status output has header");
    assertTrue(output.find("Commands registered") != std::string::npos, "Status shows command count");
}

void testConsoleUnknownCommand() {
    std::cout << "\n=== Console Unknown Command ===" << std::endl;
    ServerConsole console;
    console.init();

    std::string output = console.executeCommand("foobar");
    assertTrue(output.find("Unknown command") != std::string::npos, "Unknown command error message");
}

void testConsoleCustomCommand() {
    std::cout << "\n=== Console Custom Command ===" << std::endl;
    ServerConsole console;
    console.init();

    console.registerCommand("ping", "Reply with pong",
        [](const std::vector<std::string>& /*args*/) -> std::string {
            return "pong";
        });

    std::string output = console.executeCommand("ping");
    assertTrue(output == "pong", "Custom command returns expected output");
    assertTrue(console.getCommandCount() >= 3, "Custom command registered");
}

void testConsoleLogBuffer() {
    std::cout << "\n=== Console Log Buffer ===" << std::endl;
    ServerConsole console;
    console.init();

    console.addLogMessage(utils::LogLevel::INFO, "Test message 1");
    console.addLogMessage(utils::LogLevel::INFO, "Test message 2");

    assertTrue(console.getLogBuffer().size() == 2, "Two log entries buffered");
    assertTrue(console.getLogBuffer()[0] == "Test message 1", "First log entry correct");
}

void testConsoleEmptyCommand() {
    std::cout << "\n=== Console Empty Command ===" << std::endl;
    ServerConsole console;
    console.init();

    std::string output = console.executeCommand("");
    assertTrue(output.empty(), "Empty command returns empty string");
}

void testConsoleNotInitialized() {
    std::cout << "\n=== Console Not Initialized ===" << std::endl;
    ServerConsole console;

    std::string output = console.executeCommand("help");
    assertTrue(output.find("not initialized") != std::string::npos, "Not-initialized message");
}

void testConsoleShutdown() {
    std::cout << "\n=== Console Shutdown ===" << std::endl;
    ServerConsole console;
    console.init();
    assertTrue(console.getCommandCount() >= 2, "Commands before shutdown");

    console.shutdown();
    assertTrue(console.getCommandCount() == 0, "Commands cleared after shutdown");
}

void testConsoleInteractiveMode() {
    std::cout << "\n=== Console Interactive Mode ===" << std::endl;
    ServerConsole console;
    assertTrue(!console.isInteractive(), "Default is non-interactive");
    console.setInteractive(true);
    assertTrue(console.isInteractive(), "Interactive mode set");
}

// ==================== FleetMoraleSystem Tests ====================

void testFleetMoraleRecordWin() {
    std::cout << "\n=== Fleet Morale Record Win ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    sys.recordWin("cap1");
    assertTrue(sys.getMoraleScore("cap1") > 0.0f, "Morale score positive after win");
    assertTrue(sys.getMoraleState("cap1") == "Steady", "Morale state is Steady after one win");
}

void testFleetMoraleRecordLoss() {
    std::cout << "\n=== Fleet Morale Record Loss ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    sys.recordLoss("cap1");
    assertTrue(sys.getMoraleScore("cap1") < 0.0f, "Morale score negative after loss");
}

void testFleetMoraleMultipleEvents() {
    std::cout << "\n=== Fleet Morale Multiple Events ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    for (int i = 0; i < 10; i++) {
        sys.recordWin("cap1");
    }
    // 10 wins * 1.0 = 10, but let's accumulate: each recordWin increments wins
    // After 10 wins: score = 10 * 1.0 = 10 ... need >= 50
    // Actually wins accumulate: after 10 calls, wins=10, score=10. Need 50 wins for 50.
    for (int i = 0; i < 40; i++) {
        sys.recordWin("cap1");
    }
    assertTrue(sys.getMoraleScore("cap1") >= 50.0f, "Morale >= 50 after 50 wins");
    assertTrue(sys.getMoraleState("cap1") == "Inspired", "Morale state Inspired at high morale");
}

void testFleetMoraleLossStreak() {
    std::cout << "\n=== Fleet Morale Loss Streak ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    for (int i = 0; i < 5; i++) {
        sys.recordLoss("cap1");
    }
    sys.recordShipLost("cap1");
    sys.recordShipLost("cap1");
    // score = 0*1 - 5*1.5 - 2*2.0 + 0 = -11.5 => Doubtful
    std::string state = sys.getMoraleState("cap1");
    assertTrue(state == "Doubtful" || state == "Disengaged",
               "Morale state Doubtful or Disengaged after losses");
}

void testFleetMoraleSavedByPlayer() {
    std::cout << "\n=== Fleet Morale Saved By Player ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    world.createEntity("cap1");
    sys.recordSavedByPlayer("cap1");
    assertTrue(sys.getMoraleScore("cap1") > 0.0f, "Saved by player increases morale");
}

void testFleetMoraleMissionTogether() {
    std::cout << "\n=== Fleet Morale Mission Together ===" << std::endl;
    ecs::World world;
    systems::FleetMoraleSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    sys.recordMissionTogether("cap1");
    auto* morale = entity->getComponent<components::FleetMorale>();
    assertTrue(morale != nullptr, "FleetMorale component created");
    assertTrue(morale->missions_together == 1, "Missions together counter incremented");
}

// ==================== CaptainPersonalitySystem Tests ====================

void testCaptainPersonalityAssign() {
    std::cout << "\n=== Captain Personality Assign ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Solari");
    float agg = sys.getPersonalityTrait("cap1", "aggression");
    float soc = sys.getPersonalityTrait("cap1", "sociability");
    float opt = sys.getPersonalityTrait("cap1", "optimism");
    float pro = sys.getPersonalityTrait("cap1", "professionalism");
    assertTrue(agg >= 0.0f && agg <= 1.0f, "Aggression in valid range");
    assertTrue(soc >= 0.0f && soc <= 1.0f, "Sociability in valid range");
    assertTrue(opt >= 0.0f && opt <= 1.0f, "Optimism in valid range");
    assertTrue(pro >= 0.0f && pro <= 1.0f, "Professionalism in valid range");
}

void testCaptainPersonalityFactionTraits() {
    std::cout << "\n=== Captain Personality Faction Traits ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "Keldari_Captain", "Keldari");
    float agg = sys.getPersonalityTrait("cap1", "aggression");
    assertTrue(agg > 0.5f, "Keldari captain has high aggression");
}

void testCaptainPersonalitySetTrait() {
    std::cout << "\n=== Captain Personality Set Trait ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Solari");
    sys.setPersonalityTrait("cap1", "aggression", 0.9f);
    assertTrue(approxEqual(sys.getPersonalityTrait("cap1", "aggression"), 0.9f),
               "Set trait reads back correctly");
}

void testCaptainPersonalityGetFaction() {
    std::cout << "\n=== Captain Personality Get Faction ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Veyren");
    assertTrue(sys.getCaptainFaction("cap1") == "Veyren", "Faction returned correctly");
}

void testCaptainPersonalityDeterministic() {
    std::cout << "\n=== Captain Personality Deterministic ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Aurelian");
    float agg1 = sys.getPersonalityTrait("cap1", "aggression");
    float soc1 = sys.getPersonalityTrait("cap1", "sociability");
    // Assign again - should get same result (deterministic)
    sys.assignPersonality("cap1", "TestCaptain", "Aurelian");
    float agg2 = sys.getPersonalityTrait("cap1", "aggression");
    float soc2 = sys.getPersonalityTrait("cap1", "sociability");
    assertTrue(approxEqual(agg1, agg2), "Aggression is deterministic");
    assertTrue(approxEqual(soc1, soc2), "Sociability is deterministic");
}

// ==================== FleetChatterSystem Tests ====================

void testFleetChatterSetActivity() {
    std::cout << "\n=== Fleet Chatter Set Activity ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    sys.setActivity("cap1", "Mining");
    auto* chatter = entity->getComponent<components::FleetChatterState>();
    assertTrue(chatter != nullptr, "FleetChatterState component created");
    assertTrue(chatter->current_activity == "Mining", "Activity set to Mining");
}

void testFleetChatterGetLine() {
    std::cout << "\n=== Fleet Chatter Get Line ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    addComp<components::FleetMorale>(entity);
    sys.setActivity("cap1", "Mining");
    std::string line = sys.getNextChatterLine("cap1");
    assertTrue(!line.empty(), "Chatter line is non-empty");
}

void testFleetChatterCooldown() {
    std::cout << "\n=== Fleet Chatter Cooldown ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Idle");
    sys.getNextChatterLine("cap1");
    std::string line2 = sys.getNextChatterLine("cap1");
    assertTrue(line2.empty(), "Second line empty due to cooldown");
}

void testFleetChatterLinesSpoken() {
    std::cout << "\n=== Fleet Chatter Lines Spoken ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Combat");
    sys.getNextChatterLine("cap1");
    assertTrue(sys.getTotalLinesSpoken("cap1") == 1, "Total lines spoken is 1");
}

void testFleetChatterCooldownExpires() {
    std::cout << "\n=== Fleet Chatter Cooldown Expires ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(entity);
    addComp<components::FleetChatterState>(entity);
    sys.setActivity("cap1", "Warp");
    sys.getNextChatterLine("cap1");
    assertTrue(sys.isOnCooldown("cap1"), "On cooldown after speaking");
    sys.update(60.0f);
    assertTrue(!sys.isOnCooldown("cap1"), "Cooldown expired after 60s");
    std::string line = sys.getNextChatterLine("cap1");
    assertTrue(!line.empty(), "Can speak again after cooldown expires");
}

// ==================== WarpAnomalySystem Tests ====================

void testWarpAnomalyNoneIfNotCruising() {
    std::cout << "\n=== Warp Anomaly None If Not Cruising ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Align;
    warp->warp_time = 5.0f;
    // tryTriggerAnomaly checks warp_time < 20, not phase; update() checks phase
    // With short warp_time and non-cruise phase, no anomaly via update
    sys.update(1.0f);
    assertTrue(sys.getAnomalyCount("ship1") == 0, "No anomaly when not in Cruise phase");
}

void testWarpAnomalyNoneIfShortWarp() {
    std::cout << "\n=== Warp Anomaly None If Short Warp ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    warp->warp_time = 5.0f;
    bool triggered = sys.tryTriggerAnomaly("ship1");
    assertTrue(!triggered, "No anomaly when warp_time < 20");
}

void testWarpAnomalyTriggersOnLongWarp() {
    std::cout << "\n=== Warp Anomaly Triggers On Long Warp ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    // Try many different warp_time values to find one that triggers
    bool any_triggered = false;
    for (int i = 20; i < 300; i++) {
        warp->warp_time = static_cast<float>(i);
        if (sys.tryTriggerAnomaly("ship1")) {
            any_triggered = true;
            break;
        }
    }
    assertTrue(any_triggered, "At least one anomaly triggered on long warp");
}

void testWarpAnomalyCount() {
    std::cout << "\n=== Warp Anomaly Count ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    int triggered_count = 0;
    for (int i = 20; i < 500; i++) {
        warp->warp_time = static_cast<float>(i);
        if (sys.tryTriggerAnomaly("ship1")) {
            triggered_count++;
        }
    }
    assertTrue(sys.getAnomalyCount("ship1") == triggered_count,
               "getAnomalyCount matches triggered count");
}

void testWarpAnomalyClear() {
    std::cout << "\n=== Warp Anomaly Clear ===" << std::endl;
    ecs::World world;
    systems::WarpAnomalySystem sys(&world);
    auto* entity = world.createEntity("ship1");
    auto* warp = addComp<components::WarpState>(entity);
    warp->phase = components::WarpState::WarpPhase::Cruise;
    for (int i = 20; i < 300; i++) {
        warp->warp_time = static_cast<float>(i);
        if (sys.tryTriggerAnomaly("ship1")) break;
    }
    sys.clearAnomaly("ship1");
    auto cleared = sys.getLastAnomaly("ship1");
    assertTrue(cleared.name.empty(), "Anomaly cleared successfully");
}

// ==================== CaptainRelationshipSystem Tests ====================

void testCaptainRelationshipRecordEvent() {
    std::cout << "\n=== Captain Relationship Record Event ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    sys.recordEvent("cap1", "cap2", "saved_in_combat");
    assertTrue(sys.getAffinity("cap1", "cap2") > 0.0f,
               "Affinity positive after saved_in_combat");
}

void testCaptainRelationshipAbandoned() {
    std::cout << "\n=== Captain Relationship Abandoned ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    sys.recordEvent("cap1", "cap2", "abandoned");
    assertTrue(sys.getAffinity("cap1", "cap2") < 0.0f,
               "Affinity negative after abandoned");
}

void testCaptainRelationshipStatus() {
    std::cout << "\n=== Captain Relationship Status Friend ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    // saved_in_combat gives +10 each, need >50
    for (int i = 0; i < 6; i++) {
        sys.recordEvent("cap1", "cap2", "saved_in_combat");
    }
    assertTrue(sys.getRelationshipStatus("cap1", "cap2") == "Friend",
               "Status is Friend with high affinity");
}

void testCaptainRelationshipGrudge() {
    std::cout << "\n=== Captain Relationship Grudge ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    // abandoned gives -20 each, need < -50
    for (int i = 0; i < 3; i++) {
        sys.recordEvent("cap1", "cap2", "abandoned");
    }
    assertTrue(sys.getRelationshipStatus("cap1", "cap2") == "Grudge",
               "Status is Grudge with very negative affinity");
}

void testCaptainRelationshipMultipleEvents() {
    std::cout << "\n=== Captain Relationship Multiple Events ===" << std::endl;
    ecs::World world;
    systems::CaptainRelationshipSystem sys(&world);
    world.createEntity("cap1");
    world.createEntity("cap2");
    sys.recordEvent("cap1", "cap2", "saved_in_combat");  // +10
    sys.recordEvent("cap1", "cap2", "abandoned");         // -20
    sys.recordEvent("cap1", "cap2", "shared_victory");    // +5
    // Net: -5
    float affinity = sys.getAffinity("cap1", "cap2");
    assertTrue(approxEqual(affinity, -5.0f), "Net affinity reflects mixed events");
}

// ==================== EmotionalArcSystem Tests ====================

void testEmotionalArcVictory() {
    std::cout << "\n=== Emotional Arc Victory ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("cap1");
    float baseline = sys.getConfidence("cap1");
    sys.onCombatVictory("cap1");
    assertTrue(sys.getConfidence("cap1") > baseline, "Confidence increased after victory");
}

void testEmotionalArcDefeat() {
    std::cout << "\n=== Emotional Arc Defeat ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::EmotionalState>(entity);
    float baseline_conf = sys.getConfidence("cap1");
    float baseline_fat = sys.getFatigue("cap1");
    sys.onCombatDefeat("cap1");
    assertTrue(sys.getConfidence("cap1") < baseline_conf, "Confidence decreased after defeat");
    assertTrue(sys.getFatigue("cap1") > baseline_fat, "Fatigue increased after defeat");
}

void testEmotionalArcRest() {
    std::cout << "\n=== Emotional Arc Rest ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    auto* state = addComp<components::EmotionalState>(entity);
    state->fatigue = 50.0f;
    sys.onRest("cap1");
    assertTrue(state->fatigue < 50.0f, "Fatigue decreased after rest");
}

void testEmotionalArcTrust() {
    std::cout << "\n=== Emotional Arc Trust ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::EmotionalState>(entity);
    float baseline = sys.getTrust("cap1");
    sys.onPlayerTrust("cap1");
    assertTrue(sys.getTrust("cap1") > baseline, "Trust increased after player trust");
}

void testEmotionalArcBetray() {
    std::cout << "\n=== Emotional Arc Betray ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::EmotionalState>(entity);
    float baseline = sys.getTrust("cap1");
    sys.onPlayerBetray("cap1");
    assertTrue(sys.getTrust("cap1") < baseline, "Trust decreased after betrayal");
}

// ==================== FleetCargoSystem Tests ====================

void testFleetCargoAddContributor() {
    std::cout << "\n=== Fleet Cargo Add Contributor ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 400.0f;
    sys.addContributor("pool1", "ship1");
    sys.recalculate("pool1");
    assertTrue(sys.getTotalCapacity("pool1") == 400, "Total capacity is 400 after adding ship");
}

void testFleetCargoRemoveContributor() {
    std::cout << "\n=== Fleet Cargo Remove Contributor ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 400.0f;
    sys.addContributor("pool1", "ship1");
    sys.removeContributor("pool1", "ship1");
    assertTrue(sys.getTotalCapacity("pool1") == 0, "Total capacity 0 after removing ship");
}

void testFleetCargoMultipleShips() {
    std::cout << "\n=== Fleet Cargo Multiple Ships ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    for (int i = 0; i < 3; i++) {
        std::string sid = "ship" + std::to_string(i);
        auto* ship = world.createEntity(sid);
        auto* inv = addComp<components::Inventory>(ship);
        inv->max_capacity = 200.0f;
        sys.addContributor("pool1", sid);
    }
    sys.recalculate("pool1");
    assertTrue(sys.getTotalCapacity("pool1") == 600, "Aggregate capacity of 3 ships is 600");
}

void testFleetCargoUsedCapacity() {
    std::cout << "\n=== Fleet Cargo Used Capacity ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 400.0f;
    components::Inventory::Item item;
    item.item_id = "ore1";
    item.name = "Veldspar";
    item.type = "ore";
    item.quantity = 10;
    item.volume = 5.0f;
    inv->items.push_back(item);
    sys.addContributor("pool1", "ship1");
    sys.recalculate("pool1");
    assertTrue(sys.getUsedCapacity("pool1") == 50, "Used capacity reflects items (10*5=50)");
}

void testFleetCargoGetCapacity() {
    std::cout << "\n=== Fleet Cargo Get Capacity ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 300.0f;
    sys.addContributor("pool1", "ship1");
    assertTrue(sys.getTotalCapacity("pool1") == 300, "getTotalCapacity query returns 300");
}

// ==================== TacticalOverlaySystem Tests ====================

void testTacticalOverlayToggle() {
    std::cout << "\n=== Tactical Overlay Toggle ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);
    sys.toggleOverlay("player1");
    assertTrue(sys.isEnabled("player1"), "Overlay enabled after toggle");
}

void testTacticalOverlayToggleTwice() {
    std::cout << "\n=== Tactical Overlay Toggle Twice ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);
    sys.toggleOverlay("player1");
    sys.toggleOverlay("player1");
    assertTrue(!sys.isEnabled("player1"), "Overlay disabled after double toggle");
}

void testTacticalOverlaySetToolRange() {
    std::cout << "\n=== Tactical Overlay Set Tool Range ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    auto* overlay = addComp<components::TacticalOverlayState>(entity);
    sys.setToolRange("player1", 5000.0f, "weapon");
    assertTrue(approxEqual(overlay->tool_range, 5000.0f), "Tool range set to 5000");
}

void testTacticalOverlayRingDistances() {
    std::cout << "\n=== Tactical Overlay Ring Distances ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);
    std::vector<float> custom = {10.0f, 25.0f, 50.0f};
    sys.setRingDistances("player1", custom);
    auto result = sys.getRingDistances("player1");
    assertTrue(result.size() == 3, "Ring distances has 3 entries");
    assertTrue(approxEqual(result[0], 10.0f), "First ring distance is 10");
    assertTrue(approxEqual(result[2], 50.0f), "Third ring distance is 50");
}

void testTacticalOverlayDefaultRings() {
    std::cout << "\n=== Tactical Overlay Default Rings ===" << std::endl;
    ecs::World world;
    systems::TacticalOverlaySystem sys(&world);
    auto* entity = world.createEntity("player1");
    addComp<components::TacticalOverlayState>(entity);
    auto rings = sys.getRingDistances("player1");
    assertTrue(rings.size() == 6, "Default ring distances has 6 entries");
    assertTrue(approxEqual(rings[0], 5.0f), "Default first ring is 5.0");
    assertTrue(approxEqual(rings[5], 100.0f), "Default last ring is 100.0");
}

// ==================== Damage Event Tests ====================

void testDamageEventOnShieldHit() {
    std::cout << "\n=== Damage Event On Shield Hit ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 500.0f;
    health->shield_max = 500.0f;
    health->armor_hp = 300.0f;
    health->armor_max = 300.0f;
    health->hull_hp = 200.0f;
    health->hull_max = 200.0f;

    combatSys.applyDamage("target1", 50.0f, "kinetic");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent created on damage");
    assertTrue(dmgEvent->recent_hits.size() == 1, "One hit recorded");
    assertTrue(dmgEvent->recent_hits[0].layer_hit == "shield", "Hit registered on shield layer");
    assertTrue(approxEqual(dmgEvent->recent_hits[0].damage_amount, 50.0f), "Damage amount recorded");
    assertTrue(dmgEvent->recent_hits[0].damage_type == "kinetic", "Damage type recorded");
    assertTrue(!dmgEvent->recent_hits[0].shield_depleted, "Shield not depleted");
}

void testDamageEventShieldDepleted() {
    std::cout << "\n=== Damage Event Shield Depleted ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 20.0f;
    health->shield_max = 500.0f;
    health->armor_hp = 300.0f;
    health->armor_max = 300.0f;
    health->hull_hp = 200.0f;
    health->hull_max = 200.0f;

    // Apply 50 damage; 20 to shield (depletes) + 30 overflows to armor
    combatSys.applyDamage("target1", 50.0f, "kinetic");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent created");
    assertTrue(dmgEvent->recent_hits.size() == 1, "One hit recorded");
    assertTrue(dmgEvent->recent_hits[0].shield_depleted, "Shield depleted flag set");
    assertTrue(approxEqual(health->shield_hp, 0.0f), "Shield HP is 0");
}

void testDamageEventHullCritical() {
    std::cout << "\n=== Damage Event Hull Critical ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 0.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 0.0f;
    health->armor_max = 100.0f;
    health->hull_hp = 100.0f;
    health->hull_max = 100.0f;

    // Hit hull for 80 damage, leaving 20 HP (20% < 25% threshold)
    combatSys.applyDamage("target1", 80.0f, "explosive");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent created");
    assertTrue(dmgEvent->recent_hits[0].hull_critical, "Hull critical flag set (below 25%)");
    assertTrue(dmgEvent->recent_hits[0].layer_hit == "hull", "Hit on hull layer");
}

void testDamageEventMultipleHits() {
    std::cout << "\n=== Damage Event Multiple Hits ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 500.0f;
    health->shield_max = 500.0f;
    health->armor_hp = 300.0f;
    health->armor_max = 300.0f;

    combatSys.applyDamage("target1", 10.0f, "em");
    combatSys.applyDamage("target1", 20.0f, "thermal");
    combatSys.applyDamage("target1", 30.0f, "kinetic");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent exists");
    assertTrue(dmgEvent->recent_hits.size() == 3, "Three hits recorded");
    assertTrue(approxEqual(dmgEvent->total_damage_taken, 60.0f), "Total damage tracked");
}

void testDamageEventClearOldHits() {
    std::cout << "\n=== Damage Event Clear Old Hits ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* target = world.createEntity("target1");
    auto* health = addComp<components::Health>(target);
    health->shield_hp = 500.0f;
    health->shield_max = 500.0f;

    combatSys.applyDamage("target1", 10.0f, "em");

    auto* dmgEvent = target->getComponent<components::DamageEvent>();
    assertTrue(dmgEvent != nullptr, "DamageEvent exists");
    assertTrue(dmgEvent->recent_hits.size() == 1, "One hit before clear");

    // Clear with a future timestamp beyond max_age
    dmgEvent->clearOldHits(100.0f, 5.0f);
    assertTrue(dmgEvent->recent_hits.size() == 0, "Old hits cleared");
}

// ==================== AI Retreat Tests ====================

void testAIFleeOnLowHealth() {
    std::cout << "\n=== AI Flee On Low Health ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state = components::AI::State::Attacking;
    ai->target_entity_id = "player1";
    ai->flee_threshold = 0.25f;

    auto* health = addComp<components::Health>(npc);
    health->shield_hp = 0.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 0.0f;
    health->armor_max = 100.0f;
    health->hull_hp = 50.0f;   // 50 out of 300 total = 16.7% < 25%
    health->hull_max = 100.0f;

    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;

    auto* player = world.createEntity("player1");
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 1000.0f; playerPos->y = 0.0f; playerPos->z = 0.0f;
    addComp<components::Player>(player);

    aiSys.update(0.1f);

    assertTrue(ai->state == components::AI::State::Fleeing, "NPC flees when health below threshold");
}

void testAINoFleeAboveThreshold() {
    std::cout << "\n=== AI No Flee Above Threshold ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state = components::AI::State::Attacking;
    ai->target_entity_id = "player1";
    ai->flee_threshold = 0.25f;

    auto* health = addComp<components::Health>(npc);
    health->shield_hp = 50.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 50.0f;
    health->armor_max = 100.0f;
    health->hull_hp = 100.0f;   // 200 out of 300 total = 66.7% > 25%
    health->hull_max = 100.0f;

    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;

    auto* player = world.createEntity("player1");
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 1000.0f; playerPos->y = 0.0f; playerPos->z = 0.0f;
    addComp<components::Player>(player);

    // NPC needs a weapon to stay in attacking state (orbitBehavior checks for weapon)
    addComp<components::Weapon>(npc);

    aiSys.update(0.1f);

    assertTrue(ai->state != components::AI::State::Fleeing, "NPC does not flee when health above threshold");
}

void testAIFleeThresholdCustom() {
    std::cout << "\n=== AI Flee Threshold Custom ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc1");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state = components::AI::State::Attacking;
    ai->target_entity_id = "player1";
    ai->flee_threshold = 0.50f;  // Custom high threshold

    auto* health = addComp<components::Health>(npc);
    health->shield_hp = 30.0f;
    health->shield_max = 100.0f;
    health->armor_hp = 30.0f;
    health->armor_max = 100.0f;
    health->hull_hp = 80.0f;   // 140 out of 300 = 46.7% < 50%
    health->hull_max = 100.0f;

    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    auto* vel = addComp<components::Velocity>(npc);
    vel->max_speed = 100.0f;

    auto* player = world.createEntity("player1");
    auto* playerPos = addComp<components::Position>(player);
    playerPos->x = 1000.0f; playerPos->y = 0.0f; playerPos->z = 0.0f;
    addComp<components::Player>(player);

    aiSys.update(0.1f);

    assertTrue(ai->state == components::AI::State::Fleeing, "NPC flees with custom high threshold");
}

// ==================== Warp State Phase Tests ====================

void testWarpStatePhaseAlign() {
    std::cout << "\n=== Warp State Phase Align ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);

    bool warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(warped, "Warp initiated");
    assertTrue(warpState->phase == components::WarpState::WarpPhase::Align, "Initial phase is Align");
}

void testWarpStatePhaseCruise() {
    std::cout << "\n=== Warp State Phase Cruise ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);

    moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);

    // Warp duration ~2.5s at 200km with no Ship component (align_time=2.5 + ~0 travel)
    // Progress rate = 1/2.5 = 0.4/s; Cruise phase at progress 0.2-0.85
    // Need 0.5s+ elapsed for progress >= 0.2 (entry to cruise)
    for (int i = 0; i < 8; i++) {
        moveSys.update(0.1f);  // 0.8 seconds total, progress ~0.32
    }

    assertTrue(warpState->phase == components::WarpState::WarpPhase::Cruise, "Phase is Cruise at 30% progress");
    assertTrue(warpState->warp_time > 0.0f, "Warp time is tracking");
}

void testWarpStatePhaseExit() {
    std::cout << "\n=== Warp State Phase Exit ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);

    moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);

    // Warp duration ~2.5s; Exit phase at progress > 0.85 = 2.125s elapsed
    // 22 updates of 0.1s = 2.2s, progress ~0.88 (in exit phase)
    for (int i = 0; i < 22; i++) {
        moveSys.update(0.1f);  // 2.2 seconds total
    }

    assertTrue(warpState->phase == components::WarpState::WarpPhase::Exit, "Phase is Exit near completion");
}

void testWarpStateResetOnArrival() {
    std::cout << "\n=== Warp State Reset On Arrival ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);

    moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);

    // Complete the warp (10+ seconds at 0.1 progress/s)
    for (int i = 0; i < 110; i++) {
        moveSys.update(0.1f);  // 11.0 seconds total
    }

    assertTrue(warpState->phase == components::WarpState::WarpPhase::None, "Phase reset to None after arrival");
    assertTrue(approxEqual(warpState->warp_time, 0.0f), "Warp time reset");
    assertTrue(approxEqual(pos->x, 200000.0f), "Ship arrived at destination X");
}

void testWarpStateIntensity() {
    std::cout << "\n=== Warp State Intensity ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    warpState->mass_norm = 0.5f;  // medium-mass ship

    moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);

    // Advance a few ticks
    moveSys.update(1.0f);

    assertTrue(warpState->intensity > 0.0f, "Intensity increases during warp");
    assertTrue(warpState->intensity <= 1.0f, "Intensity clamped to max 1.0");
}

// ── Warp Disruption Tests ──────────────────────────────────────────

void testWarpDisruptionPreventsWarp() {
    std::cout << "\n=== Warp Disruption Prevents Warp ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->warp_strength = 1;  // default warp core strength

    // Apply disruption equal to warp strength
    warpState->warp_disrupt_strength = 1;

    bool warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(!warped, "Warp rejected when disrupted (strength 1 >= warp core 1)");
}

void testWarpDisruptionInsufficientStrength() {
    std::cout << "\n=== Warp Disruption Insufficient Strength ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->warp_strength = 3;  // high warp core strength (e.g., warp core stabilizer fitted)

    // Apply disruption less than warp strength
    warpState->warp_disrupt_strength = 2;

    bool warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(warped, "Warp succeeds when disruption (2) < warp core strength (3)");
}

void testIsWarpDisruptedQuery() {
    std::cout << "\n=== Is Warp Disrupted Query ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    addComp<components::Position>(ship);
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->warp_strength = 1;

    warpState->warp_disrupt_strength = 0;
    assertTrue(!moveSys.isWarpDisrupted("ship1"), "Not disrupted with 0 disrupt strength");

    warpState->warp_disrupt_strength = 1;
    assertTrue(moveSys.isWarpDisrupted("ship1"), "Disrupted when disrupt strength >= warp core");

    warpState->warp_disrupt_strength = 3;
    assertTrue(moveSys.isWarpDisrupted("ship1"), "Disrupted when disrupt strength exceeds warp core");
}

// ── Ship-Class Warp Speed Tests ────────────────────────────────────

void testWarpSpeedFromShipComponent() {
    std::cout << "\n=== Warp Speed From Ship Component ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    auto* warpState = addComp<components::WarpState>(ship);
    auto* shipComp = addComp<components::Ship>(ship);
    shipComp->warp_speed_au = 5.0f;   // frigate
    shipComp->align_time = 2.5f;

    bool warped = moveSys.commandWarp("ship1", 200000.0f, 0.0f, 0.0f);
    assertTrue(warped, "Warp initiated with frigate speed");
    assertTrue(warpState->warp_speed == 5.0f, "WarpState.warp_speed set from Ship component");
}

void testBattleshipSlowerWarp() {
    std::cout << "\n=== Battleship Slower Warp ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    // Frigate ship
    auto* frig = world.createEntity("frigate1");
    auto* fpos = addComp<components::Position>(frig);
    fpos->x = 0.0f; fpos->y = 0.0f; fpos->z = 0.0f;
    addComp<components::Velocity>(frig);
    auto* fws = addComp<components::WarpState>(frig);
    auto* fs = addComp<components::Ship>(frig);
    fs->ship_class = "Frigate";
    fs->warp_speed_au = 5.0f;
    fs->align_time = 2.5f;

    // Battleship
    auto* bs = world.createEntity("bs1");
    auto* bpos = addComp<components::Position>(bs);
    bpos->x = 0.0f; bpos->y = 0.0f; bpos->z = 0.0f;
    addComp<components::Velocity>(bs);
    auto* bws = addComp<components::WarpState>(bs);
    auto* bsc = addComp<components::Ship>(bs);
    bsc->ship_class = "Battleship";
    bsc->warp_speed_au = 2.0f;
    bsc->align_time = 10.0f;

    moveSys.commandWarp("frigate1", 200000.0f, 0.0f, 0.0f);
    moveSys.commandWarp("bs1", 200000.0f, 0.0f, 0.0f);

    // Advance enough for frigate to complete warp (align_time=2.5s + ~0 travel ≈ 2.5s)
    for (int i = 0; i < 30; i++) {
        moveSys.update(0.1f);  // 3.0 seconds total
    }

    // Frigate should have arrived (warp_duration ≈ 2.5s, 3.0s > 2.5s)
    assertTrue(fws->phase == components::WarpState::WarpPhase::None,
               "Frigate arrived (faster warp)");

    // Battleship should still be in warp (warp_duration ≈ 10.0s, 3.0s < 10.0s)
    assertTrue(bws->phase != components::WarpState::WarpPhase::None,
               "Battleship still warping (slower warp)");
}

void testWarpNoDisruptionWithoutWarpState() {
    std::cout << "\n=== Warp No Disruption Without WarpState ===" << std::endl;

    ecs::World world;
    systems::MovementSystem moveSys(&world);

    auto* ship = world.createEntity("ship1");
    auto* pos = addComp<components::Position>(ship);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(ship);
    // No WarpState component - disruption check should return false

    assertTrue(!moveSys.isWarpDisrupted("ship1"), "No disruption without WarpState component");
}

// ── AI Dynamic Orbit Tests ──────────────────────────────────────────

void testAIDynamicOrbitFrigate() {
    std::cout << "\n=== AI Dynamic Orbit Frigate ===" << std::endl;

    float dist = systems::AISystem::orbitDistanceForClass("Frigate");
    assertTrue(dist == 5000.0f, "Frigate orbit distance is 5000m");

    float dist2 = systems::AISystem::orbitDistanceForClass("Destroyer");
    assertTrue(dist2 == 5000.0f, "Destroyer orbit distance is 5000m");
}

void testAIDynamicOrbitCruiser() {
    std::cout << "\n=== AI Dynamic Orbit Cruiser ===" << std::endl;

    float dist = systems::AISystem::orbitDistanceForClass("Cruiser");
    assertTrue(dist == 15000.0f, "Cruiser orbit distance is 15000m");

    float dist2 = systems::AISystem::orbitDistanceForClass("Battlecruiser");
    assertTrue(dist2 == 15000.0f, "Battlecruiser orbit distance is 15000m");
}

void testAIDynamicOrbitBattleship() {
    std::cout << "\n=== AI Dynamic Orbit Battleship ===" << std::endl;

    float dist = systems::AISystem::orbitDistanceForClass("Battleship");
    assertTrue(dist == 30000.0f, "Battleship orbit distance is 30000m");
}

void testAIDynamicOrbitCapital() {
    std::cout << "\n=== AI Dynamic Orbit Capital ===" << std::endl;

    float distCarrier = systems::AISystem::orbitDistanceForClass("Carrier");
    assertTrue(distCarrier == 50000.0f, "Carrier orbit distance is 50000m");

    float distTitan = systems::AISystem::orbitDistanceForClass("Titan");
    assertTrue(distTitan == 50000.0f, "Titan orbit distance is 50000m");
}

void testAIDynamicOrbitUnknown() {
    std::cout << "\n=== AI Dynamic Orbit Unknown ===" << std::endl;

    float dist = systems::AISystem::orbitDistanceForClass("SomeUnknownClass");
    assertTrue(dist == 10000.0f, "Unknown class orbit distance is 10000m default");
}

void testAIEngagementRangeFromWeapon() {
    std::cout << "\n=== AI Engagement Range From Weapon ===" << std::endl;

    ecs::World world;
    auto* npc = world.createEntity("npc_test");
    auto* weapon = addComp<components::Weapon>(npc);
    weapon->optimal_range = 5000.0f;
    weapon->falloff_range = 2500.0f;

    float range = systems::AISystem::engagementRangeFromWeapon(npc);
    assertTrue(range == 7500.0f, "Engagement range is optimal + falloff");
}

void testAIEngagementRangeNoWeapon() {
    std::cout << "\n=== AI Engagement Range No Weapon ===" << std::endl;

    ecs::World world;
    auto* npc = world.createEntity("npc_no_weapon");

    float range = systems::AISystem::engagementRangeFromWeapon(npc);
    assertTrue(range == 0.0f, "No weapon returns 0 engagement range");
}

void testAITargetSelectionClosest() {
    std::cout << "\n=== AI Target Selection Closest ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    // Create NPC
    auto* npc = world.createEntity("npc_selector");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->target_selection = components::AI::TargetSelection::Closest;
    ai->awareness_range = 50000.0f;
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f; pos->y = 0.0f; pos->z = 0.0f;
    addComp<components::Velocity>(npc);

    // Create two players at different distances
    auto* far_player = world.createEntity("player_far");
    auto* far_pos = addComp<components::Position>(far_player);
    far_pos->x = 10000.0f;
    addComp<components::Player>(far_player);

    auto* near_player = world.createEntity("player_near");
    auto* near_pos = addComp<components::Position>(near_player);
    near_pos->x = 3000.0f;
    addComp<components::Player>(near_player);

    // Run AI - should pick nearest
    aiSys.update(1.0f);
    assertTrue(ai->target_entity_id == "player_near",
               "Closest selection picks nearest player");
}

void testAITargetSelectionLowestHP() {
    std::cout << "\n=== AI Target Selection Lowest HP ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc_hp_select");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->target_selection = components::AI::TargetSelection::LowestHP;
    ai->awareness_range = 50000.0f;
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f;
    addComp<components::Velocity>(npc);

    // Player at 5000m with full HP
    auto* full_hp = world.createEntity("player_full");
    auto* fpos = addComp<components::Position>(full_hp);
    fpos->x = 5000.0f;
    addComp<components::Player>(full_hp);
    auto* fh = addComp<components::Health>(full_hp);
    fh->shield_hp = 100.0f; fh->shield_max = 100.0f;
    fh->armor_hp = 100.0f; fh->armor_max = 100.0f;
    fh->hull_hp = 100.0f; fh->hull_max = 100.0f;

    // Player at 3000m with low HP
    auto* low_hp = world.createEntity("player_low");
    auto* lpos = addComp<components::Position>(low_hp);
    lpos->x = 3000.0f;
    addComp<components::Player>(low_hp);
    auto* lh = addComp<components::Health>(low_hp);
    lh->shield_hp = 0.0f; lh->shield_max = 100.0f;
    lh->armor_hp = 10.0f; lh->armor_max = 100.0f;
    lh->hull_hp = 50.0f; lh->hull_max = 100.0f;

    aiSys.update(1.0f);
    assertTrue(ai->target_entity_id == "player_low",
               "LowestHP selection picks damaged player");
}

void testAIDynamicOrbitApplied() {
    std::cout << "\n=== AI Dynamic Orbit Applied ===" << std::endl;

    ecs::World world;
    systems::AISystem aiSys(&world);

    auto* npc = world.createEntity("npc_orbit_test");
    auto* ai = addComp<components::AI>(npc);
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->use_dynamic_orbit = true;
    ai->orbit_distance = 1000.0f;  // default, should be overridden
    ai->awareness_range = 50000.0f;
    auto* pos = addComp<components::Position>(npc);
    pos->x = 0.0f;
    addComp<components::Velocity>(npc);
    auto* ship = addComp<components::Ship>(npc);
    ship->ship_class = "Battleship";

    // Create a player in range
    auto* player = world.createEntity("player_orb");
    auto* ppos = addComp<components::Position>(player);
    ppos->x = 40000.0f;
    addComp<components::Player>(player);

    aiSys.update(1.0f);
    assertTrue(ai->orbit_distance == 30000.0f,
               "Dynamic orbit updates to battleship distance (30000m)");
}

// ── Protocol Message Tests ──────────────────────────────────────────

void testProtocolDockMessages() {
    std::cout << "\n=== Protocol Dock Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Dock success
    std::string dockSuccess = proto.createDockSuccess("station_01");
    assertTrue(dockSuccess.find("dock_success") != std::string::npos,
               "Dock success contains message type");
    assertTrue(dockSuccess.find("station_01") != std::string::npos,
               "Dock success contains station ID");

    // Dock failed
    std::string dockFailed = proto.createDockFailed("out of range");
    assertTrue(dockFailed.find("dock_failed") != std::string::npos,
               "Dock failed contains message type");
    assertTrue(dockFailed.find("out of range") != std::string::npos,
               "Dock failed contains reason");
}

void testProtocolUndockMessage() {
    std::cout << "\n=== Protocol Undock Message ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string undock = proto.createUndockSuccess();
    assertTrue(undock.find("undock_success") != std::string::npos,
               "Undock success contains message type");
}

void testProtocolRepairMessage() {
    std::cout << "\n=== Protocol Repair Message ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string repair = proto.createRepairResult(5000.0f, 100.0f, 100.0f, 100.0f);
    assertTrue(repair.find("repair_result") != std::string::npos,
               "Repair result contains message type");
    assertTrue(repair.find("5000") != std::string::npos,
               "Repair result contains cost");
}

void testProtocolDamageEventMessage() {
    std::cout << "\n=== Protocol Damage Event Message ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string dmg = proto.createDamageEvent("target_01", 150.0f, "kinetic",
                                               "shield", true, false, false);
    assertTrue(dmg.find("damage_event") != std::string::npos,
               "Damage event contains message type");
    assertTrue(dmg.find("target_01") != std::string::npos,
               "Damage event contains target ID");
    assertTrue(dmg.find("shield") != std::string::npos,
               "Damage event contains layer_hit");
    assertTrue(dmg.find("\"shield_depleted\":true") != std::string::npos,
               "Damage event has shield_depleted flag");
}

void testProtocolDockRequestParse() {
    std::cout << "\n=== Protocol Dock Request Parse ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"dock_request\",\"data\":{\"station_id\":\"s1\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Dock request parses successfully");
    assertTrue(type == atlas::network::MessageType::DOCK_REQUEST, "Parsed type is DOCK_REQUEST");
}

void testProtocolWarpMessages() {
    std::cout << "\n=== Protocol Warp Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse warp_request message
    std::string msg = "{\"message_type\":\"warp_request\",\"data\":{\"dest_x\":1000,\"dest_y\":0,\"dest_z\":5000}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Warp request parses successfully");
    assertTrue(type == atlas::network::MessageType::WARP_REQUEST, "Parsed type is WARP_REQUEST");

    // Warp result creation
    std::string result = proto.createWarpResult(true);
    assertTrue(result.find("warp_result") != std::string::npos, "Warp result contains message type");
    assertTrue(result.find("true") != std::string::npos, "Warp result contains success");

    // Warp result with failure reason
    std::string fail = proto.createWarpResult(false, "Warp drive disrupted");
    assertTrue(fail.find("false") != std::string::npos, "Warp fail contains false");
    assertTrue(fail.find("Warp drive disrupted") != std::string::npos, "Warp fail contains reason");
}

void testProtocolMovementMessages() {
    std::cout << "\n=== Protocol Movement Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse approach message
    std::string msg = "{\"message_type\":\"approach\",\"data\":{\"target_id\":\"npc_1\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Approach message parses successfully");
    assertTrue(type == atlas::network::MessageType::APPROACH, "Parsed type is APPROACH");

    // Parse orbit message
    msg = "{\"message_type\":\"orbit\",\"data\":{\"target_id\":\"npc_1\",\"distance\":5000}}";
    ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Orbit message parses successfully");
    assertTrue(type == atlas::network::MessageType::ORBIT, "Parsed type is ORBIT");

    // Parse stop message
    msg = "{\"message_type\":\"stop\",\"data\":{}}";
    ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Stop message parses successfully");
    assertTrue(type == atlas::network::MessageType::STOP, "Parsed type is STOP");

    // Movement acknowledgement creation
    std::string ack = proto.createMovementAck("approach", true);
    assertTrue(ack.find("approach") != std::string::npos, "Movement ack contains command");
    assertTrue(ack.find("true") != std::string::npos, "Movement ack contains success");
}

// ==================== Mining System Tests ====================

void testMiningCreateDeposit() {
    std::cout << "\n=== Mining: Create Deposit ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    std::string id = mineSys.createDeposit("Veldspar", 5000.0f, 100.0f, 0.0f, 0.0f);
    assertTrue(!id.empty(), "Deposit entity created");

    auto* entity = world.getEntity(id);
    assertTrue(entity != nullptr, "Deposit entity exists in world");

    auto* dep = entity->getComponent<components::MineralDeposit>();
    assertTrue(dep != nullptr, "Deposit has MineralDeposit component");
    assertTrue(dep->mineral_type == "Veldspar", "Mineral type is Veldspar");
    assertTrue(approxEqual(dep->quantity_remaining, 5000.0f), "Quantity remaining is 5000");
    assertTrue(!dep->isDepleted(), "Deposit is not depleted");

    auto* pos = entity->getComponent<components::Position>();
    assertTrue(pos != nullptr, "Deposit has Position component");
    assertTrue(approxEqual(pos->x, 100.0f), "Deposit x position correct");
}

void testMiningStartStop() {
    std::cout << "\n=== Mining: Start and Stop ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    std::string dep_id = mineSys.createDeposit("Scordite", 1000.0f, 0.0f, 0.0f, 0.0f);

    auto* miner = world.createEntity("miner_1");
    auto* pos = addComp<components::Position>(miner);
    pos->x = 5000.0f; // within default 10000m range
    auto* laser = addComp<components::MiningLaser>(miner);
    laser->yield_per_cycle = 50.0f;
    laser->cycle_time = 10.0f;
    addComp<components::Inventory>(miner);

    bool started = mineSys.startMining("miner_1", dep_id);
    assertTrue(started, "Mining started successfully");
    assertTrue(laser->active, "Laser is active");
    assertTrue(mineSys.getActiveMinerCount() == 1, "One active miner");

    bool stopped = mineSys.stopMining("miner_1");
    assertTrue(stopped, "Mining stopped successfully");
    assertTrue(!laser->active, "Laser is inactive after stop");
    assertTrue(mineSys.getActiveMinerCount() == 0, "No active miners");
}

void testMiningRangeCheck() {
    std::cout << "\n=== Mining: Range Check ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    std::string dep_id = mineSys.createDeposit("Veldspar", 1000.0f, 0.0f, 0.0f, 0.0f);

    auto* miner = world.createEntity("miner_far");
    auto* pos = addComp<components::Position>(miner);
    pos->x = 20000.0f; // too far (default range 10000m)
    addComp<components::MiningLaser>(miner);
    addComp<components::Inventory>(miner);

    bool started = mineSys.startMining("miner_far", dep_id);
    assertTrue(!started, "Mining fails when out of range");
}

void testMiningCycleCompletion() {
    std::cout << "\n=== Mining: Cycle Completion ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    std::string dep_id = mineSys.createDeposit("Veldspar", 1000.0f, 0.0f, 0.0f, 0.0f, 0.1f);

    auto* miner = world.createEntity("miner_1");
    auto* pos = addComp<components::Position>(miner);
    pos->x = 100.0f;
    auto* laser = addComp<components::MiningLaser>(miner);
    laser->yield_per_cycle = 50.0f;
    laser->cycle_time = 10.0f;
    auto* inv = addComp<components::Inventory>(miner);
    inv->max_capacity = 500.0f;

    mineSys.startMining("miner_1", dep_id);

    // Advance 10 seconds — one full cycle
    mineSys.update(10.0f);

    assertTrue(inv->items.size() == 1, "Ore item added to inventory");
    assertTrue(inv->items[0].item_id == "Veldspar", "Mined Veldspar");
    assertTrue(inv->items[0].quantity == 50, "Mined 50 units");

    auto* dep = world.getEntity(dep_id)->getComponent<components::MineralDeposit>();
    assertTrue(approxEqual(dep->quantity_remaining, 950.0f), "Deposit reduced by 50");
}

void testMiningDepletedDeposit() {
    std::cout << "\n=== Mining: Depleted Deposit ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    // Small deposit — only 20 units
    std::string dep_id = mineSys.createDeposit("Kernite", 20.0f, 0.0f, 0.0f, 0.0f, 0.1f);

    auto* miner = world.createEntity("miner_1");
    addComp<components::Position>(miner);
    auto* laser = addComp<components::MiningLaser>(miner);
    laser->yield_per_cycle = 50.0f;
    laser->cycle_time = 5.0f;
    auto* inv = addComp<components::Inventory>(miner);
    inv->max_capacity = 500.0f;

    mineSys.startMining("miner_1", dep_id);
    mineSys.update(5.0f);

    // Should only get 20 units (deposit was 20, yield was 50)
    assertTrue(inv->items.size() == 1, "Ore item added");
    assertTrue(inv->items[0].quantity == 20, "Only mined available 20 units");

    auto* dep = world.getEntity(dep_id)->getComponent<components::MineralDeposit>();
    assertTrue(dep->isDepleted(), "Deposit is depleted");
    assertTrue(!laser->active, "Laser stops when deposit depleted");
}

void testMiningCargoFull() {
    std::cout << "\n=== Mining: Cargo Full ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    std::string dep_id = mineSys.createDeposit("Veldspar", 10000.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    auto* miner = world.createEntity("miner_1");
    addComp<components::Position>(miner);
    auto* laser = addComp<components::MiningLaser>(miner);
    laser->yield_per_cycle = 100.0f;
    laser->cycle_time = 5.0f;
    auto* inv = addComp<components::Inventory>(miner);
    inv->max_capacity = 50.0f; // only 50 m3 free, vol_per_unit=1.0

    mineSys.startMining("miner_1", dep_id);
    mineSys.update(5.0f);

    // Should only mine 50 units (cargo limit)
    assertTrue(inv->items.size() == 1, "Ore item added");
    assertTrue(inv->items[0].quantity == 50, "Capped by cargo capacity");
}

void testMiningOreStacking() {
    std::cout << "\n=== Mining: Ore Stacking ===" << std::endl;

    ecs::World world;
    systems::MiningSystem mineSys(&world);

    std::string dep_id = mineSys.createDeposit("Veldspar", 10000.0f, 0.0f, 0.0f, 0.0f, 0.1f);

    auto* miner = world.createEntity("miner_1");
    addComp<components::Position>(miner);
    auto* laser = addComp<components::MiningLaser>(miner);
    laser->yield_per_cycle = 30.0f;
    laser->cycle_time = 5.0f;
    auto* inv = addComp<components::Inventory>(miner);
    inv->max_capacity = 5000.0f;

    mineSys.startMining("miner_1", dep_id);

    // Complete two full cycles
    mineSys.update(5.0f);
    mineSys.update(5.0f);

    assertTrue(inv->items.size() == 1, "Ore stacked into single item");
    assertTrue(inv->items[0].quantity == 60, "Two cycles stacked: 30+30=60");
}

// ==================== Mining Drone Tests ====================

void testMiningDroneLaunchAndMine() {
    std::cout << "\n=== Mining Drone: Launch and Mine ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);
    systems::MiningSystem mineSys(&world);

    // Create a mineral deposit
    std::string dep_id = mineSys.createDeposit("Veldspar", 5000.0f, 0.0f, 0.0f, 0.0f, 0.1f);

    // Create a ship with mining drones
    auto* ship = world.createEntity("ship_1");
    auto* bay = addComp<components::DroneBay>(ship);
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 5000.0f;
    addComp<components::Position>(ship);

    components::DroneBay::DroneInfo mDrone;
    mDrone.drone_id = "mining_drone_1";
    mDrone.name = "Mining Drone I";
    mDrone.type = "mining_drone";
    mDrone.mining_yield = 25.0f;
    mDrone.rate_of_fire = 60.0f; // 60s cycle
    mDrone.hitpoints = 50.0f;
    mDrone.current_hp = 50.0f;
    mDrone.bandwidth_use = 5;
    mDrone.volume = 5.0f;
    bay->stored_drones.push_back(mDrone);

    // Launch drone and set mining target
    bool launched = droneSys.launchDrone("ship_1", "mining_drone_1");
    assertTrue(launched, "Mining drone launched");
    assertTrue(droneSys.getDeployedCount("ship_1") == 1, "One drone deployed");

    bool targeted = droneSys.setMiningTarget("ship_1", dep_id);
    assertTrue(targeted, "Mining target set");

    // Complete one cycle (drone rate_of_fire = 60s, but cooldown starts at 0)
    droneSys.update(0.0f); // first tick: mines immediately (cooldown=0)

    assertTrue(inv->items.size() == 1, "Ore mined by drone");
    assertTrue(inv->items[0].item_id == "Veldspar", "Mined correct mineral");
    assertTrue(inv->items[0].quantity == 25, "Mined correct amount");

    auto* dep = world.getEntity(dep_id)->getComponent<components::MineralDeposit>();
    assertTrue(approxEqual(dep->quantity_remaining, 4975.0f), "Deposit reduced by mining drone");
}

void testSalvageDroneLaunchAndSalvage() {
    std::cout << "\n=== Salvage Drone: Launch and Salvage ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);
    systems::WreckSalvageSystem wreckSys(&world);

    // Create a wreck with loot
    std::string wreck_id = wreckSys.createWreck("dead_npc", 100.0f, 0.0f, 0.0f);
    auto* wreck_entity = world.getEntity(wreck_id);
    auto* wreck_inv = wreck_entity->getComponent<components::Inventory>();
    components::Inventory::Item salvage;
    salvage.item_id = "salvage_metal";
    salvage.name = "Salvaged Metal";
    salvage.type = "salvage";
    salvage.quantity = 5;
    salvage.volume = 0.5f;
    wreck_inv->items.push_back(salvage);

    // Create ship with salvage drone
    auto* ship = world.createEntity("ship_1");
    auto* bay = addComp<components::DroneBay>(ship);
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 500.0f;
    addComp<components::Position>(ship);

    components::DroneBay::DroneInfo sDrone;
    sDrone.drone_id = "salvage_drone_1";
    sDrone.name = "Salvage Drone I";
    sDrone.type = "salvage_drone";
    sDrone.salvage_chance = 1.0f; // 100% for testing
    sDrone.rate_of_fire = 10.0f;
    sDrone.hitpoints = 30.0f;
    sDrone.current_hp = 30.0f;
    sDrone.bandwidth_use = 5;
    sDrone.volume = 5.0f;
    bay->stored_drones.push_back(sDrone);

    bool launched = droneSys.launchDrone("ship_1", "salvage_drone_1");
    assertTrue(launched, "Salvage drone launched");

    bool targeted = droneSys.setSalvageTarget("ship_1", wreck_id);
    assertTrue(targeted, "Salvage target set");

    // First tick: salvage immediately (cooldown=0), chance=1.0 guaranteed
    droneSys.update(0.0f);

    // Wreck should now be salvaged
    auto* wreck = wreck_entity->getComponent<components::Wreck>();
    assertTrue(wreck->salvaged, "Wreck marked as salvaged");

    // Items transferred to ship
    assertTrue(inv->items.size() == 1, "Salvage transferred to ship");
    assertTrue(inv->items[0].item_id == "salvage_metal", "Correct salvage item");
    assertTrue(inv->items[0].quantity == 5, "Correct salvage quantity");
}

void testSalvageDroneAlreadySalvaged() {
    std::cout << "\n=== Salvage Drone: Already Salvaged ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_id = wreckSys.createWreck("dead_npc2", 0.0f, 0.0f, 0.0f);
    auto* wreck_entity = world.getEntity(wreck_id);
    auto* wreck = wreck_entity->getComponent<components::Wreck>();
    wreck->salvaged = true; // already salvaged

    auto* ship = world.createEntity("ship_2");
    auto* bay = addComp<components::DroneBay>(ship);
    addComp<components::Inventory>(ship);
    addComp<components::Position>(ship);

    // Cannot set salvage target on already-salvaged wreck
    bool targeted = droneSys.setSalvageTarget("ship_2", wreck_id);
    assertTrue(!targeted, "Cannot target already-salvaged wreck");
}

void testMiningDroneTargetDepletedDeposit() {
    std::cout << "\n=== Mining Drone: Depleted Deposit ===" << std::endl;

    ecs::World world;
    systems::DroneSystem droneSys(&world);
    systems::MiningSystem mineSys(&world);

    std::string dep_id = mineSys.createDeposit("Veldspar", 0.0f, 0.0f, 0.0f, 0.0f);

    auto* ship = world.createEntity("ship_3");
    auto* bay = addComp<components::DroneBay>(ship);
    addComp<components::Inventory>(ship);
    addComp<components::Position>(ship);

    bool targeted = droneSys.setMiningTarget("ship_3", dep_id);
    assertTrue(!targeted, "Cannot target depleted deposit");
}

// ==================== Combat Death → Wreck Integration Tests ====================

void testCombatDeathCallbackFires() {
    std::cout << "\n=== Combat: Death Callback Fires ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);
    systems::WreckSalvageSystem wreckSys(&world);

    std::string wreck_created;
    combatSys.setDeathCallback([&](const std::string& entity_id, float x, float y, float z) {
        wreck_created = wreckSys.createWreck(entity_id, x, y, z);
    });

    auto* npc = world.createEntity("npc_1");
    auto* hp = addComp<components::Health>(npc);
    hp->shield_hp = 0.0f;
    hp->armor_hp = 0.0f;
    hp->hull_hp = 10.0f;
    hp->hull_max = 100.0f;
    auto* pos = addComp<components::Position>(npc);
    pos->x = 500.0f;
    pos->y = 200.0f;
    pos->z = 100.0f;

    // Apply lethal damage
    combatSys.applyDamage("npc_1", 50.0f, "kinetic");

    assertTrue(!wreck_created.empty(), "Wreck created on death");
    assertTrue(wreckSys.getActiveWreckCount() == 1, "One active wreck");

    auto* wreck_entity = world.getEntity(wreck_created);
    assertTrue(wreck_entity != nullptr, "Wreck entity exists");
    auto* wreck_pos = wreck_entity->getComponent<components::Position>();
    assertTrue(wreck_pos != nullptr, "Wreck has position");
    assertTrue(approxEqual(wreck_pos->x, 500.0f), "Wreck at correct x");
    assertTrue(approxEqual(wreck_pos->y, 200.0f), "Wreck at correct y");
}

void testCombatDeathCallbackWithLoot() {
    std::cout << "\n=== Combat: Death Callback with Loot ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);
    systems::LootSystem lootSys(&world);
    lootSys.setRandomSeed(999);

    std::string wreck_id;
    combatSys.setDeathCallback([&](const std::string& entity_id, float, float, float) {
        wreck_id = lootSys.generateLoot(entity_id);
    });

    auto* npc = world.createEntity("npc_loot");
    auto* hp = addComp<components::Health>(npc);
    hp->shield_hp = 0.0f;
    hp->armor_hp = 0.0f;
    hp->hull_hp = 5.0f;
    hp->hull_max = 100.0f;
    addComp<components::Position>(npc);

    auto* lt = addComp<components::LootTable>(npc);
    components::LootTable::LootEntry entry;
    entry.item_id = "module_afterburner";
    entry.name = "Afterburner I";
    entry.type = "module";
    entry.drop_chance = 1.0f; // always drops
    entry.min_quantity = 1;
    entry.max_quantity = 1;
    entry.volume = 5.0f;
    lt->entries.push_back(entry);
    lt->isk_drop = 5000.0;

    combatSys.applyDamage("npc_loot", 100.0f, "em");

    assertTrue(!wreck_id.empty(), "Loot wreck created on death");
    auto* wreck = world.getEntity(wreck_id);
    assertTrue(wreck != nullptr, "Wreck entity exists");
    auto* wreck_inv = wreck->getComponent<components::Inventory>();
    assertTrue(wreck_inv != nullptr, "Wreck has inventory");
    assertTrue(wreck_inv->items.size() >= 1, "Wreck contains loot");
}

void testCombatNoCallbackOnSurvival() {
    std::cout << "\n=== Combat: No Callback On Survival ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    bool callback_fired = false;
    combatSys.setDeathCallback([&](const std::string&, float, float, float) {
        callback_fired = true;
    });

    auto* npc = world.createEntity("npc_alive");
    auto* hp = addComp<components::Health>(npc);
    hp->shield_hp = 100.0f;
    hp->armor_hp = 100.0f;
    hp->hull_hp = 100.0f;
    hp->hull_max = 100.0f;

    combatSys.applyDamage("npc_alive", 10.0f, "kinetic");
    assertTrue(!callback_fired, "Death callback does NOT fire when entity survives");
}

// ==================== Combat Loop Integration Tests ====================

void testCombatFireWeaponAppliesDamage() {
    std::cout << "\n=== Combat Loop: Fire Weapon Applies Damage ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    // Create shooter with weapon
    auto* shooter = world.createEntity("player_1");
    auto* wep = addComp<components::Weapon>(shooter);
    wep->damage = 50.0f;
    wep->damage_type = "kinetic";
    wep->optimal_range = 10000.0f;
    wep->falloff_range = 5000.0f;
    wep->rate_of_fire = 3.0f;
    wep->cooldown = 0.0f;
    wep->ammo_count = 100;
    wep->capacitor_cost = 10.0f;
    auto* spos = addComp<components::Position>(shooter);
    spos->x = 0.0f; spos->y = 0.0f; spos->z = 0.0f;

    // Create target with health
    auto* target = world.createEntity("npc_target");
    auto* hp = addComp<components::Health>(target);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 100.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 5000.0f; tpos->y = 0.0f; tpos->z = 0.0f;

    float shield_before = hp->shield_hp;
    bool fired = combatSys.fireWeapon("player_1", "npc_target");

    assertTrue(fired, "Weapon fired successfully");
    assertTrue(hp->shield_hp < shield_before, "Target shield took damage");
    assertTrue(wep->cooldown > 0.0f, "Weapon now on cooldown");
    assertTrue(wep->ammo_count == 99, "Ammo consumed");
}

void testCombatFireWeaponCooldownPrevents() {
    std::cout << "\n=== Combat Loop: Cooldown Prevents Firing ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* shooter = world.createEntity("player_cd");
    auto* wep = addComp<components::Weapon>(shooter);
    wep->damage = 50.0f;
    wep->damage_type = "em";
    wep->optimal_range = 10000.0f;
    wep->falloff_range = 5000.0f;
    wep->rate_of_fire = 3.0f;
    wep->cooldown = 2.0f;  // On cooldown
    wep->ammo_count = 100;
    addComp<components::Position>(shooter);

    auto* target = world.createEntity("npc_cd_target");
    auto* hp = addComp<components::Health>(target);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 100.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;
    addComp<components::Position>(target);

    bool fired = combatSys.fireWeapon("player_cd", "npc_cd_target");

    assertTrue(!fired, "Weapon blocked by cooldown");
    assertTrue(hp->shield_hp == 100.0f, "Target took no damage");
}

void testCombatFireWeaponOutOfRange() {
    std::cout << "\n=== Combat Loop: Out of Range Prevents Firing ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* shooter = world.createEntity("player_range");
    auto* wep = addComp<components::Weapon>(shooter);
    wep->damage = 50.0f;
    wep->damage_type = "thermal";
    wep->optimal_range = 5000.0f;
    wep->falloff_range = 2000.0f;
    wep->rate_of_fire = 3.0f;
    wep->cooldown = 0.0f;
    wep->ammo_count = 100;
    auto* spos = addComp<components::Position>(shooter);
    spos->x = 0.0f; spos->y = 0.0f; spos->z = 0.0f;

    auto* target = world.createEntity("npc_far");
    auto* hp = addComp<components::Health>(target);
    hp->shield_hp = 100.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 100.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 50000.0f; tpos->y = 0.0f; tpos->z = 0.0f;  // Way beyond range

    bool fired = combatSys.fireWeapon("player_range", "npc_far");

    assertTrue(!fired, "Weapon blocked by range");
    assertTrue(hp->shield_hp == 100.0f, "Target took no damage when out of range");
}

void testCombatFireWeaponDamageLayerCascade() {
    std::cout << "\n=== Combat Loop: Damage Cascades Shield → Armor → Hull ===" << std::endl;

    ecs::World world;
    systems::CombatSystem combatSys(&world);

    auto* shooter = world.createEntity("player_cascade");
    auto* wep = addComp<components::Weapon>(shooter);
    wep->damage = 200.0f;
    wep->damage_type = "em";
    wep->optimal_range = 20000.0f;
    wep->falloff_range = 5000.0f;
    wep->rate_of_fire = 5.0f;
    wep->cooldown = 0.0f;
    wep->ammo_count = 100;
    auto* spos = addComp<components::Position>(shooter);
    spos->x = 0.0f; spos->y = 0.0f; spos->z = 0.0f;

    auto* target = world.createEntity("npc_cascade");
    auto* hp = addComp<components::Health>(target);
    hp->shield_hp = 50.0f; hp->shield_max = 100.0f;
    hp->armor_hp = 50.0f; hp->armor_max = 100.0f;
    hp->hull_hp = 100.0f; hp->hull_max = 100.0f;
    auto* tpos = addComp<components::Position>(target);
    tpos->x = 1000.0f; tpos->y = 0.0f; tpos->z = 0.0f;

    combatSys.fireWeapon("player_cascade", "npc_cascade");

    assertTrue(hp->shield_hp == 0.0f, "Shield depleted");
    assertTrue(hp->armor_hp == 0.0f, "Armor depleted");
    assertTrue(hp->hull_hp < 100.0f, "Hull took overflow damage");
}

// ==================== System Resources Component Tests ====================

void testSystemResourcesTracking() {
    std::cout << "\n=== System Resources: Tracking ===" << std::endl;

    ecs::World world;
    auto* system = world.createEntity("system_jita");
    auto* res = addComp<components::SystemResources>(system);

    components::SystemResources::ResourceEntry veldspar;
    veldspar.mineral_type = "Veldspar";
    veldspar.total_quantity = 100000.0f;
    veldspar.remaining_quantity = 100000.0f;
    res->resources.push_back(veldspar);

    components::SystemResources::ResourceEntry scordite;
    scordite.mineral_type = "Scordite";
    scordite.total_quantity = 50000.0f;
    scordite.remaining_quantity = 30000.0f;
    res->resources.push_back(scordite);

    assertTrue(res->resources.size() == 2, "Two resource types tracked");
    assertTrue(approxEqual(res->totalRemaining(), 130000.0f), "Total remaining correct");

    // Simulate depletion
    res->resources[0].remaining_quantity -= 10000.0f;
    assertTrue(approxEqual(res->totalRemaining(), 120000.0f), "Total after depletion");
}

// ==================== New Protocol Message Tests ====================

void testProtocolSalvageMessages() {
    std::cout << "\n=== Protocol: Salvage Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse salvage request
    std::string msg = "{\"message_type\":\"salvage_request\",\"data\":{\"wreck_id\":\"wreck_1\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Salvage request parses successfully");
    assertTrue(type == atlas::network::MessageType::SALVAGE_REQUEST, "Type is SALVAGE_REQUEST");

    // Create salvage result
    std::string result = proto.createSalvageResult(true, "wreck_1", 3);
    assertTrue(result.find("salvage_result") != std::string::npos, "Result has correct type");
    assertTrue(result.find("wreck_1") != std::string::npos, "Result contains wreck_id");
    assertTrue(result.find("3") != std::string::npos, "Result contains items_recovered");
}

void testProtocolLootMessages() {
    std::cout << "\n=== Protocol: Loot Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"loot_all\",\"data\":{\"wreck_id\":\"wreck_2\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Loot all parses successfully");
    assertTrue(type == atlas::network::MessageType::LOOT_ALL, "Type is LOOT_ALL");

    std::string result = proto.createLootResult(true, "wreck_2", 5, 10000.0);
    assertTrue(result.find("loot_result") != std::string::npos, "Result has correct type");
    assertTrue(result.find("wreck_2") != std::string::npos, "Result contains wreck_id");
    assertTrue(result.find("10000") != std::string::npos, "Result contains isk_gained");
}

void testProtocolMiningMessages() {
    std::cout << "\n=== Protocol: Mining Messages ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    // Parse mining start
    std::string msg = "{\"message_type\":\"mining_start\",\"data\":{\"deposit_id\":\"deposit_0\"}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Mining start parses successfully");
    assertTrue(type == atlas::network::MessageType::MINING_START, "Type is MINING_START");

    // Parse mining stop
    msg = "{\"message_type\":\"mining_stop\",\"data\":{}}";
    ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Mining stop parses successfully");
    assertTrue(type == atlas::network::MessageType::MINING_STOP, "Type is MINING_STOP");

    // Create mining result
    std::string result = proto.createMiningResult(true, "deposit_0", "Veldspar", 100);
    assertTrue(result.find("mining_result") != std::string::npos, "Result has correct type");
    assertTrue(result.find("Veldspar") != std::string::npos, "Result contains mineral_type");
    assertTrue(result.find("100") != std::string::npos, "Result contains quantity_mined");
}

void testProtocolLootResultParse() {
    std::cout << "\n=== Protocol: Loot Result Parse ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"loot_result\",\"data\":{\"success\":true,\"wreck_id\":\"wreck_3\",\"items_collected\":2,\"isk_gained\":5000}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Loot result parses successfully");
    assertTrue(type == atlas::network::MessageType::LOOT_RESULT, "Type is LOOT_RESULT");
}

void testProtocolMiningResultParse() {
    std::cout << "\n=== Protocol: Mining Result Parse ===" << std::endl;

    atlas::network::ProtocolHandler proto;

    std::string msg = "{\"message_type\":\"mining_result\",\"data\":{\"success\":true,\"deposit_id\":\"deposit_1\",\"mineral_type\":\"Scordite\",\"quantity_mined\":50}}";
    atlas::network::MessageType type;
    std::string data;
    bool ok = proto.parseMessage(msg, type, data);
    assertTrue(ok, "Mining result parses successfully");
    assertTrue(type == atlas::network::MessageType::MINING_RESULT, "Type is MINING_RESULT");
}

// ==================== AI Mining State Test ====================

void testAIMiningState() {
    std::cout << "\n=== AI: Mining State ===" << std::endl;

    ecs::World world;
    auto* npc = world.createEntity("miner_npc");
    auto* ai = addComp<components::AI>(npc);
    ai->state = components::AI::State::Mining;

    assertTrue(ai->state == components::AI::State::Mining, "AI state set to Mining");
    // Mining state is distinct from other states
    assertTrue(ai->state != components::AI::State::Idle, "Mining != Idle");
    assertTrue(ai->state != components::AI::State::Attacking, "Mining != Attacking");
}

// ==================== Ship Generation Model Data Tests ====================

void testShipModelDataParsed() {
    std::cout << "\n=== ShipDatabase: Model Data Parsed ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    const data::ShipTemplate* fang = db.getShip("fang");
    if (fang) {
        assertTrue(fang->model_data.has_model_data, "Fang has model_data");
        assertTrue(fang->model_data.turret_hardpoints >= 2, "Fang turret hardpoints >= 2");
        assertTrue(fang->model_data.turret_hardpoints <= 3, "Fang turret hardpoints <= 3");
        assertTrue(fang->model_data.engine_count >= 2, "Fang engine count >= 2");
        assertTrue(fang->model_data.engine_count <= 3, "Fang engine count <= 3");
        assertTrue(fang->model_data.generation_seed > 0, "Fang generation seed > 0");
    } else {
        assertTrue(false, "Fang template found for model_data test");
    }
}

void testShipModelDataCapitalShips() {
    std::cout << "\n=== ShipDatabase: Capital Ship Model Data ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    const data::ShipTemplate* empyrean = db.getShip("empyrean");
    if (empyrean) {
        assertTrue(empyrean->model_data.has_model_data, "Empyrean has model_data");
        assertTrue(empyrean->model_data.turret_hardpoints >= 6, "Titan turrets >= 6");
        assertTrue(empyrean->model_data.turret_hardpoints <= 10, "Titan turrets <= 10");
        assertTrue(empyrean->model_data.engine_count >= 8, "Titan engines >= 8");
        assertTrue(empyrean->model_data.engine_count <= 12, "Titan engines <= 12");
    } else {
        assertTrue(false, "Empyrean template found for capital model_data test");
    }

    const data::ShipTemplate* solarius = db.getShip("solarius");
    if (solarius) {
        assertTrue(solarius->model_data.has_model_data, "Solarius has model_data");
        assertTrue(solarius->model_data.drone_bays >= 5, "Carrier drone_bays >= 5");
        assertTrue(solarius->model_data.drone_bays <= 10, "Carrier drone_bays <= 10");
    } else {
        assertTrue(false, "Solarius template found for carrier model_data test");
    }
}

void testShipModelDataAllShipsHaveModelData() {
    std::cout << "\n=== ShipDatabase: All Ships Have Model Data ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    auto ids = db.getShipIds();
    assertTrue(ids.size() >= 90, "At least 90 ships loaded");

    int withModelData = 0;
    for (const auto& id : ids) {
        const data::ShipTemplate* ship = db.getShip(id);
        if (ship && ship->model_data.has_model_data) {
            withModelData++;
        }
    }

    assertTrue(withModelData == static_cast<int>(ids.size()),
               "All ships have model_data (" + std::to_string(withModelData) + "/" + std::to_string(ids.size()) + ")");
}

void testShipModelDataSeedUniqueness() {
    std::cout << "\n=== ShipDatabase: Model Data Seed Uniqueness ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    auto ids = db.getShipIds();
    std::map<int, std::string> seedMap;
    int uniqueSeeds = 0;
    for (const auto& id : ids) {
        const data::ShipTemplate* ship = db.getShip(id);
        if (ship && ship->model_data.has_model_data && ship->model_data.generation_seed > 0) {
            if (seedMap.find(ship->model_data.generation_seed) == seedMap.end()) {
                seedMap[ship->model_data.generation_seed] = id;
                uniqueSeeds++;
            }
        }
    }

    // Seeds should be mostly unique (allow small number of collisions due to hash)
    assertTrue(uniqueSeeds >= static_cast<int>(ids.size()) * 9 / 10,
               "Most seeds are unique (" + std::to_string(uniqueSeeds) + "/" + std::to_string(ids.size()) + ")");
}

void testShipModelDataEngineCountPositive() {
    std::cout << "\n=== ShipDatabase: All Ships Have Engines ===" << std::endl;

    data::ShipDatabase db;
    if (db.loadFromDirectory("../data") == 0) {
        if (db.loadFromDirectory("data") == 0) {
            db.loadFromDirectory("../../data");
        }
    }

    auto ids = db.getShipIds();
    int allHaveEngines = 0;
    for (const auto& id : ids) {
        const data::ShipTemplate* ship = db.getShip(id);
        if (ship && ship->model_data.has_model_data && ship->model_data.engine_count >= 2) {
            allHaveEngines++;
        }
    }

    assertTrue(allHaveEngines == static_cast<int>(ids.size()),
               "All ships have >= 2 engines (" + std::to_string(allHaveEngines) + "/" + std::to_string(ids.size()) + ")");
}

void testShipModelDataMissingReturnsDefaults() {
    std::cout << "\n=== ShipDatabase: Missing Model Data Returns Defaults ===" << std::endl;

    // A ShipTemplate without model_data block should have defaults
    data::ShipTemplate empty;
    assertTrue(!empty.model_data.has_model_data, "Default template has no model_data");
    assertTrue(empty.model_data.turret_hardpoints == 0, "Default turret hardpoints is 0");
    assertTrue(empty.model_data.launcher_hardpoints == 0, "Default launcher hardpoints is 0");
    assertTrue(empty.model_data.drone_bays == 0, "Default drone bays is 0");
    assertTrue(empty.model_data.engine_count == 2, "Default engine count is 2");
    assertTrue(empty.model_data.generation_seed == 0, "Default generation seed is 0");
}

// ==================== Fleet Formation System Tests ====================

void testFleetFormationSetFormation() {
    std::cout << "\n=== Fleet Formation: Set Formation ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("leader");
    world.createEntity("wing1");

    using FT = components::FleetFormation::FormationType;
    sys.setFormation("leader", FT::Arrow, 0);
    sys.setFormation("wing1", FT::Arrow, 1);

    assertTrue(sys.getFormation("leader") == FT::Arrow, "Leader has Arrow formation");
    assertTrue(sys.getFormation("wing1") == FT::Arrow, "Wing1 has Arrow formation");
}

void testFleetFormationLeaderAtOrigin() {
    std::cout << "\n=== Fleet Formation: Leader At Origin ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("leader");

    using FT = components::FleetFormation::FormationType;
    sys.setFormation("leader", FT::Arrow, 0);
    sys.computeOffsets();

    float ox = 0, oy = 0, oz = 0;
    assertTrue(sys.getOffset("leader", ox, oy, oz), "Leader has offset");
    assertTrue(approxEqual(ox, 0.0f) && approxEqual(oy, 0.0f) && approxEqual(oz, 0.0f),
               "Leader offset is (0,0,0)");
}

void testFleetFormationArrowOffsets() {
    std::cout << "\n=== Fleet Formation: Arrow Offsets ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("s0");
    world.createEntity("s1");
    world.createEntity("s2");

    using FT = components::FleetFormation::FormationType;
    sys.setFormation("s0", FT::Arrow, 0);
    sys.setFormation("s1", FT::Arrow, 1);
    sys.setFormation("s2", FT::Arrow, 2);
    sys.computeOffsets();

    float ox1 = 0, oy1 = 0, oz1 = 0;
    sys.getOffset("s1", ox1, oy1, oz1);
    assertTrue(ox1 < 0.0f, "Slot 1 is to the left");
    assertTrue(oz1 < 0.0f, "Slot 1 is behind");

    float ox2 = 0, oy2 = 0, oz2 = 0;
    sys.getOffset("s2", ox2, oy2, oz2);
    assertTrue(ox2 > 0.0f, "Slot 2 is to the right");
    assertTrue(oz2 < 0.0f, "Slot 2 is behind");
}

void testFleetFormationLineOffsets() {
    std::cout << "\n=== Fleet Formation: Line Offsets ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("s0");
    world.createEntity("s1");
    world.createEntity("s2");

    using FT = components::FleetFormation::FormationType;
    sys.setFormation("s0", FT::Line, 0);
    sys.setFormation("s1", FT::Line, 1);
    sys.setFormation("s2", FT::Line, 2);
    sys.computeOffsets();

    float ox0 = 0, oy0 = 0, oz0 = 0;
    sys.getOffset("s0", ox0, oy0, oz0);
    assertTrue(approxEqual(ox0, 0.0f) && approxEqual(oz0, 0.0f), "Line slot 0 at origin");

    float ox1 = 0, oy1 = 0, oz1 = 0;
    sys.getOffset("s1", ox1, oy1, oz1);
    assertTrue(approxEqual(ox1, 0.0f), "Line slot 1 aligned in X");
    assertTrue(oz1 < 0.0f, "Line slot 1 behind leader");

    float ox2 = 0, oy2 = 0, oz2 = 0;
    sys.getOffset("s2", ox2, oy2, oz2);
    assertTrue(oz2 < oz1, "Line slot 2 further behind than slot 1");
}

void testFleetFormationDiamondOffsets() {
    std::cout << "\n=== Fleet Formation: Diamond Offsets ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    for (int i = 0; i < 4; ++i)
        world.createEntity("d" + std::to_string(i));

    using FT = components::FleetFormation::FormationType;
    for (int i = 0; i < 4; ++i)
        sys.setFormation("d" + std::to_string(i), FT::Diamond, i);
    sys.computeOffsets();

    float ox0 = 0, oy0 = 0, oz0 = 0;
    sys.getOffset("d0", ox0, oy0, oz0);
    assertTrue(approxEqual(ox0, 0.0f) && approxEqual(oz0, 0.0f), "Diamond slot 0 at origin");

    float ox1 = 0, oy1 = 0, oz1 = 0;
    sys.getOffset("d1", ox1, oy1, oz1);
    assertTrue(ox1 < 0.0f, "Diamond slot 1 to the left");

    float ox2 = 0, oy2 = 0, oz2 = 0;
    sys.getOffset("d2", ox2, oy2, oz2);
    assertTrue(ox2 > 0.0f, "Diamond slot 2 to the right");

    float ox3 = 0, oy3 = 0, oz3 = 0;
    sys.getOffset("d3", ox3, oy3, oz3);
    assertTrue(approxEqual(ox3, 0.0f), "Diamond slot 3 centered in X");
    assertTrue(oz3 < oz1, "Diamond slot 3 behind slots 1 & 2");
}

void testFleetFormationNoneType() {
    std::cout << "\n=== Fleet Formation: None Type ===" << std::endl;
    ecs::World world;
    systems::FleetFormationSystem sys(&world);
    world.createEntity("e1");

    assertTrue(sys.getFormation("e1") == components::FleetFormation::FormationType::None,
               "Entity without formation returns None");

    float ox = 0, oy = 0, oz = 0;
    assertTrue(!sys.getOffset("e1", ox, oy, oz), "No offset for entity without component");
}

// ==================== Extended Captain Personality Tests ====================

void testCaptainPersonalityNewTraitsAssigned() {
    std::cout << "\n=== Captain Personality: New Traits Assigned ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "TestCaptain", "Keldari");

    float loyalty = sys.getPersonalityTrait("cap1", "loyalty");
    float paranoia = sys.getPersonalityTrait("cap1", "paranoia");
    float ambition = sys.getPersonalityTrait("cap1", "ambition");
    float adaptability = sys.getPersonalityTrait("cap1", "adaptability");

    assertTrue(loyalty >= 0.0f && loyalty <= 1.0f, "Loyalty in valid range");
    assertTrue(paranoia >= 0.0f && paranoia <= 1.0f, "Paranoia in valid range");
    assertTrue(ambition >= 0.0f && ambition <= 1.0f, "Ambition in valid range");
    assertTrue(adaptability >= 0.0f && adaptability <= 1.0f, "Adaptability in valid range");
}

void testCaptainPersonalityKeldariHighParanoia() {
    std::cout << "\n=== Captain Personality: Keldari High Paranoia ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "KeldariCaptain", "Keldari");
    float paranoia = sys.getPersonalityTrait("cap1", "paranoia");
    assertTrue(paranoia > 0.5f, "Keldari captain has high paranoia");
}

void testCaptainPersonalitySolariHighLoyalty() {
    std::cout << "\n=== Captain Personality: Solari High Loyalty ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "SolariCaptain", "Solari");
    float loyalty = sys.getPersonalityTrait("cap1", "loyalty");
    assertTrue(loyalty > 0.5f, "Solari captain has high loyalty");
}

void testCaptainPersonalitySetNewTrait() {
    std::cout << "\n=== Captain Personality: Set New Trait ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "Test", "Veyren");
    sys.setPersonalityTrait("cap1", "loyalty", 0.95f);
    assertTrue(approxEqual(sys.getPersonalityTrait("cap1", "loyalty"), 0.95f),
               "Loyalty set correctly");
    sys.setPersonalityTrait("cap1", "paranoia", 0.1f);
    assertTrue(approxEqual(sys.getPersonalityTrait("cap1", "paranoia"), 0.1f),
               "Paranoia set correctly");
}

void testCaptainPersonalityNewTraitsDeterministic() {
    std::cout << "\n=== Captain Personality: New Traits Deterministic ===" << std::endl;
    ecs::World world;
    systems::CaptainPersonalitySystem sys(&world);
    world.createEntity("cap1");
    sys.assignPersonality("cap1", "Test", "Aurelian");
    float loy1 = sys.getPersonalityTrait("cap1", "loyalty");
    float par1 = sys.getPersonalityTrait("cap1", "paranoia");
    sys.assignPersonality("cap1", "Test", "Aurelian");
    float loy2 = sys.getPersonalityTrait("cap1", "loyalty");
    float par2 = sys.getPersonalityTrait("cap1", "paranoia");
    assertTrue(approxEqual(loy1, loy2), "Loyalty is deterministic");
    assertTrue(approxEqual(par1, par2), "Paranoia is deterministic");
}

// ==================== Captain Memory System Tests ====================

void testCaptainMemoryRecordEvent() {
    std::cout << "\n=== Captain Memory: Record Event ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    sys.recordMemory("cap1", "combat_win", "vs Pirates", 100.0f, 0.5f);
    assertTrue(sys.totalMemories("cap1") == 1, "One memory recorded");
    assertTrue(sys.countMemories("cap1", "combat_win") == 1, "One combat_win memory");
}

void testCaptainMemoryMultipleEvents() {
    std::cout << "\n=== Captain Memory: Multiple Events ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    sys.recordMemory("cap1", "combat_win", "vs Pirates", 100.0f, 0.5f);
    sys.recordMemory("cap1", "combat_loss", "vs Boss", 200.0f, -0.8f);
    sys.recordMemory("cap1", "saved_by_player", "close call", 300.0f, 0.9f);
    assertTrue(sys.totalMemories("cap1") == 3, "Three memories recorded");
    assertTrue(sys.countMemories("cap1", "combat_win") == 1, "One combat_win");
    assertTrue(sys.countMemories("cap1", "combat_loss") == 1, "One combat_loss");
}

void testCaptainMemoryAverageWeight() {
    std::cout << "\n=== Captain Memory: Average Weight ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    sys.recordMemory("cap1", "combat_win", "", 100.0f, 1.0f);
    sys.recordMemory("cap1", "combat_loss", "", 200.0f, -1.0f);
    float avg = sys.averageEmotionalWeight("cap1");
    assertTrue(approxEqual(avg, 0.0f), "Average weight is zero for balanced events");
}

void testCaptainMemoryMostRecent() {
    std::cout << "\n=== Captain Memory: Most Recent ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    sys.recordMemory("cap1", "combat_win", "", 100.0f, 0.5f);
    sys.recordMemory("cap1", "warp_anomaly", "strange lights", 200.0f, 0.2f);
    assertTrue(sys.mostRecentEvent("cap1") == "warp_anomaly", "Most recent is warp_anomaly");
}

void testCaptainMemoryCapacity() {
    std::cout << "\n=== Captain Memory: Capacity Cap ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);
    world.createEntity("cap1");

    // Fill 55 memories — should cap at 50
    for (int i = 0; i < 55; ++i) {
        sys.recordMemory("cap1", "event" + std::to_string(i), "", static_cast<float>(i), 0.1f);
    }
    assertTrue(sys.totalMemories("cap1") == 50, "Memory capped at 50");
}

void testCaptainMemoryNoEntity() {
    std::cout << "\n=== Captain Memory: No Entity ===" << std::endl;
    ecs::World world;
    systems::CaptainMemorySystem sys(&world);

    assertTrue(sys.totalMemories("nonexistent") == 0, "No memories for nonexistent entity");
    assertTrue(sys.mostRecentEvent("nonexistent").empty(), "No recent event for nonexistent entity");
}

// ==================== Contextual Chatter Tests ====================

void testContextualChatterReturnsLine() {
    std::cout << "\n=== Contextual Chatter: Returns Line ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem chatterSys(&world);
    systems::CaptainPersonalitySystem personalitySys(&world);

    auto* entity = world.createEntity("cap1");
    personalitySys.assignPersonality("cap1", "TestCaptain", "Keldari");
    addComp<components::FleetChatterState>(entity);
    chatterSys.setActivity("cap1", "Combat");

    std::string line = chatterSys.getContextualLine("cap1");
    assertTrue(!line.empty(), "Contextual chatter returns a line");
}

void testContextualChatterRespectsCooldown() {
    std::cout << "\n=== Contextual Chatter: Respects Cooldown ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem chatterSys(&world);
    systems::CaptainPersonalitySystem personalitySys(&world);

    auto* entity = world.createEntity("cap1");
    personalitySys.assignPersonality("cap1", "TestCaptain", "Solari");
    addComp<components::FleetChatterState>(entity);
    chatterSys.setActivity("cap1", "Mining");

    chatterSys.getContextualLine("cap1");
    std::string second = chatterSys.getContextualLine("cap1");
    assertTrue(second.empty(), "Contextual chatter on cooldown returns empty");
}

void testContextualChatterFallbackWithoutPersonality() {
    std::cout << "\n=== Contextual Chatter: Fallback Without Personality ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem chatterSys(&world);

    auto* entity = world.createEntity("cap1");
    addComp<components::FleetChatterState>(entity);
    chatterSys.setActivity("cap1", "Idle");

    std::string line = chatterSys.getContextualLine("cap1");
    assertTrue(!line.empty(), "Falls back to generic pool without personality");
}

// ==================== Ship Fitting System Tests ====================

void testShipFittingFitModule() {
    std::cout << "\n=== Ship Fitting: Fit Module ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Frigate";
    ship->cpu_max = 100.0f;
    ship->powergrid_max = 50.0f;
    addComp<components::ModuleRack>(ship_entity);

    bool ok = fittingSys.fitModule("ship1", "mod1", "Light Autocannon", "high", 10.0f, 5.0f);
    assertTrue(ok, "Fitting a high-slot module succeeds");
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 1, "One high-slot module fitted");
}

void testShipFittingSlotLimit() {
    std::cout << "\n=== Ship Fitting: Slot Limit ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Frigate";
    ship->cpu_max = 500.0f;
    ship->powergrid_max = 500.0f;
    addComp<components::ModuleRack>(ship_entity);

    // Frigate has 3 high slots
    fittingSys.fitModule("ship1", "h1", "Gun 1", "high", 5.0f, 5.0f);
    fittingSys.fitModule("ship1", "h2", "Gun 2", "high", 5.0f, 5.0f);
    fittingSys.fitModule("ship1", "h3", "Gun 3", "high", 5.0f, 5.0f);
    bool ok = fittingSys.fitModule("ship1", "h4", "Gun 4", "high", 5.0f, 5.0f);
    assertTrue(!ok, "Cannot fit more than 3 high-slot modules on Frigate");
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 3, "Still 3 high-slot modules");
}

void testShipFittingCPUOverflow() {
    std::cout << "\n=== Ship Fitting: CPU Overflow ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Cruiser";
    ship->cpu_max = 30.0f;
    ship->powergrid_max = 500.0f;
    addComp<components::ModuleRack>(ship_entity);

    fittingSys.fitModule("ship1", "h1", "Gun 1", "high", 15.0f, 5.0f);
    fittingSys.fitModule("ship1", "h2", "Gun 2", "high", 15.0f, 5.0f);
    bool ok = fittingSys.fitModule("ship1", "h3", "Gun 3", "high", 15.0f, 5.0f);
    assertTrue(!ok, "Cannot exceed CPU budget");
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 2, "Still 2 modules");
}

void testShipFittingRemoveModule() {
    std::cout << "\n=== Ship Fitting: Remove Module ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Frigate";
    ship->cpu_max = 100.0f;
    ship->powergrid_max = 50.0f;
    addComp<components::ModuleRack>(ship_entity);

    fittingSys.fitModule("ship1", "h1", "Gun 1", "high", 10.0f, 5.0f);
    fittingSys.fitModule("ship1", "h2", "Gun 2", "high", 10.0f, 5.0f);
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 2, "2 modules before remove");

    bool ok = fittingSys.removeModule("ship1", "high", 0);
    assertTrue(ok, "Remove succeeds");
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 1, "1 module after remove");
}

void testShipFittingValidate() {
    std::cout << "\n=== Ship Fitting: Validate ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Frigate";
    ship->cpu_max = 100.0f;
    ship->powergrid_max = 50.0f;
    addComp<components::ModuleRack>(ship_entity);

    fittingSys.fitModule("ship1", "h1", "Gun 1", "high", 10.0f, 5.0f);
    assertTrue(fittingSys.validateFitting("ship1"), "Valid fitting within budget");
}

void testShipFittingSlotCapacity() {
    std::cout << "\n=== Ship Fitting: Slot Capacity Lookup ===" << std::endl;
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Frigate", "high") == 3,
               "Frigate has 3 high slots");
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Frigate", "mid") == 3,
               "Frigate has 3 mid slots");
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Frigate", "low") == 2,
               "Frigate has 2 low slots");
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Battleship", "high") == 7,
               "Battleship has 7 high slots");
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Cruiser", "mid") == 4,
               "Cruiser has 4 mid slots");
}

void testShipFittingMidLowSlots() {
    std::cout << "\n=== Ship Fitting: Mid and Low Slots ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Cruiser";
    ship->cpu_max = 500.0f;
    ship->powergrid_max = 500.0f;
    addComp<components::ModuleRack>(ship_entity);

    fittingSys.fitModule("ship1", "m1", "Shield Booster", "mid", 10.0f, 10.0f);
    fittingSys.fitModule("ship1", "l1", "Armor Plate", "low", 10.0f, 10.0f);
    assertTrue(fittingSys.getFittedCount("ship1", "mid") == 1, "1 mid-slot module fitted");
    assertTrue(fittingSys.getFittedCount("ship1", "low") == 1, "1 low-slot module fitted");
}

// ==================== Player Fleet Tests ====================

void testPlayerFleetCreate() {
    std::cout << "\n=== Player Fleet: Create ===" << std::endl;
    ecs::World world;
    systems::FleetSystem fleetSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::Player>(player);
    addComp<components::Position>(player);

    std::string fid = fleetSys.createPlayerFleet("player1", "My Fleet");
    assertTrue(!fid.empty(), "Player fleet created successfully");
    assertTrue(fleetSys.isPlayerFleet(fid), "Fleet is marked as player fleet");
    assertTrue(fleetSys.getMemberCount(fid) == 1, "Fleet has 1 member (player)");
}

void testPlayerFleetAssignCaptains() {
    std::cout << "\n=== Player Fleet: Assign Captains ===" << std::endl;
    ecs::World world;
    systems::FleetSystem fleetSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::Player>(player);
    addComp<components::Position>(player);

    std::string fid = fleetSys.createPlayerFleet("player1");

    // Add 4 captains
    for (int i = 1; i <= 4; ++i) {
        std::string cid = "captain" + std::to_string(i);
        auto* cap = world.createEntity(cid);
        addComp<components::Position>(cap);
        bool ok = fleetSys.assignCaptain(fid, cid, "Captain " + std::to_string(i));
        assertTrue(ok, "Captain " + std::to_string(i) + " assigned");
    }
    assertTrue(fleetSys.getMemberCount(fid) == 5, "Fleet has 5 members total");
}

void testPlayerFleetMaxCap() {
    std::cout << "\n=== Player Fleet: Max 5 Ships ===" << std::endl;
    ecs::World world;
    systems::FleetSystem fleetSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::Player>(player);
    addComp<components::Position>(player);

    std::string fid = fleetSys.createPlayerFleet("player1");

    for (int i = 1; i <= 4; ++i) {
        std::string cid = "captain" + std::to_string(i);
        auto* cap = world.createEntity(cid);
        addComp<components::Position>(cap);
        fleetSys.assignCaptain(fid, cid, "Captain " + std::to_string(i));
    }

    // 5th captain should fail
    auto* extra = world.createEntity("captain5");
    addComp<components::Position>(extra);
    bool ok = fleetSys.assignCaptain(fid, "captain5", "Extra Captain");
    assertTrue(!ok, "Cannot add 5th captain (fleet full at 5)");
    assertTrue(fleetSys.getMemberCount(fid) == 5, "Fleet still 5 members");
}

void testPlayerFleetNotPlayerFleet() {
    std::cout << "\n=== Player Fleet: assignCaptain on non-player fleet ===" << std::endl;
    ecs::World world;
    systems::FleetSystem fleetSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::Player>(player);
    addComp<components::Position>(player);

    // Create a normal fleet (not player fleet)
    std::string fid = fleetSys.createFleet("player1", "Normal Fleet");
    assertTrue(!fleetSys.isPlayerFleet(fid), "Normal fleet is not player fleet");

    auto* cap = world.createEntity("cap1");
    addComp<components::Position>(cap);
    bool ok = fleetSys.assignCaptain(fid, "cap1", "Captain");
    assertTrue(!ok, "assignCaptain fails on non-player fleet");
}

// ==================== Main ====================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "EVE OFFLINE C++ Server System Tests" << std::endl;
    std::cout << "Capacitor, Shield, Weapon, Targeting," << std::endl;
    std::cout << "ShipDB, WormholeDB, Wormhole, Fleet," << std::endl;
    std::cout << "Mission, Skill, Module, Inventory," << std::endl;
    std::cout << "Loot, NpcDB, Drone, Insurance, Bounty, Market," << std::endl;
    std::cout << "WorldPersistence, Interdictors, StealthBombers," << std::endl;
    std::cout << "PI, Manufacturing, Research," << std::endl;
    std::cout << "Chat, CharacterCreation, Tournament, Leaderboard," << std::endl;
    std::cout << "Station, WreckSalvage, ServerConsole," << std::endl;
    std::cout << "Logger, ServerMetrics," << std::endl;
    std::cout << "FleetMorale, CaptainPersonality, FleetChatter," << std::endl;
    std::cout << "WarpAnomaly, CaptainRelationship, EmotionalArc," << std::endl;
    std::cout << "FleetCargo, TacticalOverlay," << std::endl;
    std::cout << "DamageEvent, AIRetreat, WarpState," << std::endl;
    std::cout << "AIDynamicOrbit, AITargetSelection, Protocol," << std::endl;
    std::cout << "Mining, MiningDrones, SalvageDrones," << std::endl;
    std::cout << "CombatDeathWreck, SystemResources," << std::endl;
    std::cout << "ShipModelData," << std::endl;
    std::cout << "FleetFormation, CaptainMemory," << std::endl;
    std::cout << "ExtendedPersonality, ContextualChatter," << std::endl;
    std::cout << "ShipFitting, PlayerFleet" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // Capacitor tests
    testCapacitorRecharge();
    testCapacitorConsume();
    testCapacitorPercentage();
    
    // Shield recharge tests
    testShieldRecharge();
    testShieldPercentage();
    
    // Weapon system tests
    testWeaponCooldown();
    testWeaponFireWithCapacitor();
    testWeaponFireInsufficientCapacitor();
    testWeaponFireOutOfRange();
    testWeaponDamageFalloff();
    testWeaponDamageResistances();
    testWeaponAutoFireAI();
    testWeaponNoAutoFireIdleAI();
    
    // Targeting system tests
    testTargetLockUnlock();
    testTargetLockMaxTargets();
    testTargetLockNonexistent();
    
    // ShipDatabase tests
    testShipDatabaseLoadFromDirectory();
    testShipDatabaseGetShip();
    testShipDatabaseResistances();
    testShipDatabaseGetShipIds();
    testShipDatabaseCapitalShips();
    testShipDatabaseMarauders();
    testShipDatabaseInterdictors();
    testShipDatabaseStealthBombers();
    testShipDatabaseSecondHACs();
    
    // WormholeDatabase tests
    testWormholeDatabaseLoad();
    testWormholeDatabaseGetClass();
    testWormholeDatabaseEffects();
    testWormholeDatabaseClassIds();
    
    // WormholeSystem tests
    testWormholeLifetimeDecay();
    testWormholeJumpMass();
    testWormholeMassCollapse();
    testWormholeNonexistent();
    testSolarSystemComponent();
    
    // Fleet system tests
    testFleetCreateAndDisband();
    testFleetAddRemoveMembers();
    testFleetFCLeavePromotes();
    testFleetDisbandOnEmpty();
    testFleetPromoteMember();
    testFleetSquadAndWingOrganization();
    testFleetBonuses();
    testFleetBroadcastTarget();
    testFleetWarp();
    testFleetDisbandPermission();
    testFleetMembershipComponent();
    
    // World persistence tests
    testSerializeDeserializeBasicEntity();
    testSerializeDeserializeHealthCapacitor();
    testSerializeDeserializeShipAndFaction();
    testSerializeDeserializeStandings();
    testStandingsGetStanding();
    testStandingsModify();
    testSerializeDeserializeAIAndWeapon();
    testSerializeDeserializePlayerComponent();
    testSerializeDeserializeMultipleEntities();
    testSaveLoadFile();
    testLoadNonexistentFile();
    testSerializeDeserializeWormholeAndSolarSystem();
    testEmptyWorldSerialize();
    
    // Movement system & collision tests
    testMovementBasicUpdate();
    testMovementSpeedLimit();
    testMovementCollisionZonePush();
    testMovementCollisionZoneVelocityKilled();
    testMovementOutsideCollisionZoneUnaffected();
    testMovementMultipleCollisionZones();
    
    // Logger tests
    testLoggerLevels();
    testLoggerFileOutput();
    testLoggerLevelFiltering();
    
    // ServerMetrics tests
    testMetricsTickTiming();
    testMetricsCounters();
    testMetricsUptime();
    testMetricsSummary();
    testMetricsResetWindow();

    // Mission system tests
    testMissionAcceptAndComplete();
    testMissionTimeout();
    testMissionAbandon();
    testMissionDuplicatePrevention();

    // Skill system tests
    testSkillTraining();
    testSkillInstantTrain();
    testSkillQueueMultiple();
    testSkillInvalidLevel();

    // Module system tests
    testModuleActivation();
    testModuleCycling();
    testModuleCapDrain();
    testModuleFittingValidation();
    testModuleToggle();

    // Movement command tests
    testMovementOrbitCommand();
    testMovementApproachCommand();
    testMovementStopCommand();
    testMovementWarpDistance();

    // Inventory system tests
    testInventoryAddItem();
    testInventoryCapacityLimit();
    testInventoryRemoveItem();
    testInventoryTransfer();
    testInventoryHasItem();

    // Loot system tests
    testLootGenerate();
    testLootCollect();
    testLootEmptyTable();

    // NpcDatabase tests
    testNpcDatabaseLoad();
    testNpcDatabaseGetNpc();
    testNpcDatabaseHpValues();
    testNpcDatabaseWeapons();
    testNpcDatabaseResistances();
    testNpcDatabaseIds();
    testNpcDatabaseNonexistent();

    // Drone system tests
    testDroneLaunch();
    testDroneRecall();
    testDroneRecallAll();
    testDroneBandwidthLimit();
    testDroneCombatUpdate();
    testDroneDestroyedRemoval();
    testSerializeDeserializeDroneBay();

    // Insurance system tests
    testInsurancePurchase();
    testInsuranceClaim();
    testInsurancePlatinum();
    testInsuranceExpiry();
    testInsuranceInsufficientFunds();

    // Bounty system tests
    testBountyProcessKill();
    testBountyMultipleKills();
    testBountyLedgerRecordLimit();
    testBountyNonexistentPlayer();

    // Market system tests
    testMarketPlaceSellOrder();
    testMarketBuyFromMarket();
    testMarketPriceQueries();
    testMarketOrderExpiry();

    // Corporation system tests
    testCorpCreate();
    testCorpJoin();
    testCorpLeave();
    testCorpCeoCannotLeave();
    testCorpTaxRate();
    testCorpApplyTax();
    testSerializeDeserializeCorporation();

    // Contract system tests
    testContractCreate();
    testContractAccept();
    testContractComplete();
    testContractExpiry();
    testContractStatusQuery();
    testSerializeDeserializeContractBoard();

    // PI system tests
    testPIInstallExtractor();
    testPIInstallProcessor();
    testPIExtractionCycle();
    testPIProcessingCycle();
    testPICpuPowergridLimit();
    testPIStorageCapacityLimit();

    // Manufacturing system tests
    testManufacturingStartJob();
    testManufacturingJobCompletion();
    testManufacturingMultipleRuns();
    testManufacturingJobSlotLimit();
    testManufacturingCancelJob();
    testManufacturingInsufficientFunds();

    // Research system tests
    testResearchME();
    testResearchTE();
    testResearchInvention();
    testResearchInventionFailure();
    testResearchJobSlotLimit();
    testResearchInsufficientFunds();

    // Chat system tests
    testChatJoinChannel();
    testChatLeaveChannel();
    testChatSendMessage();
    testChatMutePlayer();
    testChatUnmutePlayer();
    testChatSetMotd();
    testChatMaxMembers();
    testChatMessageHistory();
    testChatMutedPlayerCannotSend();
    testChatNonMemberCannotSend();

    // Character creation system tests
    testCharacterCreate();
    testCharacterInvalidRace();
    testCharacterInstallImplant();
    testCharacterImplantSlotOccupied();
    testCharacterRemoveImplant();
    testCharacterCloneGrade();
    testCharacterJumpClone();
    testCharacterCloneCooldownDecay();
    testCharacterSecurityStatus();
    testCharacterEmploymentHistory();
    testCharacterRaceAttributes();

    // Tournament system tests
    testTournamentCreate();
    testTournamentRegister();
    testTournamentMaxParticipants();
    testTournamentDuplicateRegister();
    testTournamentStart();
    testTournamentEmptyCannotStart();
    testTournamentScoring();
    testTournamentElimination();
    testTournamentRoundAdvance();
    testTournamentCompletion();
    testTournamentRegisterAfterStart();

    // Leaderboard system tests
    testLeaderboardRecordKill();
    testLeaderboardMultiplePlayers();
    testLeaderboardIskTracking();
    testLeaderboardMissionTracking();
    testLeaderboardRanking();
    testLeaderboardAchievementDefine();
    testLeaderboardAchievementUnlock();
    testLeaderboardAchievementNoDuplicate();
    testLeaderboardNonexistentPlayer();
    testLeaderboardDamageTracking();

    // Station system tests
    testStationCreate();
    testStationDuplicateCreate();
    testStationDockInRange();
    testStationDockOutOfRange();
    testStationUndock();
    testStationUndockNotDocked();
    testStationRepair();
    testStationRepairNoDamage();
    testStationRepairNotDocked();
    testStationDockedCount();
    testStationDoubleDock();
    testStationMovementStopsOnDock();

    // Wreck & Salvage system tests
    testWreckCreate();
    testWreckLifetimeDecay();
    testSalvageWreckInRange();
    testSalvageWreckOutOfRange();
    testSalvageAlreadySalvaged();
    testWreckActiveCount();
    testWreckHasInventory();

    // Server console tests
    testConsoleInit();
    testConsoleHelpCommand();
    testConsoleStatusCommand();
    testConsoleUnknownCommand();
    testConsoleCustomCommand();
    testConsoleLogBuffer();
    testConsoleEmptyCommand();
    testConsoleNotInitialized();
    testConsoleShutdown();
    testConsoleInteractiveMode();

    // Fleet morale system tests
    testFleetMoraleRecordWin();
    testFleetMoraleRecordLoss();
    testFleetMoraleMultipleEvents();
    testFleetMoraleLossStreak();
    testFleetMoraleSavedByPlayer();
    testFleetMoraleMissionTogether();

    // Captain personality system tests
    testCaptainPersonalityAssign();
    testCaptainPersonalityFactionTraits();
    testCaptainPersonalitySetTrait();
    testCaptainPersonalityGetFaction();
    testCaptainPersonalityDeterministic();

    // Fleet chatter system tests
    testFleetChatterSetActivity();
    testFleetChatterGetLine();
    testFleetChatterCooldown();
    testFleetChatterLinesSpoken();
    testFleetChatterCooldownExpires();

    // Warp anomaly system tests
    testWarpAnomalyNoneIfNotCruising();
    testWarpAnomalyNoneIfShortWarp();
    testWarpAnomalyTriggersOnLongWarp();
    testWarpAnomalyCount();
    testWarpAnomalyClear();

    // Captain relationship system tests
    testCaptainRelationshipRecordEvent();
    testCaptainRelationshipAbandoned();
    testCaptainRelationshipStatus();
    testCaptainRelationshipGrudge();
    testCaptainRelationshipMultipleEvents();

    // Emotional arc system tests
    testEmotionalArcVictory();
    testEmotionalArcDefeat();
    testEmotionalArcRest();
    testEmotionalArcTrust();
    testEmotionalArcBetray();

    // Fleet cargo system tests
    testFleetCargoAddContributor();
    testFleetCargoRemoveContributor();
    testFleetCargoMultipleShips();
    testFleetCargoUsedCapacity();
    testFleetCargoGetCapacity();

    // Tactical overlay system tests
    testTacticalOverlayToggle();
    testTacticalOverlayToggleTwice();
    testTacticalOverlaySetToolRange();
    testTacticalOverlayRingDistances();
    testTacticalOverlayDefaultRings();

    // Damage event tests
    testDamageEventOnShieldHit();
    testDamageEventShieldDepleted();
    testDamageEventHullCritical();
    testDamageEventMultipleHits();
    testDamageEventClearOldHits();

    // AI retreat tests
    testAIFleeOnLowHealth();
    testAINoFleeAboveThreshold();
    testAIFleeThresholdCustom();

    // Warp state phase tests
    testWarpStatePhaseAlign();
    testWarpStatePhaseCruise();
    testWarpStatePhaseExit();
    testWarpStateResetOnArrival();
    testWarpStateIntensity();

    // Warp disruption tests
    testWarpDisruptionPreventsWarp();
    testWarpDisruptionInsufficientStrength();
    testIsWarpDisruptedQuery();

    // Ship-class warp speed tests
    testWarpSpeedFromShipComponent();
    testBattleshipSlowerWarp();
    testWarpNoDisruptionWithoutWarpState();

    // AI dynamic orbit tests
    testAIDynamicOrbitFrigate();
    testAIDynamicOrbitCruiser();
    testAIDynamicOrbitBattleship();
    testAIDynamicOrbitCapital();
    testAIDynamicOrbitUnknown();
    testAIEngagementRangeFromWeapon();
    testAIEngagementRangeNoWeapon();
    testAITargetSelectionClosest();
    testAITargetSelectionLowestHP();
    testAIDynamicOrbitApplied();

    // Protocol message tests
    testProtocolDockMessages();
    testProtocolUndockMessage();
    testProtocolRepairMessage();
    testProtocolDamageEventMessage();
    testProtocolDockRequestParse();
    testProtocolWarpMessages();
    testProtocolMovementMessages();

    // Mining system tests
    testMiningCreateDeposit();
    testMiningStartStop();
    testMiningRangeCheck();
    testMiningCycleCompletion();
    testMiningDepletedDeposit();
    testMiningCargoFull();
    testMiningOreStacking();

    // Mining drone tests
    testMiningDroneLaunchAndMine();
    testSalvageDroneLaunchAndSalvage();
    testSalvageDroneAlreadySalvaged();
    testMiningDroneTargetDepletedDeposit();

    // Combat death → wreck integration tests
    testCombatDeathCallbackFires();
    testCombatDeathCallbackWithLoot();
    testCombatNoCallbackOnSurvival();

    // Combat loop integration tests
    testCombatFireWeaponAppliesDamage();
    testCombatFireWeaponCooldownPrevents();
    testCombatFireWeaponOutOfRange();
    testCombatFireWeaponDamageLayerCascade();

    // System resources tests
    testSystemResourcesTracking();

    // New protocol message tests
    testProtocolSalvageMessages();
    testProtocolLootMessages();
    testProtocolMiningMessages();
    testProtocolLootResultParse();
    testProtocolMiningResultParse();

    // AI mining state test
    testAIMiningState();

    // Ship generation model data tests
    testShipModelDataParsed();
    testShipModelDataCapitalShips();
    testShipModelDataAllShipsHaveModelData();
    testShipModelDataSeedUniqueness();
    testShipModelDataEngineCountPositive();
    testShipModelDataMissingReturnsDefaults();

    // Fleet formation tests
    testFleetFormationSetFormation();
    testFleetFormationLeaderAtOrigin();
    testFleetFormationArrowOffsets();
    testFleetFormationLineOffsets();
    testFleetFormationDiamondOffsets();
    testFleetFormationNoneType();

    // Extended captain personality tests
    testCaptainPersonalityNewTraitsAssigned();
    testCaptainPersonalityKeldariHighParanoia();
    testCaptainPersonalitySolariHighLoyalty();
    testCaptainPersonalitySetNewTrait();
    testCaptainPersonalityNewTraitsDeterministic();

    // Captain memory tests
    testCaptainMemoryRecordEvent();
    testCaptainMemoryMultipleEvents();
    testCaptainMemoryAverageWeight();
    testCaptainMemoryMostRecent();
    testCaptainMemoryCapacity();
    testCaptainMemoryNoEntity();

    // Contextual chatter tests
    testContextualChatterReturnsLine();
    testContextualChatterRespectsCooldown();
    testContextualChatterFallbackWithoutPersonality();

    // Ship fitting system tests
    testShipFittingFitModule();
    testShipFittingSlotLimit();
    testShipFittingCPUOverflow();
    testShipFittingRemoveModule();
    testShipFittingValidate();
    testShipFittingSlotCapacity();
    testShipFittingMidLowSlots();

    // Player fleet tests
    testPlayerFleetCreate();
    testPlayerFleetAssignCaptains();
    testPlayerFleetMaxCap();
    testPlayerFleetNotPlayerFleet();

    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << testsPassed << "/" << testsRun << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return (testsPassed == testsRun) ? 0 : 1;
}
