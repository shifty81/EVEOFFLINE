#include "../engine/ecs/ECS.h"
#include <iostream>
#include <cassert>

using namespace atlas::ecs;

void test_create_entity() {
    World world;
    EntityID e1 = world.CreateEntity();
    EntityID e2 = world.CreateEntity();

    assert(e1 != e2);
    assert(world.IsAlive(e1));
    assert(world.IsAlive(e2));
    assert(world.EntityCount() == 2);

    std::cout << "[PASS] test_create_entity" << std::endl;
}

void test_destroy_entity() {
    World world;
    EntityID e1 = world.CreateEntity();
    world.CreateEntity();

    world.DestroyEntity(e1);

    assert(!world.IsAlive(e1));
    assert(world.EntityCount() == 1);

    std::cout << "[PASS] test_destroy_entity" << std::endl;
}

void test_tick_callback() {
    World world;
    float receivedDt = 0.0f;

    world.SetTickCallback([&](float dt) {
        receivedDt = dt;
    });

    world.Update(0.033f);

    assert(receivedDt > 0.03f && receivedDt < 0.04f);
    std::cout << "[PASS] test_tick_callback" << std::endl;
}
