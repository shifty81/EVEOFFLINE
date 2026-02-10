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
#include "data/world_persistence.h"
#include "systems/movement_system.h"
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>
#include <memory>
#include <fstream>

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
    assertTrue(approxEqual(lstandings->corporation_standings["Venom Syndicate"], -7.5f), "Corporation standing (Serpentis) preserved");
    
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

// ==================== Main ====================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "EVE OFFLINE C++ Server System Tests" << std::endl;
    std::cout << "Capacitor, Shield, Weapon, Targeting," << std::endl;
    std::cout << "ShipDB, WormholeDB, Wormhole, Fleet," << std::endl;
    std::cout << "WorldPersistence, Interdictors, StealthBombers" << std::endl;
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
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << testsPassed << "/" << testsRun << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return (testsPassed == testsRun) ? 0 : 1;
}
