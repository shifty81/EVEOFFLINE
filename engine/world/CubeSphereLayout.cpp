#include "CubeSphereLayout.h"

namespace atlas::world {

WorldPos CubeSphereLayout::CubeToSphere(CubeFace face, double u, double v, double radius) {
    double x = 0, y = 0, z = 0;

    switch (face) {
        case POS_X: x =  1; y = v; z = -u; break;
        case NEG_X: x = -1; y = v; z =  u; break;
        case POS_Y: x = u; y =  1; z = -v; break;
        case NEG_Y: x = u; y = -1; z =  v; break;
        case POS_Z: x = u; y = v; z =  1; break;
        case NEG_Z: x = -u; y = v; z = -1; break;
    }

    double len = std::sqrt(x * x + y * y + z * z);
    if (len > 0.0) {
        return {radius * x / len, radius * y / len, radius * z / len};
    }
    return {0, 0, 0};
}

ChunkCoord CubeSphereLayout::WorldToChunk(const WorldPos& pos, int lod) const {
    int gridSize = 1 << lod;
    double invRadius = 1.0 / radius;
    double nx = pos.x * invRadius;
    double ny = pos.y * invRadius;
    double nz = pos.z * invRadius;

    double ax = std::abs(nx), ay = std::abs(ny), az = std::abs(nz);

    int face = 0;
    double u = 0, v = 0;

    if (ax >= ay && ax >= az) {
        face = nx > 0 ? POS_X : NEG_X;
        u = (nx > 0 ? -nz : nz) / ax;
        v = ny / ax;
    } else if (ay >= ax && ay >= az) {
        face = ny > 0 ? POS_Y : NEG_Y;
        u = nx / ay;
        v = (ny > 0 ? -nz : nz) / ay;
    } else {
        face = nz > 0 ? POS_Z : NEG_Z;
        u = (nz > 0 ? nx : -nx) / az;
        v = ny / az;
    }

    int cx = static_cast<int>((u * 0.5 + 0.5) * gridSize);
    int cy = static_cast<int>((v * 0.5 + 0.5) * gridSize);
    cx = std::max(0, std::min(cx, gridSize - 1));
    cy = std::max(0, std::min(cy, gridSize - 1));

    return {cx, cy, face, lod};
}

WorldPos CubeSphereLayout::ChunkToWorld(const ChunkCoord& chunk) const {
    int gridSize = 1 << chunk.lod;
    double u = (static_cast<double>(chunk.x) + 0.5) / gridSize * 2.0 - 1.0;
    double v = (static_cast<double>(chunk.y) + 0.5) / gridSize * 2.0 - 1.0;
    return CubeToSphere(static_cast<CubeFace>(chunk.z), u, v, radius);
}

ChunkID CubeSphereLayout::MakeChunkID(const ChunkCoord& chunk) const {
    uint64_t id = 0;
    id |= static_cast<uint64_t>(chunk.z & 0x7) << 48;
    id |= static_cast<uint64_t>(chunk.lod & 0xF) << 44;
    id |= static_cast<uint64_t>(chunk.x & 0xFFFFF) << 22;
    id |= static_cast<uint64_t>(chunk.y & 0x3FFFFF);
    return {id};
}

void CubeSphereLayout::GetNeighbors(const ChunkCoord& chunk, std::vector<ChunkCoord>& out) const {
    out.clear();
    int gridSize = 1 << chunk.lod;
    const int dx[] = {-1, 1, 0, 0};
    const int dy[] = {0, 0, -1, 1};

    for (int i = 0; i < 4; ++i) {
        int nx = chunk.x + dx[i];
        int ny = chunk.y + dy[i];
        if (nx >= 0 && nx < gridSize && ny >= 0 && ny < gridSize) {
            out.push_back({nx, ny, chunk.z, chunk.lod});
        }
    }
}

float CubeSphereLayout::ChunkWorldSize(int lod) const {
    int gridSize = 1 << lod;
    return static_cast<float>(2.0 * radius / gridSize);
}

}
