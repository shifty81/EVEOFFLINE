#pragma once
#include <cstdint>

namespace atlas::asset {

constexpr uint32_t ASSET_MAGIC = 0x41544C53; // "ATLS"

enum class AssetType : uint16_t {
    Graph,
    World,
    Planet,
    Galaxy,
    Mesh,
    Material,
    Mechanic,
    VoxelSchema,
    GameType,
    EditorProfile,
};

struct AssetHeader {
    uint32_t magic = ASSET_MAGIC;
    uint16_t version = 1;
    AssetType type = AssetType::Graph;
    uint32_t size = 0;
    uint64_t hash = 0;
};

}
