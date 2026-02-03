#include "auth/whitelist.h"
#include <fstream>
#include <sstream>
#include <algorithm>

namespace eve {
namespace auth {

Whitelist::Whitelist() {
}

bool Whitelist::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    whitelisted_names_.clear();
    whitelisted_ids_.clear();
    
    std::string line;
    bool in_names_section = false;
    bool in_ids_section = false;
    
    while (std::getline(file, line)) {
        // Remove whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line.find("//") == 0) {
            continue;
        }
        
        // Check for section headers
        if (line.find("\"steam_names\"") != std::string::npos) {
            in_names_section = true;
            in_ids_section = false;
            continue;
        } else if (line.find("\"steam_ids\"") != std::string::npos) {
            in_ids_section = true;
            in_names_section = false;
            continue;
        }
        
        // Parse entries
        if (in_names_section) {
            size_t quote_start = line.find('"');
            size_t quote_end = line.rfind('"');
            if (quote_start != std::string::npos && quote_end != std::string::npos && quote_start < quote_end) {
                std::string name = line.substr(quote_start + 1, quote_end - quote_start - 1);
                if (!name.empty()) {
                    whitelisted_names_.insert(name);
                }
            }
        } else if (in_ids_section) {
            // Try to parse as number
            try {
                size_t pos = line.find_first_of("0123456789");
                if (pos != std::string::npos) {
                    std::string num_str;
                    for (size_t i = pos; i < line.length() && std::isdigit(line[i]); i++) {
                        num_str += line[i];
                    }
                    if (!num_str.empty()) {
                        uint64_t steam_id = std::stoull(num_str);
                        whitelisted_ids_.insert(steam_id);
                    }
                }
            } catch (...) {
                // Skip invalid entries
            }
        }
    }
    
    file.close();
    return true;
}

bool Whitelist::saveToFile(const std::string& filepath) const {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    file << "{" << std::endl;
    file << "  \"steam_names\": [" << std::endl;
    
    bool first = true;
    for (const auto& name : whitelisted_names_) {
        if (!first) file << "," << std::endl;
        file << "    \"" << name << "\"";
        first = false;
    }
    file << std::endl << "  ]," << std::endl;
    
    file << "  \"steam_ids\": [" << std::endl;
    first = true;
    for (const auto& id : whitelisted_ids_) {
        if (!first) file << "," << std::endl;
        file << "    " << id;
        first = false;
    }
    file << std::endl << "  ]" << std::endl;
    file << "}" << std::endl;
    
    file.close();
    return true;
}

void Whitelist::addSteamName(const std::string& steam_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    whitelisted_names_.insert(steam_name);
}

void Whitelist::removeSteamName(const std::string& steam_name) {
    std::lock_guard<std::mutex> lock(mutex_);
    whitelisted_names_.erase(steam_name);
}

bool Whitelist::isSteamNameWhitelisted(const std::string& steam_name) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return whitelisted_names_.find(steam_name) != whitelisted_names_.end();
}

void Whitelist::addSteamId(uint64_t steam_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    whitelisted_ids_.insert(steam_id);
}

void Whitelist::removeSteamId(uint64_t steam_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    whitelisted_ids_.erase(steam_id);
}

bool Whitelist::isSteamIdWhitelisted(uint64_t steam_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return whitelisted_ids_.find(steam_id) != whitelisted_ids_.end();
}

bool Whitelist::hasAccess(const std::string& steam_name, uint64_t steam_id) const {
    return isSteamNameWhitelisted(steam_name) || isSteamIdWhitelisted(steam_id);
}

std::vector<std::string> Whitelist::getSteamNames() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::vector<std::string>(whitelisted_names_.begin(), whitelisted_names_.end());
}

std::vector<uint64_t> Whitelist::getSteamIds() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::vector<uint64_t>(whitelisted_ids_.begin(), whitelisted_ids_.end());
}

void Whitelist::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    whitelisted_names_.clear();
    whitelisted_ids_.clear();
}

bool Whitelist::isEmpty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return whitelisted_names_.empty() && whitelisted_ids_.empty();
}

} // namespace auth
} // namespace eve
