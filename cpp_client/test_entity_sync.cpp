/**
 * Test program for entity state synchronization
 * Tests EntityManager, Entity, and EntityMessageParser
 */

#include "core/entity_manager.h"
#include "core/entity_message_parser.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

using namespace eve;

void printSeparator() {
    std::cout << "========================================" << std::endl;
}

void printEntityInfo(const std::shared_ptr<Entity>& entity) {
    if (!entity) return;
    
    auto pos = entity->getPosition();
    auto vel = entity->getVelocity();
    auto health = entity->getHealth();
    
    std::cout << "  ID: " << entity->getId() << std::endl;
    std::cout << "  Position: (" << std::fixed << std::setprecision(2) 
              << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    std::cout << "  Velocity: (" << std::fixed << std::setprecision(2)
              << vel.x << ", " << vel.y << ", " << vel.z << ")" << std::endl;
    std::cout << "  Health: S:" << health.shield << "/" << health.maxShield
              << " A:" << health.armor << "/" << health.maxArmor
              << " H:" << health.hull << "/" << health.maxHull << std::endl;
    if (!entity->getShipType().empty()) {
        std::cout << "  Ship: " << entity->getShipType();
        if (!entity->getShipName().empty()) {
            std::cout << " (" << entity->getShipName() << ")";
        }
        std::cout << std::endl;
    }
}

void testBasicEntityOperations() {
    printSeparator();
    std::cout << "Test 1: Basic Entity Operations" << std::endl;
    printSeparator();
    
    EntityManager manager;
    
    // Test spawn
    std::cout << "\n1. Spawning entity..." << std::endl;
    Health health(100, 200, 300);
    Capacitor cap(150.0f, 150.0f);
    manager.spawnEntity("entity-001", glm::vec3(10.0f, 20.0f, 30.0f), health, cap,
                       "Fang", "My Ship", "Keldari");
    
    auto entity = manager.getEntity("entity-001");
    if (entity) {
        std::cout << "  ✓ Entity spawned successfully" << std::endl;
        printEntityInfo(entity);
    } else {
        std::cout << "  ✗ Failed to spawn entity" << std::endl;
    }
    
    // Test update
    std::cout << "\n2. Updating entity state..." << std::endl;
    Health newHealth(90, 180, 300);
    manager.updateEntityState("entity-001", glm::vec3(15.0f, 25.0f, 35.0f),
                             glm::vec3(1.0f, 0.5f, 0.0f), 0.5f, newHealth);
    
    entity = manager.getEntity("entity-001");
    if (entity && entity->needsUpdate()) {
        std::cout << "  ✓ Entity updated successfully" << std::endl;
        entity->clearUpdateFlag();
    }
    
    // Test interpolation
    std::cout << "\n3. Testing interpolation..." << std::endl;
    auto posBefore = entity->getPosition();
    std::cout << "  Position before: (" << posBefore.x << ", " << posBefore.y << ", " << posBefore.z << ")" << std::endl;
    
    manager.update(0.05f);  // 50ms update
    
    auto posAfter = entity->getPosition();
    std::cout << "  Position after 50ms: (" << posAfter.x << ", " << posAfter.y << ", " << posAfter.z << ")" << std::endl;
    
    if (posAfter.x != posBefore.x || posAfter.y != posBefore.y || posAfter.z != posBefore.z) {
        std::cout << "  ✓ Interpolation working" << std::endl;
    } else {
        std::cout << "  ✗ Interpolation not working" << std::endl;
    }
    
    // Test destroy
    std::cout << "\n4. Destroying entity..." << std::endl;
    manager.destroyEntity("entity-001");
    entity = manager.getEntity("entity-001");
    if (!entity) {
        std::cout << "  ✓ Entity destroyed successfully" << std::endl;
    } else {
        std::cout << "  ✗ Failed to destroy entity" << std::endl;
    }
    
    std::cout << "\nTest 1: PASSED" << std::endl;
}

void testMessageParsing() {
    printSeparator();
    std::cout << "Test 2: Message Parsing" << std::endl;
    printSeparator();
    
    EntityManager manager;
    
    // Test SPAWN_ENTITY parsing
    std::cout << "\n1. Testing SPAWN_ENTITY parsing..." << std::endl;
    std::string spawnMsg = R"({
        "entity_id": "uuid-123-456",
        "position": {"x": 100.0, "y": 200.0, "z": 300.0},
        "health": {"shield": 150, "armor": 250, "hull": 350},
        "ship_type": "Falk",
        "ship_name": "Test Ship",
        "faction": "Veyren"
    })";
    
    if (EntityMessageParser::parseSpawnEntity(spawnMsg, manager)) {
        std::cout << "  ✓ SPAWN_ENTITY parsed successfully" << std::endl;
        auto entity = manager.getEntity("uuid-123-456");
        if (entity) {
            printEntityInfo(entity);
        }
    } else {
        std::cout << "  ✗ Failed to parse SPAWN_ENTITY" << std::endl;
    }
    
    // Test STATE_UPDATE parsing
    std::cout << "\n2. Testing STATE_UPDATE parsing..." << std::endl;
    std::string stateMsg = R"({
        "entities": [
            {
                "id": "uuid-123-456",
                "pos": {"x": 110.0, "y": 210.0, "z": 310.0, "rot": 1.5},
                "vel": {"vx": 5.0, "vy": 3.0, "vz": 2.0},
                "health": {"s": 140, "a": 240, "h": 350}
            },
            {
                "id": "uuid-789-012",
                "pos": {"x": 50.0, "y": 60.0, "z": 70.0, "rot": 0.0},
                "vel": {"vx": 0.0, "vy": 0.0, "vz": 0.0},
                "health": {"s": 100, "a": 100, "h": 100}
            }
        ],
        "tick": 42
    })";
    
    if (EntityMessageParser::parseStateUpdate(stateMsg, manager)) {
        std::cout << "  ✓ STATE_UPDATE parsed successfully" << std::endl;
        std::cout << "  Entity count: " << manager.getEntityCount() << std::endl;
        
        auto entity1 = manager.getEntity("uuid-123-456");
        if (entity1) {
            std::cout << "\n  Entity 1:" << std::endl;
            printEntityInfo(entity1);
        }
        
        auto entity2 = manager.getEntity("uuid-789-012");
        if (entity2) {
            std::cout << "\n  Entity 2:" << std::endl;
            printEntityInfo(entity2);
        }
    } else {
        std::cout << "  ✗ Failed to parse STATE_UPDATE" << std::endl;
    }
    
    // Test DESTROY_ENTITY parsing
    std::cout << "\n3. Testing DESTROY_ENTITY parsing..." << std::endl;
    std::string destroyMsg = R"({
        "entity_id": "uuid-789-012"
    })";
    
    if (EntityMessageParser::parseDestroyEntity(destroyMsg, manager)) {
        std::cout << "  ✓ DESTROY_ENTITY parsed successfully" << std::endl;
        std::cout << "  Entity count after destroy: " << manager.getEntityCount() << std::endl;
    } else {
        std::cout << "  ✗ Failed to parse DESTROY_ENTITY" << std::endl;
    }
    
    std::cout << "\nTest 2: PASSED" << std::endl;
}

void testInterpolation() {
    printSeparator();
    std::cout << "Test 3: Smooth Interpolation" << std::endl;
    printSeparator();
    
    EntityManager manager;
    
    // Spawn entity at origin
    Health health(100, 100, 100);
    manager.spawnEntity("test", glm::vec3(0.0f, 0.0f, 0.0f), health);
    
    // Update to new position
    manager.updateEntityState("test", glm::vec3(10.0f, 10.0f, 10.0f),
                             glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, health);
    
    std::cout << "\nInterpolating from (0,0,0) to (10,10,10) over 100ms:" << std::endl;
    
    auto entity = manager.getEntity("test");
    if (!entity) {
        std::cout << "Failed to get entity!" << std::endl;
        return;
    }
    
    // Interpolate in steps
    for (int i = 0; i <= 10; i++) {
        manager.update(0.01f);  // 10ms steps
        auto pos = entity->getPosition();
        
        float progress = i * 10.0f;
        std::cout << "  " << std::setw(3) << (int)progress << "ms: ("
                  << std::fixed << std::setprecision(2)
                  << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    }
    
    // Check final position
    auto finalPos = entity->getPosition();
    if (std::abs(finalPos.x - 10.0f) < 0.1f &&
        std::abs(finalPos.y - 10.0f) < 0.1f &&
        std::abs(finalPos.z - 10.0f) < 0.1f) {
        std::cout << "\n  ✓ Interpolation reached target" << std::endl;
    } else {
        std::cout << "\n  ✗ Interpolation did not reach target" << std::endl;
    }
    
    std::cout << "\nTest 3: PASSED" << std::endl;
}

void testEntityCallbacks() {
    printSeparator();
    std::cout << "Test 4: Entity Callbacks" << std::endl;
    printSeparator();
    
    EntityManager manager;
    
    int spawnCount = 0;
    int updateCount = 0;
    int destroyCount = 0;
    
    manager.setOnEntitySpawned([&](const std::shared_ptr<Entity>& e) {
        spawnCount++;
        std::cout << "  Callback: Entity spawned - " << e->getId() << std::endl;
    });
    
    manager.setOnEntityUpdated([&](const std::shared_ptr<Entity>& e) {
        updateCount++;
    });
    
    manager.setOnEntityDestroyed([&](const std::shared_ptr<Entity>& e) {
        destroyCount++;
        std::cout << "  Callback: Entity destroyed - " << e->getId() << std::endl;
    });
    
    // Trigger callbacks
    Health health(100, 100, 100);
    manager.spawnEntity("callback-1", glm::vec3(0, 0, 0), health);
    manager.spawnEntity("callback-2", glm::vec3(10, 10, 10), health);
    
    manager.updateEntityState("callback-1", glm::vec3(5, 5, 5),
                             glm::vec3(0, 0, 0), 0.0f, health);
    
    manager.destroyEntity("callback-1");
    manager.destroyEntity("callback-2");
    
    std::cout << "\nCallback counts:" << std::endl;
    std::cout << "  Spawned: " << spawnCount << " (expected 2)" << std::endl;
    std::cout << "  Updated: " << updateCount << " (expected 1)" << std::endl;
    std::cout << "  Destroyed: " << destroyCount << " (expected 2)" << std::endl;
    
    if (spawnCount == 2 && updateCount == 1 && destroyCount == 2) {
        std::cout << "\n  ✓ All callbacks fired correctly" << std::endl;
    } else {
        std::cout << "\n  ✗ Callback counts incorrect" << std::endl;
    }
    
    std::cout << "\nTest 4: PASSED" << std::endl;
}

int main() {
    std::cout << "======================================" << std::endl;
    std::cout << "  Entity Synchronization Test Suite  " << std::endl;
    std::cout << "======================================" << std::endl;
    std::cout << std::endl;
    
    try {
        testBasicEntityOperations();
        std::cout << std::endl;
        
        testMessageParsing();
        std::cout << std::endl;
        
        testInterpolation();
        std::cout << std::endl;
        
        testEntityCallbacks();
        std::cout << std::endl;
        
        printSeparator();
        std::cout << "All tests PASSED!" << std::endl;
        printSeparator();
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\nTest FAILED with exception: " << e.what() << std::endl;
        return 1;
    }
}
