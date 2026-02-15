#include "../engine/core/Engine.h"
#include <iostream>
#include <cassert>

using namespace atlas;

void test_engine_init_and_shutdown() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.maxTicks = 1;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    assert(engine.Running());
    engine.Shutdown();
    assert(!engine.Running());

    std::cout << "[PASS] test_engine_init_and_shutdown" << std::endl;
}

void test_engine_run_loop_ticks() {
    EngineConfig cfg;
    cfg.mode = EngineMode::Server;
    cfg.tickRate = 60;
    cfg.maxTicks = 5;

    Engine engine(cfg);
    engine.InitCore();
    engine.InitECS();
    engine.InitNetworking();

    // Add a tick callback to count ticks
    int tickCount = 0;
    engine.GetWorld().SetTickCallback([&](float dt) {
        tickCount++;
        (void)dt;
    });

    engine.Run();

    assert(tickCount == 5);
    assert(!engine.Running());

    std::cout << "[PASS] test_engine_run_loop_ticks" << std::endl;
}

void test_engine_capabilities() {
    // Editor capabilities
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Editor;
        Engine engine(cfg);
        assert(engine.Can(Capability::AssetWrite));
        assert(engine.Can(Capability::Rendering));
        assert(engine.Can(Capability::GraphEdit));
        assert(engine.Can(Capability::HotReload));
        assert(!engine.Can(Capability::NetAuthority));
    }

    // Server capabilities
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        Engine engine(cfg);
        assert(!engine.Can(Capability::AssetWrite));
        assert(!engine.Can(Capability::Rendering));
        assert(!engine.Can(Capability::GraphEdit));
        assert(engine.Can(Capability::NetAuthority));
    }

    // Client capabilities
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Client;
        Engine engine(cfg);
        assert(!engine.Can(Capability::AssetWrite));
        assert(engine.Can(Capability::Rendering));
        assert(!engine.Can(Capability::GraphEdit));
        assert(!engine.Can(Capability::NetAuthority));
    }

    std::cout << "[PASS] test_engine_capabilities" << std::endl;
}

void test_engine_net_mode_from_config() {
    // Server config should init net in Server mode
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Server;
        cfg.maxTicks = 0;
        Engine engine(cfg);
        engine.InitCore();
        engine.InitNetworking();
        assert(engine.GetNet().Mode() == net::NetMode::Server);
        assert(engine.GetNet().IsAuthority());
    }

    // Client config should init net in Client mode
    {
        EngineConfig cfg;
        cfg.mode = EngineMode::Client;
        Engine engine(cfg);
        engine.InitCore();
        engine.InitNetworking();
        assert(engine.GetNet().Mode() == net::NetMode::Client);
        assert(!engine.GetNet().IsAuthority());
    }

    std::cout << "[PASS] test_engine_net_mode_from_config" << std::endl;
}
