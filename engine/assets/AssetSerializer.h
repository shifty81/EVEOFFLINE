#pragma once
#include <string>
#include <fstream>
#include <vector>

namespace atlas::asset {

template<typename T>
bool WriteBinary(const std::string& path, const T& data) {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;
    out.write(reinterpret_cast<const char*>(&data), sizeof(T));
    return out.good();
}

template<typename T>
bool ReadBinary(const std::string& path, T& outData) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return false;
    in.read(reinterpret_cast<char*>(&outData), sizeof(T));
    return in.good();
}

}
