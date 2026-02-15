#pragma once
#include <vector>
#include <cstdint>
#include <cmath>

namespace atlas::world {

struct ChunkID {
    uint64_t value = 0;
};

struct WorldPos {
    double x = 0.0, y = 0.0, z = 0.0;
};

struct ChunkCoord {
    int x = 0, y = 0, z = 0;
    int lod = 0;
};

class WorldLayout {
public:
    virtual ~WorldLayout() = default;

    // Coordinate mapping
    virtual ChunkCoord WorldToChunk(const WorldPos& pos, int lod) const = 0;
    virtual WorldPos ChunkToWorld(const ChunkCoord& chunk) const = 0;

    // Chunk identity
    virtual ChunkID MakeChunkID(const ChunkCoord& chunk) const = 0;

    // Topology
    virtual void GetNeighbors(
        const ChunkCoord& chunk,
        std::vector<ChunkCoord>& outNeighbors
    ) const = 0;

    // LOD
    virtual int MaxLOD() const = 0;
    virtual bool IsValidLOD(int lod) const = 0;

    // Streaming hints
    virtual float ChunkWorldSize(int lod) const = 0;
};

}
