/**
 * Test program for network system
 * Tests connection to Python server and message exchange
 */

#include "network/network_manager.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

void printEntity(const json& entity) {
    int id = entity.value("id", -1);
    if (entity.contains("pos")) {
        auto pos = entity["pos"];
        float x = pos.value("x", 0.0f);
        float y = pos.value("y", 0.0f);
        float z = pos.value("z", 0.0f);
        std::cout << "  Entity " << id << " at (" << x << ", " << y << ", " << z << ")" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::cout << "=== EVE OFFLINE Network Test ===" << std::endl;
    std::cout << std::endl;

    // Parse arguments
    std::string host = "localhost";
    int port = 8765;
    std::string characterName = "TestPilot";

    if (argc > 1) host = argv[1];
    if (argc > 2) port = std::atoi(argv[2]);
    if (argc > 3) characterName = argv[3];

    std::cout << "Server: " << host << ":" << port << std::endl;
    std::cout << "Character: " << characterName << std::endl;
    std::cout << std::endl;

    // Create network manager
    eve::NetworkManager network;

    // Register message handlers
    int entityCount = 0;
    int updateCount = 0;

    network.registerHandler("state_update", [&](const std::string& dataJson) {
        try {
            auto data = json::parse(dataJson);
            if (data.contains("entities")) {
                entityCount = data["entities"].size();
                updateCount++;
                
                // Print first update
                if (updateCount == 1) {
                    std::cout << "First state update received:" << std::endl;
                    for (const auto& entity : data["entities"]) {
                        printEntity(entity);
                    }
                    std::cout << std::endl;
                }
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse STATE_UPDATE: " << e.what() << std::endl;
        }
    });

    network.registerHandler("spawn_entity", [](const std::string& dataJson) {
        try {
            auto data = json::parse(dataJson);
            int entityId = data.value("entity_id", -1);
            std::cout << "Entity spawned: " << entityId << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse spawn_entity: " << e.what() << std::endl;
        }
    });

    network.registerHandler("destroy_entity", [](const std::string& dataJson) {
        try {
            auto data = json::parse(dataJson);
            int entityId = data.value("entity_id", -1);
            std::cout << "Entity destroyed: " << entityId << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse destroy_entity: " << e.what() << std::endl;
        }
    });

    network.registerHandler("chat", [](const std::string& dataJson) {
        try {
            auto data = json::parse(dataJson);
            std::string msg = data.value("message", "");
            std::cout << "Chat: " << msg << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to parse CHAT: " << e.what() << std::endl;
        }
    });

    // Connect to server
    std::cout << "Connecting to server..." << std::endl;
    if (!network.connect(host, port, "test_player_1", characterName)) {
        std::cerr << "Failed to connect!" << std::endl;
        return 1;
    }

    std::cout << "Connected! Running for 10 seconds..." << std::endl;
    std::cout << std::endl;

    // Test movement
    std::cout << "Sending movement command..." << std::endl;
    network.sendMove(10.0f, 0.0f, 0.0f);

    // Run for 10 seconds
    auto startTime = std::chrono::steady_clock::now();
    while (true) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();
        
        if (elapsed >= 10) break;

        // Update network
        network.update();

        // Print status every 2 seconds
        if (elapsed > 0 && elapsed % 2 == 0) {
            static int lastPrintTime = -1;
            if (elapsed != lastPrintTime) {
                std::cout << "Status: " << network.getConnectionState() 
                          << " | Updates: " << updateCount 
                          << " | Entities: " << entityCount << std::endl;
                lastPrintTime = elapsed;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }

    std::cout << std::endl;
    std::cout << "Test complete!" << std::endl;
    std::cout << "Total updates received: " << updateCount << std::endl;
    std::cout << "Final entity count: " << entityCount << std::endl;

    // Disconnect
    network.disconnect();

    std::cout << std::endl;
    std::cout << "=== Test Finished ===" << std::endl;

    return 0;
}
