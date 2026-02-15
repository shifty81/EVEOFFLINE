#include "AssetBinary.h"
#include <fstream>

namespace atlas::asset {

bool AssetBinary::WriteGraph(const std::string& path, const vm::Bytecode& prog) {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;

    AssetHeader hdr;
    hdr.type = AssetType::Graph;
    hdr.size = static_cast<uint32_t>(
        sizeof(uint32_t) +
        prog.constants.size() * sizeof(vm::Value) +
        prog.instructions.size() * sizeof(vm::Instruction)
    );

    out.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));

    uint32_t constCount = static_cast<uint32_t>(prog.constants.size());
    out.write(reinterpret_cast<const char*>(&constCount), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(prog.constants.data()),
              static_cast<std::streamsize>(prog.constants.size() * sizeof(vm::Value)));

    uint32_t codeCount = static_cast<uint32_t>(prog.instructions.size());
    out.write(reinterpret_cast<const char*>(&codeCount), sizeof(uint32_t));
    out.write(reinterpret_cast<const char*>(prog.instructions.data()),
              static_cast<std::streamsize>(prog.instructions.size() * sizeof(vm::Instruction)));

    return out.good();
}

bool AssetBinary::ReadGraph(const std::string& path, vm::Bytecode& prog) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return false;

    AssetHeader hdr;
    in.read(reinterpret_cast<char*>(&hdr), sizeof(hdr));

    if (hdr.magic != ASSET_MAGIC) return false;

    uint32_t constCount;
    in.read(reinterpret_cast<char*>(&constCount), sizeof(uint32_t));

    prog.constants.resize(constCount);
    in.read(reinterpret_cast<char*>(prog.constants.data()),
            static_cast<std::streamsize>(constCount * sizeof(vm::Value)));

    uint32_t codeCount;
    in.read(reinterpret_cast<char*>(&codeCount), sizeof(uint32_t));

    prog.instructions.resize(codeCount);
    in.read(reinterpret_cast<char*>(prog.instructions.data()),
            static_cast<std::streamsize>(codeCount * sizeof(vm::Instruction)));

    return in.good();
}

}
