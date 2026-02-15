#include "ConsolePanel.h"
#include <sstream>

namespace atlas::editor {

void ConsolePanel::Draw() {
    // Stub: In a real implementation, this would render via Atlas UI
    // Display history, input field, execute button
}

void ConsolePanel::AddLine(const std::string& line) {
    m_history.push_back(line);
}

void ConsolePanel::Execute(const std::string& command) {
    m_history.push_back("> " + command);

    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "spawn_entity") {
        auto id = m_world.CreateEntity();
        m_history.push_back("Created entity " + std::to_string(id));
    } else if (cmd == "ecs.dump") {
        auto entities = m_world.GetEntities();
        m_history.push_back("Entities: " + std::to_string(entities.size()));
        for (auto e : entities) {
            auto types = m_world.GetComponentTypes(e);
            m_history.push_back("  Entity " + std::to_string(e) + " (" + std::to_string(types.size()) + " components)");
        }
    } else if (cmd == "set") {
        std::string key;
        iss >> key;
        if (key == "tickrate") {
            uint32_t rate = 0;
            iss >> rate;
            if (rate > 0) {
                m_scheduler.SetTickRate(rate);
                m_history.push_back("Tick rate set to " + std::to_string(rate));
            } else {
                m_history.push_back("Invalid tick rate");
            }
        } else {
            m_history.push_back("Unknown setting: " + key);
        }
    } else if (cmd == "net.mode") {
        auto mode = m_net.Mode();
        std::string modeStr;
        switch (mode) {
            case net::NetMode::Standalone: modeStr = "Standalone"; break;
            case net::NetMode::Client: modeStr = "Client"; break;
            case net::NetMode::Server: modeStr = "Server"; break;
            case net::NetMode::P2P_Host: modeStr = "P2P_Host"; break;
            case net::NetMode::P2P_Peer: modeStr = "P2P_Peer"; break;
        }
        m_history.push_back("Net mode: " + modeStr);
    } else if (cmd == "help") {
        m_history.push_back("Commands: spawn_entity, ecs.dump, set tickrate <N>, net.mode, help");
    } else {
        m_history.push_back("Unknown command: " + cmd);
    }
}

}
