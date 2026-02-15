#include "../engine/assets/AssetRegistry.h"
#include "../engine/assets/AssetBinary.h"
#include "../engine/graphvm/GraphVM.h"
#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>

using namespace atlas::asset;
using namespace atlas::vm;

void test_asset_binary_roundtrip() {
    Bytecode original;
    original.constants = {10, 20, 30};
    original.instructions = {
        {OpCode::LOAD_CONST, 0, 0, 0},
        {OpCode::LOAD_CONST, 1, 0, 0},
        {OpCode::ADD, 0, 0, 0},
        {OpCode::STORE_VAR, 0, 0, 0},
        {OpCode::END, 0, 0, 0}
    };

    std::string path = "/tmp/atlas_test_asset.atlasb";
    assert(AssetBinary::WriteGraph(path, original));

    Bytecode loaded;
    assert(AssetBinary::ReadGraph(path, loaded));

    assert(loaded.constants.size() == original.constants.size());
    assert(loaded.instructions.size() == original.instructions.size());

    for (size_t i = 0; i < original.constants.size(); ++i) {
        assert(loaded.constants[i] == original.constants[i]);
    }

    for (size_t i = 0; i < original.instructions.size(); ++i) {
        assert(loaded.instructions[i].opcode == original.instructions[i].opcode);
        assert(loaded.instructions[i].a == original.instructions[i].a);
    }

    std::filesystem::remove(path);
    std::cout << "[PASS] test_asset_binary_roundtrip" << std::endl;
}

void test_asset_registry_scan() {
    std::string testDir = "/tmp/atlas_test_assets";
    std::filesystem::create_directories(testDir);

    std::ofstream(testDir + "/test1.atlas").close();
    std::ofstream(testDir + "/test2.atlas").close();

    AssetRegistry registry;
    registry.Scan(testDir);

    assert(registry.Count() == 2);
    assert(registry.Get("test1") != nullptr);
    assert(registry.Get("test2") != nullptr);
    assert(registry.Get("nonexistent") == nullptr);

    std::filesystem::remove_all(testDir);
    std::cout << "[PASS] test_asset_registry_scan" << std::endl;
}
