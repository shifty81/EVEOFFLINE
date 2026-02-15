#pragma once
#include "../graphvm/GraphVM.h"
#include "AssetFormat.h"
#include <string>

namespace atlas::asset {

class AssetBinary {
public:
    static bool WriteGraph(const std::string& path, const vm::Bytecode& prog);
    static bool ReadGraph(const std::string& path, vm::Bytecode& prog);
};

}
