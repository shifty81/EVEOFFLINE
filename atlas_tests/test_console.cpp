#include "../editor/panels/ConsolePanel.h"
#include "../engine/ecs/ECS.h"
#include "../engine/net/NetContext.h"
#include "../engine/sim/TickScheduler.h"
#include <iostream>
#include <cassert>
#include <string>

using namespace atlas::editor;
using namespace atlas::ecs;
using namespace atlas::net;
using namespace atlas::sim;

void test_console_spawn_entity() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);

    assert(world.EntityCount() == 0);
    console.Execute("spawn_entity");
    assert(world.EntityCount() == 1);

    // Check history
    auto& history = console.History();
    assert(history.size() == 2); // "> spawn_entity" and "Created entity N"
    assert(history[0] == "> spawn_entity");

    std::cout << "[PASS] test_console_spawn_entity" << std::endl;
}

void test_console_ecs_dump() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);

    world.CreateEntity();
    world.CreateEntity();

    console.Execute("ecs.dump");

    auto& history = console.History();
    // "> ecs.dump", "Entities: 2", "  Entity 1 (0 components)", "  Entity 2 (0 components)"
    assert(history.size() == 4);
    assert(history[1] == "Entities: 2");

    std::cout << "[PASS] test_console_ecs_dump" << std::endl;
}

void test_console_set_tickrate() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;
    scheduler.SetTickRate(30);

    ConsolePanel console(world, net, scheduler);

    console.Execute("set tickrate 60");

    assert(scheduler.TickRate() == 60);

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[1] == "Tick rate set to 60");

    std::cout << "[PASS] test_console_set_tickrate" << std::endl;
}

void test_console_net_mode() {
    World world;
    NetContext net;
    net.Init(NetMode::Server);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);

    console.Execute("net.mode");

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[1] == "Net mode: Server");

    std::cout << "[PASS] test_console_net_mode" << std::endl;
}

void test_console_help() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);

    console.Execute("help");

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[0] == "> help");

    std::cout << "[PASS] test_console_help" << std::endl;
}

void test_console_unknown_command() {
    World world;
    NetContext net;
    net.Init(NetMode::Standalone);
    TickScheduler scheduler;

    ConsolePanel console(world, net, scheduler);

    console.Execute("foobar");

    auto& history = console.History();
    assert(history.size() == 2);
    assert(history[1] == "Unknown command: foobar");

    std::cout << "[PASS] test_console_unknown_command" << std::endl;
}
