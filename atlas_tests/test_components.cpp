#include "../engine/ecs/ECS.h"
#include <iostream>
#include <cassert>
#include <string>

using namespace atlas::ecs;

struct Position {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct Velocity {
    float dx = 0.0f;
    float dy = 0.0f;
    float dz = 0.0f;
};

struct Health {
    int current = 100;
    int max = 100;
};

void test_add_and_get_component() {
    World world;
    EntityID e = world.CreateEntity();

    Position pos{1.0f, 2.0f, 3.0f};
    world.AddComponent<Position>(e, pos);

    Position* retrieved = world.GetComponent<Position>(e);
    assert(retrieved != nullptr);
    assert(retrieved->x == 1.0f);
    assert(retrieved->y == 2.0f);
    assert(retrieved->z == 3.0f);

    std::cout << "[PASS] test_add_and_get_component" << std::endl;
}

void test_has_component() {
    World world;
    EntityID e = world.CreateEntity();

    assert(!world.HasComponent<Position>(e));

    Position pos{0, 0, 0};
    world.AddComponent<Position>(e, pos);

    assert(world.HasComponent<Position>(e));
    assert(!world.HasComponent<Velocity>(e));

    std::cout << "[PASS] test_has_component" << std::endl;
}

void test_remove_component() {
    World world;
    EntityID e = world.CreateEntity();

    Position pos{5.0f, 5.0f, 5.0f};
    world.AddComponent<Position>(e, pos);
    assert(world.HasComponent<Position>(e));

    world.RemoveComponent<Position>(e);
    assert(!world.HasComponent<Position>(e));
    assert(world.GetComponent<Position>(e) == nullptr);

    std::cout << "[PASS] test_remove_component" << std::endl;
}

void test_multiple_components() {
    World world;
    EntityID e = world.CreateEntity();

    Position pos{1.0f, 2.0f, 3.0f};
    Velocity vel{0.1f, 0.2f, 0.3f};
    Health hp{50, 100};

    world.AddComponent<Position>(e, pos);
    world.AddComponent<Velocity>(e, vel);
    world.AddComponent<Health>(e, hp);

    assert(world.HasComponent<Position>(e));
    assert(world.HasComponent<Velocity>(e));
    assert(world.HasComponent<Health>(e));

    auto* p = world.GetComponent<Position>(e);
    auto* v = world.GetComponent<Velocity>(e);
    auto* h = world.GetComponent<Health>(e);

    assert(p && p->x == 1.0f);
    assert(v && v->dx == 0.1f);
    assert(h && h->current == 50);

    auto types = world.GetComponentTypes(e);
    assert(types.size() == 3);

    std::cout << "[PASS] test_multiple_components" << std::endl;
}

void test_destroy_entity_removes_components() {
    World world;
    EntityID e = world.CreateEntity();

    Position pos{1.0f, 2.0f, 3.0f};
    world.AddComponent<Position>(e, pos);
    assert(world.HasComponent<Position>(e));

    world.DestroyEntity(e);
    assert(!world.IsAlive(e));
    // Components should be cleaned up
    assert(!world.HasComponent<Position>(e));

    std::cout << "[PASS] test_destroy_entity_removes_components" << std::endl;
}

void test_component_update() {
    World world;
    EntityID e = world.CreateEntity();

    Position pos{0.0f, 0.0f, 0.0f};
    world.AddComponent<Position>(e, pos);

    // Modify component in place
    auto* p = world.GetComponent<Position>(e);
    assert(p != nullptr);
    p->x = 10.0f;
    p->y = 20.0f;

    // Verify the modification persists
    auto* p2 = world.GetComponent<Position>(e);
    assert(p2 != nullptr);
    assert(p2->x == 10.0f);
    assert(p2->y == 20.0f);

    std::cout << "[PASS] test_component_update" << std::endl;
}
