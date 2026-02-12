#include "config/server_config.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace atlas {

bool ServerConfig::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    // Simple JSON parsing (in production, use a library like nlohmann/json)
    // This is a minimal parser for configuration files
    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line.find("//") == 0) {
            continue;
        }
        
        // Parse key-value pairs
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) {
            continue;
        }
        
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 1);
        
        // Remove quotes, whitespace, and commas
        key.erase(0, key.find_first_not_of(" \t\""));
        key.erase(key.find_last_not_of(" \t\"") + 1);
        value.erase(0, value.find_first_not_of(" \t\""));
        value.erase(value.find_last_not_of(" \t\",") + 1);
        
        // Parse common settings
        if (key == "host") host = value;
        else if (key == "port") port = static_cast<uint16_t>(std::stoi(value));
        else if (key == "max_connections") max_connections = std::stoi(value);
        else if (key == "server_name") server_name = value;
        else if (key == "server_description") server_description = value;
        else if (key == "persistent_world") persistent_world = (value == "true");
        else if (key == "auto_save") auto_save = (value == "true");
        else if (key == "save_interval_seconds") save_interval_seconds = std::stoi(value);
        else if (key == "use_whitelist") use_whitelist = (value == "true");
        else if (key == "public_server") public_server = (value == "true");
        else if (key == "password") password = value;
        else if (key == "use_steam") use_steam = (value == "true");
        else if (key == "steam_app_id") steam_app_id = static_cast<uint32_t>(std::stoul(value));
        else if (key == "steam_authentication") steam_authentication = (value == "true");
        else if (key == "steam_server_browser") steam_server_browser = (value == "true");
        else if (key == "tick_rate") tick_rate = std::stof(value);
        else if (key == "max_entities") max_entities = std::stoi(value);
        else if (key == "data_path") data_path = value;
        else if (key == "save_path") save_path = value;
        else if (key == "log_path") log_path = value;
    }
    
    file.close();
    return true;
}

bool ServerConfig::saveToFile(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    file << "{" << std::endl;
    file << "  \"host\": \"" << host << "\"," << std::endl;
    file << "  \"port\": " << port << "," << std::endl;
    file << "  \"max_connections\": " << max_connections << "," << std::endl;
    file << "  \"server_name\": \"" << server_name << "\"," << std::endl;
    file << "  \"server_description\": \"" << server_description << "\"," << std::endl;
    file << "  \"persistent_world\": " << (persistent_world ? "true" : "false") << "," << std::endl;
    file << "  \"auto_save\": " << (auto_save ? "true" : "false") << "," << std::endl;
    file << "  \"save_interval_seconds\": " << save_interval_seconds << "," << std::endl;
    file << "  \"use_whitelist\": " << (use_whitelist ? "true" : "false") << "," << std::endl;
    file << "  \"public_server\": " << (public_server ? "true" : "false") << "," << std::endl;
    file << "  \"password\": \"" << password << "\"," << std::endl;
    file << "  \"use_steam\": " << (use_steam ? "true" : "false") << "," << std::endl;
    file << "  \"steam_app_id\": " << steam_app_id << "," << std::endl;
    file << "  \"steam_authentication\": " << (steam_authentication ? "true" : "false") << "," << std::endl;
    file << "  \"steam_server_browser\": " << (steam_server_browser ? "true" : "false") << "," << std::endl;
    file << "  \"tick_rate\": " << tick_rate << "," << std::endl;
    file << "  \"max_entities\": " << max_entities << "," << std::endl;
    file << "  \"data_path\": \"" << data_path << "\"," << std::endl;
    file << "  \"save_path\": \"" << save_path << "\"," << std::endl;
    file << "  \"log_path\": \"" << log_path << "\"" << std::endl;
    file << "}" << std::endl;
    
    file.close();
    return true;
}

} // namespace atlas
