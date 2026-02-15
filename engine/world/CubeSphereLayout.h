#pragma once
#include "WorldLayout.h"

namespace atlas::world {

enum CubeFace {
    POS_X, NEG_X,
    POS_Y, NEG_Y,
    POS_Z, NEG_Z
};

struct CubeSphereCoord {
    CubeFace face;
    int x, y;
    int lod;
};

class CubeSphereLayout : public WorldLayout {
public:
    double radius = 6371000.0;

    ChunkCoord WorldToChunk(const WorldPos& pos, int lod) const override;
    WorldPos ChunkToWorld(const ChunkCoord& chunk) const override;
    ChunkID MakeChunkID(const ChunkCoord& chunk) const override;
    void GetNeighbors(const ChunkCoord& chunk, std::vector<ChunkCoord>& outNeighbors) const override;

    int MaxLOD() const override { return 10; }
    bool IsValidLOD(int lod) const override { return lod >= 0 && lod <= 10; }
    float ChunkWorldSize(int lod) const override;

    static WorldPos CubeToSphere(CubeFace face, double u, double v, double radius);
};

}
