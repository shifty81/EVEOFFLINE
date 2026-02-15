#pragma once
#include "WorldLayout.h"

namespace atlas::world {

class VoxelGridLayout : public WorldLayout {
public:
    int chunkSize = 16;

    ChunkCoord WorldToChunk(const WorldPos& pos, int lod) const override;
    WorldPos ChunkToWorld(const ChunkCoord& chunk) const override;
    ChunkID MakeChunkID(const ChunkCoord& chunk) const override;
    void GetNeighbors(const ChunkCoord& chunk, std::vector<ChunkCoord>& outNeighbors) const override;

    int MaxLOD() const override { return 0; }
    bool IsValidLOD(int lod) const override { return lod == 0; }
    float ChunkWorldSize(int lod) const override;
};

}
