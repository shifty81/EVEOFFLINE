#include "server.h"
#include "game_session.h"
#include "systems/movement_system.h"
#include "systems/combat_system.h"
#include "systems/ai_system.h"
#include "systems/targeting_system.h"
#include "systems/capacitor_system.h"
#include "systems/shield_recharge_system.h"
#include "systems/weapon_system.h"
#include "systems/station_system.h"
#include "utils/logger.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <sys/stat.h>

namespace atlas {

Server::Server(const std::string& config_path)
    : running_(false) {
    
    config_ = std::make_unique<ServerConfig>();
    if (!config_->loadFromFile(config_path)) {
        utils::Logger::instance().warn(
            "Could not load config from " + config_path + ", using defaults");
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

    auto station = std::make_unique<systems::StationSystem>(game_world_.get());
    station_system_ = station.get();
    game_world_->addSystem(std::move(station));

    auto movement = std::make_unique<systems::MovementSystem>(game_world_.get());
    movement_system_ = movement.get();
    game_world_->addSystem(std::move(movement));
    game_world_->addSystem(std::make_unique<systems::WeaponSystem>(game_world_.get()));
    auto combat = std::make_unique<systems::CombatSystem>(game_world_.get());
    combat_system_ = combat.get();
    game_world_->addSystem(std::move(combat));
    
    auto& log = utils::Logger::instance();
    log.info("Game world initialized with " +
             std::to_string(game_world_->getEntityCount()) + " entities");
    log.info("Systems: Capacitor, ShieldRecharge, AI, Targeting, Station, Movement, Weapon, Combat");
}

bool Server::initialize() {
    auto& log = utils::Logger::instance();

    // Initialize file logging using the configured log_path
    log.init(config_->log_path);

    log.info("==================================");
    log.info("EVE OFFLINE Dedicated Server");
    log.info("==================================");
    log.info("Version: 1.0.0");
    
    // Initialize TCP server
    tcp_server_ = std::make_unique<network::TCPServer>(
        config_->host, 
        config_->port, 
        config_->max_connections
    );
    
    if (!tcp_server_->initialize()) {
        log.error("Failed to initialize TCP server");
        return false;
    }
    
    log.info("Server listening on " + config_->host + ":" +
             std::to_string(config_->port));
    
    // Initialize Steam if enabled
    if (config_->use_steam) {
        steam_auth_ = std::make_unique<auth::SteamAuth>();
        if (steam_auth_->initialize(config_->steam_app_id)) {
            log.info("Steam integration enabled");
            
            if (config_->steam_server_browser) {
                steam_auth_->registerServer(
                    config_->server_name,
                    "Space"
                );
                log.info("Registered with Steam server browser");
            }
        } else {
            log.warn("Steam initialization failed, continuing without Steam");
            config_->use_steam = false;
        }
    }
    
    // Initialize whitelist if enabled
    if (config_->use_whitelist) {
        whitelist_ = std::make_unique<auth::Whitelist>();
        if (whitelist_->loadFromFile("config/whitelist.json")) {
            log.info("Whitelist enabled with " +
                     std::to_string(whitelist_->getSteamNames().size()) +
                     " Steam names");
        } else {
            log.warn("Could not load whitelist, creating empty whitelist");
        }
    }
    
    log.info("Server Configuration:");
    log.info("  Server Name: " + config_->server_name);
    log.info("  Public Server: " + std::string(config_->public_server ? "Yes" : "No"));
    log.info("  Persistent World: " + std::string(config_->persistent_world ? "Yes" : "No"));
    log.info("  Whitelist: " + std::string(config_->use_whitelist ? "Enabled" : "Disabled"));
    log.info("  Steam Integration: " + std::string(config_->use_steam ? "Enabled" : "Disabled"));
    log.info("  Max Players: " + std::to_string(config_->max_connections));
    log.info("  Tick Rate: " + std::to_string(static_cast<int>(config_->tick_rate)) + " Hz");
    log.info("  Log Path: " + config_->log_path);
    
    // Initialize game world and systems
    initializeGameWorld();
    
    // Initialize game session (bridges networking ↔ ECS world)
    game_session_ = std::make_unique<GameSession>(
        game_world_.get(), tcp_server_.get(), config_->data_path);
    game_session_->setTargetingSystem(targeting_system_);
    game_session_->setStationSystem(station_system_);
    game_session_->setMovementSystem(movement_system_);
    game_session_->setCombatSystem(combat_system_);
    game_session_->initialize();
    
    // Initialize server console
    console_.setInteractive(true);  // Enable interactive mode by default
    console_.init(*this, *config_);
    
    return true;
}

void Server::start() {
    if (running_) {
        utils::Logger::instance().warn("Server is already running");
        return;
    }
    
    running_ = true;
    tcp_server_->start();
    
    utils::Logger::instance().info("Server started! Ready for connections.");
    utils::Logger::instance().info("Press Ctrl+C to stop the server.");
}

void Server::stop() {
    if (!running_) {
        return;
    }
    
    auto& log = utils::Logger::instance();
    log.info("Stopping server...");
    log.info(metrics_.summary());
    running_ = false;
    
    console_.shutdown();
    
    if (tcp_server_) {
        tcp_server_->stop();
    }
    
    if (steam_auth_) {
        steam_auth_->shutdown();
    }
    
    log.info("Server stopped.");
    log.shutdown();
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
        metrics_.recordTickStart();
        
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
        
        // Update console (process user input)
        console_.update();
        
        // Auto-save check
        if (config_->auto_save && config_->persistent_world) {
            auto now = std::chrono::steady_clock::now();
            if (now - last_save_time >= save_interval) {
                saveWorld();
                last_save_time = now;
            }
        }

        metrics_.recordTickEnd();

        // Update entity / player counters and emit periodic stats
        metrics_.setEntityCount(static_cast<int>(game_world_->getEntityCount()));
        metrics_.setPlayerCount(getPlayerCount());
        metrics_.logSummaryIfDue(60.0);
        
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
        int ret;
#ifdef _WIN32
        ret = _mkdir(config_->save_path.c_str());
#else
        ret = mkdir(config_->save_path.c_str(), 0755);
#endif
        if (ret != 0) {
            utils::Logger::instance().error(
                "[AutoSave] Failed to create save directory: " + config_->save_path);
            return false;
        }
    }

    std::string filepath = config_->save_path + "/world_state.json";
    utils::Logger::instance().info("[AutoSave] Saving world state...");
    return world_persistence_.saveWorld(game_world_.get(), filepath);
}

bool Server::loadWorld() {
    std::string filepath = config_->save_path + "/world_state.json";
    return world_persistence_.loadWorld(game_world_.get(), filepath);
}

} // namespace atlas
