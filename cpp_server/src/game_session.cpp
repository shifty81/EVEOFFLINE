#include "game_session.h"
#include "components/game_components.h"
#include <iostream>
#include <sstream>
#include <cmath>

namespace eve {

// Named constants
static constexpr float NPC_AWARENESS_RANGE = 50000.0f;
static constexpr float PLAYER_SPAWN_SPACING_X = 50.0f;
static constexpr float PLAYER_SPAWN_SPACING_Z = 30.0f;
static constexpr size_t MAX_CHARACTER_NAME_LEN = 32;
static constexpr size_t MAX_CHAT_MESSAGE_LEN = 256;

// Escape a string for safe embedding in JSON values
static std::string escapeJsonString(const std::string& input) {
    std::string result;
    result.reserve(input.size());
    for (char c : input) {
        switch (c) {
            case '\"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n";  break;
            case '\r': result += "\\r";  break;
            case '\t': result += "\\t";  break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    // Skip other control characters
                } else {
                    result += c;
                }
                break;
        }
    }
    return result;
}

// ---------------------------------------------------------------------------
// Construction / Initialization
// ---------------------------------------------------------------------------

GameSession::GameSession(ecs::World* world, network::TCPServer* tcp_server)
    : world_(world)
    , tcp_server_(tcp_server) {
}

void GameSession::initialize() {
    // Register the message handler on the TCP server
    tcp_server_->setMessageHandler(
        [this](const network::ClientConnection& client, const std::string& raw) {
            onClientMessage(client, raw);
        }
    );

    // Spawn a handful of NPC enemies so the world isn't empty
    spawnInitialNPCs();

    std::cout << "[GameSession] Initialized – "
              << world_->getEntityCount() << " entities in world" << std::endl;
}

// ---------------------------------------------------------------------------
// Per-tick update
// ---------------------------------------------------------------------------

void GameSession::update(float /*delta_time*/) {
    // Build a single state-update message and broadcast it to every client
    std::string state_msg = buildStateUpdate();

    std::lock_guard<std::mutex> lock(players_mutex_);
    for (const auto& kv : players_) {
        tcp_server_->sendToClient(kv.second.connection, state_msg);
    }
}

int GameSession::getPlayerCount() const {
    std::lock_guard<std::mutex> lock(players_mutex_);
    return static_cast<int>(players_.size());
}

// ---------------------------------------------------------------------------
// Incoming message dispatch
// ---------------------------------------------------------------------------

void GameSession::onClientMessage(const network::ClientConnection& client,
                                  const std::string& raw) {
    network::MessageType type;
    std::string data;

    if (!protocol_.parseMessage(raw, type, data)) {
        std::cerr << "[GameSession] Unrecognised message from "
                  << client.address << std::endl;
        return;
    }

    switch (type) {
        case network::MessageType::CONNECT:
            handleConnect(client, data);
            break;
        case network::MessageType::DISCONNECT:
            handleDisconnect(client);
            break;
        case network::MessageType::INPUT_MOVE:
            handleInputMove(client, data);
            break;
        case network::MessageType::CHAT:
            handleChat(client, data);
            break;
        default:
            break;
    }
}

// ---------------------------------------------------------------------------
// CONNECT handler
// ---------------------------------------------------------------------------

void GameSession::handleConnect(const network::ClientConnection& client,
                                const std::string& data) {
    // Reject duplicate connections from the same socket
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        if (players_.find(static_cast<int>(client.socket)) != players_.end()) {
            std::cerr << "[GameSession] Duplicate connect from "
                      << client.address << ", ignoring" << std::endl;
            return;
        }
    }

    std::string player_id   = extractJsonString(data, "player_id");
    std::string char_name   = extractJsonString(data, "character_name");

    if (player_id.empty()) {
        player_id = "player_" + std::to_string(client.socket);
    }
    if (char_name.empty()) {
        char_name = "Pilot";
    }

    // Enforce length limits
    if (char_name.size() > MAX_CHARACTER_NAME_LEN) {
        char_name.resize(MAX_CHARACTER_NAME_LEN);
    }

    // Create the player's ship entity in the game world
    std::string entity_id = createPlayerEntity(player_id, char_name);

    // Record the mapping and snapshot other players for notification
    std::vector<PlayerInfo> others;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        PlayerInfo info;
        info.entity_id      = entity_id;
        info.character_name  = char_name;
        info.connection      = client;
        players_[static_cast<int>(client.socket)] = info;

        for (const auto& kv : players_) {
            if (kv.first != static_cast<int>(client.socket)) {
                others.push_back(kv.second);
            }
        }
    }

    // Escape char_name for safe JSON embedding
    std::string safe_name = escapeJsonString(char_name);

    // Send connect_ack with the player's entity id
    std::ostringstream ack;
    ack << "{\"type\":\"connect_ack\","
        << "\"data\":{"
        << "\"success\":true,"
        << "\"player_entity_id\":\"" << entity_id << "\","
        << "\"message\":\"Welcome, " << safe_name << "!\""
        << "}}";
    tcp_server_->sendToClient(client, ack.str());

    // Send spawn_entity messages for every existing entity
    for (auto* entity : world_->getAllEntities()) {
        std::string spawn_msg = buildSpawnEntity(entity->getId());
        tcp_server_->sendToClient(client, spawn_msg);
    }

    std::cout << "[GameSession] Player connected: " << char_name
              << " (entity " << entity_id << ")" << std::endl;

    // Notify other clients about the new player entity
    std::string new_spawn = buildSpawnEntity(entity_id);
    for (const auto& other : others) {
        tcp_server_->sendToClient(other.connection, new_spawn);
    }
}

// ---------------------------------------------------------------------------
// DISCONNECT handler
// ---------------------------------------------------------------------------

void GameSession::handleDisconnect(const network::ClientConnection& client) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it != players_.end()) {
            entity_id = it->second.entity_id;
            std::cout << "[GameSession] Player disconnected: "
                      << it->second.character_name << std::endl;
            players_.erase(it);
        }
    }

    if (!entity_id.empty()) {
        world_->destroyEntity(entity_id);

        // Tell remaining clients to remove the entity
        std::ostringstream msg;
        msg << "{\"type\":\"destroy_entity\","
            << "\"data\":{\"entity_id\":\"" << entity_id << "\"}}";
        std::string destroy_msg = msg.str();

        std::lock_guard<std::mutex> lock(players_mutex_);
        for (const auto& kv : players_) {
            tcp_server_->sendToClient(kv.second.connection, destroy_msg);
        }
    }
}

// ---------------------------------------------------------------------------
// INPUT_MOVE handler
// ---------------------------------------------------------------------------

void GameSession::handleInputMove(const network::ClientConnection& client,
                                  const std::string& data) {
    std::string entity_id;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it == players_.end()) return;
        entity_id = it->second.entity_id;
    }

    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* vel = entity->getComponent<components::Velocity>();
    if (!vel) return;

    // Parse velocity – the client sends {"velocity":{"x":..,"y":..,"z":..}}
    // Our lightweight parser operates on the inner data block.
    float vx = extractJsonFloat(data, "\"x\":", 0.0f);
    float vy = extractJsonFloat(data, "\"y\":", 0.0f);
    float vz = extractJsonFloat(data, "\"z\":", 0.0f);

    vel->vx = vx;
    vel->vy = vy;
    vel->vz = vz;
}

// ---------------------------------------------------------------------------
// CHAT handler
// ---------------------------------------------------------------------------

void GameSession::handleChat(const network::ClientConnection& client,
                             const std::string& data) {
    std::string sender;
    {
        std::lock_guard<std::mutex> lock(players_mutex_);
        auto it = players_.find(static_cast<int>(client.socket));
        if (it != players_.end()) {
            sender = it->second.character_name;
        }
    }

    std::string message = extractJsonString(data, "message");

    // Enforce message length limit
    if (message.size() > MAX_CHAT_MESSAGE_LEN) {
        message.resize(MAX_CHAT_MESSAGE_LEN);
    }

    // Escape for safe JSON embedding
    std::string chat_msg = protocol_.createChatMessage(
        escapeJsonString(sender), escapeJsonString(message));

    // Broadcast chat to everyone
    tcp_server_->broadcastToAll(chat_msg);
}

// ---------------------------------------------------------------------------
// State broadcast helpers
// ---------------------------------------------------------------------------

std::string GameSession::buildStateUpdate() const {
    std::ostringstream json;
    json << "{\"type\":\"state_update\",\"data\":{\"entities\":[";

    auto entities = world_->getAllEntities();
    bool first = true;
    for (const auto* entity : entities) {
        if (!first) json << ",";
        first = false;

        auto* pos  = entity->getComponent<components::Position>();
        auto* vel  = entity->getComponent<components::Velocity>();
        auto* hp   = entity->getComponent<components::Health>();

        json << "{\"id\":\"" << entity->getId() << "\"";

        // Position
        if (pos) {
            json << ",\"pos\":{\"x\":" << pos->x
                 << ",\"y\":" << pos->y
                 << ",\"z\":" << pos->z
                 << ",\"rot\":" << pos->rotation << "}";
        }

        // Velocity
        if (vel) {
            json << ",\"vel\":{\"vx\":" << vel->vx
                 << ",\"vy\":" << vel->vy
                 << ",\"vz\":" << vel->vz << "}";
        }

        // Health
        if (hp) {
            json << ",\"health\":{"
                 << "\"shield\":" << hp->shield_hp
                 << ",\"armor\":" << hp->armor_hp
                 << ",\"hull\":" << hp->hull_hp
                 << ",\"max_shield\":" << hp->shield_max
                 << ",\"max_armor\":" << hp->armor_max
                 << ",\"max_hull\":" << hp->hull_max
                 << "}";
        }

        json << "}";
    }

    json << "]}}";
    return json.str();
}

std::string GameSession::buildSpawnEntity(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "{}";

    auto* pos  = entity->getComponent<components::Position>();
    auto* hp   = entity->getComponent<components::Health>();
    auto* ship = entity->getComponent<components::Ship>();
    auto* fac  = entity->getComponent<components::Faction>();

    std::ostringstream json;
    json << "{\"type\":\"spawn_entity\",\"data\":{";
    json << "\"entity_id\":\"" << entity_id << "\"";

    if (pos) {
        json << ",\"position\":{\"x\":" << pos->x
             << ",\"y\":" << pos->y
             << ",\"z\":" << pos->z << "}";
    }

    if (hp) {
        json << ",\"health\":{"
             << "\"shield\":" << hp->shield_hp
             << ",\"armor\":" << hp->armor_hp
             << ",\"hull\":" << hp->hull_hp
             << ",\"max_shield\":" << hp->shield_max
             << ",\"max_armor\":" << hp->armor_max
             << ",\"max_hull\":" << hp->hull_max << "}";
    }

    if (ship) {
        json << ",\"ship_type\":\"" << ship->ship_type << "\"";
        json << ",\"ship_name\":\"" << ship->ship_name << "\"";
    }

    if (fac) {
        json << ",\"faction\":\"" << fac->faction_name << "\"";
    }

    json << "}}";
    return json.str();
}

// ---------------------------------------------------------------------------
// Player entity creation
// ---------------------------------------------------------------------------

std::string GameSession::createPlayerEntity(const std::string& player_id,
                                            const std::string& character_name) {
    uint32_t id_num = next_entity_id_++;
    std::string entity_id = "player_" + std::to_string(id_num);

    auto* entity = world_->createEntity(entity_id);
    if (!entity) return entity_id;

    // Position – spawn near origin with spacing per player
    auto pos = std::make_unique<components::Position>();
    pos->x = static_cast<float>(id_num) * PLAYER_SPAWN_SPACING_X;
    pos->y = 0.0f;
    pos->z = static_cast<float>(id_num) * PLAYER_SPAWN_SPACING_Z;
    entity->addComponent(std::move(pos));

    // Velocity
    auto vel = std::make_unique<components::Velocity>();
    vel->max_speed = 300.0f;
    entity->addComponent(std::move(vel));

    // Health – Rifter-class frigate stats
    auto hp = std::make_unique<components::Health>();
    hp->shield_hp  = hp->shield_max  = 450.0f;
    hp->armor_hp   = hp->armor_max   = 350.0f;
    hp->hull_hp    = hp->hull_max    = 300.0f;
    hp->shield_recharge_rate = 3.5f;
    entity->addComponent(std::move(hp));

    // Ship info
    auto ship = std::make_unique<components::Ship>();
    ship->ship_name  = "Rifter";
    ship->ship_class = "Frigate";
    ship->ship_type  = "Frigate";
    ship->race       = "Minmatar";
    ship->cpu_max    = 125.0f;
    ship->powergrid_max = 37.0f;
    entity->addComponent(std::move(ship));

    // Player tag
    auto player = std::make_unique<components::Player>();
    player->player_id      = player_id;
    player->character_name = character_name;
    entity->addComponent(std::move(player));

    // Faction
    auto faction = std::make_unique<components::Faction>();
    faction->faction_name = "Minmatar";
    entity->addComponent(std::move(faction));

    // Capacitor
    auto cap = std::make_unique<components::Capacitor>();
    cap->capacitor_max = 250.0f;
    cap->capacitor     = 250.0f;
    cap->recharge_rate = 3.0f;
    entity->addComponent(std::move(cap));

    return entity_id;
}

// ---------------------------------------------------------------------------
// NPC spawning
// ---------------------------------------------------------------------------

void GameSession::spawnInitialNPCs() {
    spawnNPC("npc_serpentis_1", "Serpentis Spy",       "Catalyst",  "Serpentis",
             1000.0f,  0.0f, -500.0f);
    spawnNPC("npc_guristas_1", "Guristas Scout",      "Merlin",    "Guristas",
             -800.0f,  0.0f,  600.0f);
    spawnNPC("npc_blood_1",    "Blood Raider Seeker", "Punisher",  "Blood Raiders",
             500.0f,   0.0f,  1200.0f);
}

void GameSession::spawnNPC(const std::string& id, const std::string& name,
                           const std::string& ship_name,
                           const std::string& faction_name,
                           float x, float y, float z) {
    auto* entity = world_->createEntity(id);
    if (!entity) return;

    auto pos = std::make_unique<components::Position>();
    pos->x = x;  pos->y = y;  pos->z = z;
    entity->addComponent(std::move(pos));

    auto vel = std::make_unique<components::Velocity>();
    vel->max_speed = 250.0f;
    entity->addComponent(std::move(vel));

    auto hp = std::make_unique<components::Health>();
    hp->shield_hp = hp->shield_max = 300.0f;
    hp->armor_hp  = hp->armor_max  = 250.0f;
    hp->hull_hp   = hp->hull_max   = 200.0f;
    entity->addComponent(std::move(hp));

    auto ship = std::make_unique<components::Ship>();
    ship->ship_name  = ship_name;
    ship->ship_class = "Frigate";
    ship->ship_type  = "Frigate";
    entity->addComponent(std::move(ship));

    auto fac = std::make_unique<components::Faction>();
    fac->faction_name = faction_name;
    entity->addComponent(std::move(fac));

    auto ai = std::make_unique<components::AI>();
    ai->behavior = components::AI::Behavior::Aggressive;
    ai->state    = components::AI::State::Idle;
    ai->awareness_range = NPC_AWARENESS_RANGE;
    entity->addComponent(std::move(ai));

    auto weapon = std::make_unique<components::Weapon>();
    weapon->damage       = 12.0f;
    weapon->optimal_range = 5000.0f;
    weapon->rate_of_fire  = 4.0f;
    entity->addComponent(std::move(weapon));

    std::cout << "[GameSession] Spawned NPC: " << name
              << " (" << faction_name << " " << ship_name << ")" << std::endl;
}

// ---------------------------------------------------------------------------
// Lightweight JSON helpers (no external library required)
// ---------------------------------------------------------------------------

std::string GameSession::extractJsonString(const std::string& json,
                                           const std::string& key) {
    std::string search = "\"" + key + "\"";
    size_t pos = json.find(search);
    if (pos == std::string::npos) return "";

    // Skip past key and colon
    pos = json.find(':', pos + search.size());
    if (pos == std::string::npos) return "";

    // Skip whitespace
    pos = json.find('\"', pos + 1);
    if (pos == std::string::npos) return "";

    size_t end = json.find('\"', pos + 1);
    if (end == std::string::npos) return "";

    return json.substr(pos + 1, end - pos - 1);
}

float GameSession::extractJsonFloat(const std::string& json,
                                    const std::string& key,
                                    float fallback) {
    size_t pos = json.find(key);
    if (pos == std::string::npos) return fallback;

    pos += key.size();
    // Skip whitespace
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) ++pos;

    try {
        size_t end = pos;
        while (end < json.size() &&
               (json[end] == '-' || json[end] == '.' ||
                (json[end] >= '0' && json[end] <= '9') ||
                json[end] == 'e' || json[end] == 'E' || json[end] == '+')) {
            ++end;
        }
        return std::stof(json.substr(pos, end - pos));
    } catch (...) {
        return fallback;
    }
}

} // namespace eve
