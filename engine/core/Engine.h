#pragma once
#include <string>
#include <cstdint>
#include "../ecs/ECS.h"
#include "../net/NetContext.h"
#include "../sim/TickScheduler.h"

namespace atlas {

enum class RuntimeRole {
    Editor,
    Client,
    Server
};

enum class EngineMode {
    Editor,
    Client,
    Server
};

enum class Capability {
    AssetWrite,
    Rendering,
    Physics,
    GraphEdit,
    GraphExecute,
    NetAuthority,
    HotReload
};

struct EngineConfig {
    EngineMode mode = EngineMode::Client;
    std::string assetRoot = "assets";
    uint32_t tickRate = 30;
    uint32_t maxTicks = 0; // 0 = unlimited (run forever), >0 = stop after N ticks
};

class Engine {
public:
    explicit Engine(const EngineConfig& cfg);
    ~Engine();

    void InitCore();
    void InitRender();
    void InitUI();
    void InitECS();
    void InitNetworking();
    void InitEditor();

    void Run();
    void RunEditor();
    void RunClient();
    void RunServer();

    bool Running() const;
    void Shutdown();

    bool Can(Capability cap) const;

    const EngineConfig& Config() const { return m_config; }

    ecs::World& GetWorld() { return m_world; }
    net::NetContext& GetNet() { return m_net; }
    sim::TickScheduler& GetScheduler() { return m_scheduler; }

private:
    EngineConfig m_config;
    bool m_running = false;
    ecs::World m_world;
    net::NetContext m_net;
    sim::TickScheduler m_scheduler;
};

}
