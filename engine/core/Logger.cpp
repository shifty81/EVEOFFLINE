#include "Logger.h"
#include <iostream>

namespace atlas {

void Logger::Init() {
    Info("Logger initialized");
}

void Logger::Info(const std::string& msg) {
    std::cout << "[INFO] " << msg << std::endl;
}

void Logger::Warn(const std::string& msg) {
    std::cout << "[WARN] " << msg << std::endl;
}

void Logger::Error(const std::string& msg) {
    std::cerr << "[ERROR] " << msg << std::endl;
}

}
