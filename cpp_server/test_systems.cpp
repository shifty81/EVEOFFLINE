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
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>
#include <memory>

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
    
    const data::ShipTemplate* rifter = db.getShip("rifter");
    if (rifter) {
        assertTrue(rifter->name == "Rifter", "Rifter name correct");
        assertTrue(rifter->ship_class == "Frigate", "Rifter class is Frigate");
        assertTrue(rifter->race == "Minmatar", "Rifter race is Minmatar");
        assertTrue(rifter->shield_hp > 0.0f, "Rifter has shield HP");
        assertTrue(rifter->armor_hp > 0.0f, "Rifter has armor HP");
        assertTrue(rifter->hull_hp > 0.0f, "Rifter has hull HP");
        assertTrue(rifter->cpu > 0.0f, "Rifter has CPU");
        assertTrue(rifter->powergrid > 0.0f, "Rifter has powergrid");
        assertTrue(rifter->max_velocity > 0.0f, "Rifter has velocity");
        assertTrue(rifter->scan_resolution > 0.0f, "Rifter has scan resolution");
        assertTrue(rifter->max_locked_targets > 0, "Rifter has max locked targets");
    } else {
        assertTrue(false, "Rifter template found in database");
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
    
    const data::ShipTemplate* rifter = db.getShip("rifter");
    if (rifter) {
        // Rifter shield: em=0, thermal=20, kinetic=40, explosive=50 (in JSON)
        // Converted to fractions: 0.0, 0.20, 0.40, 0.50
        assertTrue(approxEqual(rifter->shield_resists.em, 0.0f), "Shield EM resist = 0%");
        assertTrue(approxEqual(rifter->shield_resists.thermal, 0.20f), "Shield thermal resist = 20%");
        assertTrue(approxEqual(rifter->shield_resists.kinetic, 0.40f), "Shield kinetic resist = 40%");
        assertTrue(approxEqual(rifter->shield_resists.explosive, 0.50f), "Shield explosive resist = 50%");
        
        // Armor: em=60, thermal=35, kinetic=25, explosive=10
        assertTrue(approxEqual(rifter->armor_resists.em, 0.60f), "Armor EM resist = 60%");
        assertTrue(approxEqual(rifter->armor_resists.thermal, 0.35f), "Armor thermal resist = 35%");
    } else {
        assertTrue(false, "Rifter template found for resistance check");
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
    
    // Check that 'rifter' is in the list
    bool found = false;
    for (const auto& id : ids) {
        if (id == "rifter") found = true;
    }
    assertTrue(found, "rifter is in ship ID list");
}

// ==================== Main ====================

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "EVE OFFLINE C++ Server System Tests" << std::endl;
    std::cout << "CapacitorSystem, ShieldRechargeSystem, WeaponSystem," << std::endl;
    std::cout << "TargetingSystem, ShipDatabase" << std::endl;
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
    
    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << testsPassed << "/" << testsRun << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;
    
    return (testsPassed == testsRun) ? 0 : 1;
}
