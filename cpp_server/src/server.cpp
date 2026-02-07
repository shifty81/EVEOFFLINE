#include "server.h"
#include "game_session.h"
#include "systems/movement_system.h"
#include "systems/combat_system.h"
#include "systems/ai_system.h"
#include "systems/targeting_system.h"
#include "systems/capacitor_system.h"
#include "systems/shield_recharge_system.h"
#include "systems/weapon_system.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sys/stat.h>

namespace eve {

Server::Server(const std::string& config_path)
    : running_(false) {
    
    config_ = std::make_unique<ServerConfig>();
    if (!config_->loadFromFile(config_path)) {
        std::cerr << "Warning: Could not load config from " << config_path 
                  << ", using defaults" << std::endl;
    }
    
    // Initialize game world
    game_world_ = std::make_unique<ecs::World>();
}

Server::~Server() {
    stop();
}

void Server::initializeGameWorld() {
    // Initialize game systems in order
    game_world_->addSystem(std::make_unique<systems::CapacitorSystem>(game_world_.get()));
    game_world_->addSystem(std::make_unique<systems::ShieldRechargeSystem>(game_world_.get()));
    game_world_->addSystem(std::make_unique<systems::AISystem>(game_world_.get()));

    auto targeting = std::make_unique<systems::TargetingSystem>(game_world_.get());
    targeting_system_ = targeting.get();
    game_world_->addSystem(std::move(targeting));

    game_world_->addSystem(std::make_unique<systems::MovementSystem>(game_world_.get()));
    game_world_->addSystem(std::make_unique<systems::WeaponSystem>(game_world_.get()));
    game_world_->addSystem(std::make_unique<systems::CombatSystem>(game_world_.get()));
    
    std::cout << "Game world initialized with " << game_world_->getEntityCount() << " entities" << std::endl;
    std::cout << "Systems: Capacitor, ShieldRecharge, AI, Targeting, Movement, Weapon, Combat" << std::endl;
}

bool Server::initialize() {
    std::cout << "==================================" << std::endl;
    std::cout << "EVE OFFLINE Dedicated Server" << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "Version: 1.0.0" << std::endl;
    std::cout << std::endl;
    
    // Initialize TCP server
    tcp_server_ = std::make_unique<network::TCPServer>(
        config_->host, 
        config_->port, 
        config_->max_connections
    );
    
    if (!tcp_server_->initialize()) {
        std::cerr << "Failed to initialize TCP server" << std::endl;
        return false;
    }
    
    std::cout << "Server listening on " << config_->host << ":" << config_->port << std::endl;
    
    // Initialize Steam if enabled
    if (config_->use_steam) {
        steam_auth_ = std::make_unique<auth::SteamAuth>();
        if (steam_auth_->initialize(config_->steam_app_id)) {
            std::cout << "Steam integration enabled" << std::endl;
            
            if (config_->steam_server_browser) {
                steam_auth_->registerServer(
                    config_->server_name,
                    "Space"
                );
                std::cout << "Registered with Steam server browser" << std::endl;
            }
        } else {
            std::cout << "Warning: Steam initialization failed, continuing without Steam" << std::endl;
            config_->use_steam = false;
        }
    }
    
    // Initialize whitelist if enabled
    if (config_->use_whitelist) {
        whitelist_ = std::make_unique<auth::Whitelist>();
        if (whitelist_->loadFromFile("config/whitelist.json")) {
            std::cout << "Whitelist enabled with " 
                     << whitelist_->getSteamNames().size() << " Steam names" << std::endl;
        } else {
            std::cout << "Warning: Could not load whitelist, creating empty whitelist" << std::endl;
        }
    }
    
    std::cout << std::endl;
    std::cout << "Server Configuration:" << std::endl;
    std::cout << "  Server Name: " << config_->server_name << std::endl;
    std::cout << "  Public Server: " << (config_->public_server ? "Yes" : "No") << std::endl;
    std::cout << "  Persistent World: " << (config_->persistent_world ? "Yes" : "No") << std::endl;
    std::cout << "  Whitelist: " << (config_->use_whitelist ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  Steam Integration: " << (config_->use_steam ? "Enabled" : "Disabled") << std::endl;
    std::cout << "  Max Players: " << config_->max_connections << std::endl;
    std::cout << "  Tick Rate: " << config_->tick_rate << " Hz" << std::endl;
    std::cout << std::endl;
    
    // Initialize game world and systems
    initializeGameWorld();
    
    // Initialize game session (bridges networking ↔ ECS world)
    game_session_ = std::make_unique<GameSession>(
        game_world_.get(), tcp_server_.get(), config_->data_path);
    game_session_->setTargetingSystem(targeting_system_);
    game_session_->initialize();
    
    return true;
}

void Server::start() {
    if (running_) {
        std::cerr << "Server is already running" << std::endl;
        return;
    }
    
    running_ = true;
    tcp_server_->start();
    
    std::cout << "Server started! Ready for connections." << std::endl;
    std::cout << "Press Ctrl+C to stop the server." << std::endl;
    std::cout << std::endl;
}

void Server::stop() {
    if (!running_) {
        return;
    }
    
    std::cout << std::endl << "Stopping server..." << std::endl;
    running_ = false;
    
    if (tcp_server_) {
        tcp_server_->stop();
    }
    
    if (steam_auth_) {
        steam_auth_->shutdown();
    }
    
    std::cout << "Server stopped." << std::endl;
}

void Server::run() {
    start();
    mainLoop();
}

void Server::mainLoop() {
    const float tick_duration = 1.0f / config_->tick_rate;
    const auto tick_ms = std::chrono::milliseconds(static_cast<int>(tick_duration * 1000));
    
    auto last_save_time = std::chrono::steady_clock::now();
    const auto save_interval = std::chrono::seconds(config_->save_interval_seconds);
    
    while (running_) {
        auto frame_start = std::chrono::steady_clock::now();
        
        // Update game world (ECS systems)
        game_world_->update(tick_duration);
        
        // Broadcast state to all connected clients
        if (game_session_) {
            game_session_->update(tick_duration);
        }
        
        // Update Steam callbacks
        if (config_->use_steam && steam_auth_) {
            updateSteam();
        }
        
        // Auto-save check
        if (config_->auto_save && config_->persistent_world) {
            auto now = std::chrono::steady_clock::now();
            if (now - last_save_time >= save_interval) {
                saveWorld();
                last_save_time = now;
            }
        }
        
        // Sleep for remaining tick time
        auto frame_end = std::chrono::steady_clock::now();
        auto frame_duration = frame_end - frame_start;
        if (frame_duration < tick_ms) {
            std::this_thread::sleep_for(tick_ms - frame_duration);
        }
    }
}

void Server::updateSteam() {
    if (steam_auth_ && steam_auth_->isInitialized()) {
        steam_auth_->update();
        
        // Update server info in Steam server browser
        if (config_->steam_server_browser) {
            steam_auth_->updateServerInfo(
                getPlayerCount(),
                config_->max_connections
            );
        }
    }
}

int Server::getPlayerCount() const {
    return game_session_ ? game_session_->getPlayerCount()
                         : (tcp_server_ ? tcp_server_->getClientCount() : 0);
}

bool Server::saveWorld() {
    // Ensure save directory exists
    struct stat st;
    if (stat(config_->save_path.c_str(), &st) != 0) {
#ifdef _WIN32
        _mkdir(config_->save_path.c_str());
#else
        mkdir(config_->save_path.c_str(), 0755);
#endif
    }

    std::string filepath = config_->save_path + "/world_state.json";
    std::cout << "[AutoSave] Saving world state..." << std::endl;
    return world_persistence_.saveWorld(game_world_.get(), filepath);
}

bool Server::loadWorld() {
    std::string filepath = config_->save_path + "/world_state.json";
    return world_persistence_.loadWorld(game_world_.get(), filepath);
}

} // namespace eve
