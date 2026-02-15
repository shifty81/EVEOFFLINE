#include "VoxelGridLayout.h"

namespace atlas::world {

ChunkCoord VoxelGridLayout::WorldToChunk(const WorldPos& pos, int /*lod*/) const {
    int cx = static_cast<int>(std::floor(pos.x / chunkSize));
    int cy = static_cast<int>(std::floor(pos.y / chunkSize));
    int cz = static_cast<int>(std::floor(pos.z / chunkSize));
    return {cx, cy, cz, 0};
}

WorldPos VoxelGridLayout::ChunkToWorld(const ChunkCoord& chunk) const {
    return {
        static_cast<double>(chunk.x * chunkSize + chunkSize / 2),
        static_cast<double>(chunk.y * chunkSize + chunkSize / 2),
        static_cast<double>(chunk.z * chunkSize + chunkSize / 2)
    };
}

ChunkID VoxelGridLayout::MakeChunkID(const ChunkCoord& chunk) const {
    uint64_t id = 0;
    id |= static_cast<uint64_t>(static_cast<uint16_t>(chunk.x)) << 32;
    id |= static_cast<uint64_t>(static_cast<uint16_t>(chunk.y)) << 16;
    id |= static_cast<uint64_t>(static_cast<uint16_t>(chunk.z));
    return {id};
}

void VoxelGridLayout::GetNeighbors(const ChunkCoord& chunk, std::vector<ChunkCoord>& out) const {
    out.clear();
    const int dx[] = {-1, 1, 0, 0, 0, 0};
    const int dy[] = {0, 0, -1, 1, 0, 0};
    const int dz[] = {0, 0, 0, 0, -1, 1};

    for (int i = 0; i < 6; ++i) {
        out.push_back({chunk.x + dx[i], chunk.y + dy[i], chunk.z + dz[i], 0});
    }
}

float VoxelGridLayout::ChunkWorldSize(int /*lod*/) const {
    return static_cast<float>(chunkSize);
}

}
