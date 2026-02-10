/**
 * Test CapacitorSystem, ShieldRechargeSystem, and WeaponSystem
 * 
 * Tests the three new dedicated ECS systems for the C++ server:
 * - CapacitorSystem: capacitor recharge and consumption
 * - ShieldRechargeSystem: passive shield regeneration
 * - WeaponSystem: weapon cooldowns, auto-fire, capacitor cost, damage cascade
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
#include "data/world_persistence.h"
#include "data/npc_database.h"
#include "systems/movement_system.h"
#include "utils/logger.h"
#include "utils/server_metrics.h"
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>
#include <memory>
#include <fstream>
#include <thread>

using namespace eve;

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

// ==================== Main ====================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "EVE OFFLINE C++ Server System Tests" << std::endl;
    std::cout << "Capacitor, Shield, Weapon, Targeting," << std::endl;
    std::cout << "ShipDB, WormholeDB, Wormhole, Fleet," << std::endl;
    std::cout << "Mission, Skill, Module, Inventory," << std::endl;
    std::cout << "Loot, NpcDB, Drone, Insurance," << std::endl;
    std::cout << "WorldPersistence, Interdictors, StealthBombers," << std::endl;
    std::cout << "Logger, ServerMetrics" << std::endl;
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

    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << testsPassed << "/" << testsRun << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return (testsPassed == testsRun) ? 0 : 1;
}
