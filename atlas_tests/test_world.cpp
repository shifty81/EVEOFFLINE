#include "../engine/world/CubeSphereLayout.h"
#include "../engine/world/VoxelGridLayout.h"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace atlas::world;

void test_cube_sphere_projection() {
    WorldPos pos = CubeSphereLayout::CubeToSphere(POS_Z, 0.0, 0.0, 100.0);

    double len = std::sqrt(pos.x * pos.x + pos.y * pos.y + pos.z * pos.z);
    assert(std::abs(len - 100.0) < 0.01);

    std::cout << "[PASS] test_cube_sphere_projection" << std::endl;
}

void test_cube_sphere_chunk_roundtrip() {
    CubeSphereLayout layout;
    layout.radius = 1000.0;

    WorldPos pos = {0, 0, 1000.0};
    ChunkCoord chunk = layout.WorldToChunk(pos, 2);
    WorldPos back = layout.ChunkToWorld(chunk);

    // Should be roughly on the same face
    assert(back.z > 0);

    std::cout << "[PASS] test_cube_sphere_chunk_roundtrip" << std::endl;
}

void test_cube_sphere_neighbors() {
    CubeSphereLayout layout;
    ChunkCoord chunk = {2, 2, POS_Z, 3};

    std::vector<ChunkCoord> neighbors;
    layout.GetNeighbors(chunk, neighbors);

    assert(neighbors.size() == 4);
    std::cout << "[PASS] test_cube_sphere_neighbors" << std::endl;
}

void test_cube_sphere_lod() {
    CubeSphereLayout layout;
    assert(layout.MaxLOD() == 10);
    assert(layout.IsValidLOD(0));
    assert(layout.IsValidLOD(10));
    assert(!layout.IsValidLOD(11));
    assert(!layout.IsValidLOD(-1));

    std::cout << "[PASS] test_cube_sphere_lod" << std::endl;
}

void test_voxel_chunk_roundtrip() {
    VoxelGridLayout layout;
    layout.chunkSize = 16;

    WorldPos pos = {35.0, 10.0, -20.0};
    ChunkCoord chunk = layout.WorldToChunk(pos, 0);
    WorldPos back = layout.ChunkToWorld(chunk);

    assert(std::abs(back.x - (chunk.x * 16 + 8)) < 0.01);
    assert(std::abs(back.y - (chunk.y * 16 + 8)) < 0.01);
    assert(std::abs(back.z - (chunk.z * 16 + 8)) < 0.01);

    std::cout << "[PASS] test_voxel_chunk_roundtrip" << std::endl;
}

void test_voxel_neighbors() {
    VoxelGridLayout layout;
    ChunkCoord chunk = {0, 0, 0, 0};

    std::vector<ChunkCoord> neighbors;
    layout.GetNeighbors(chunk, neighbors);

    assert(neighbors.size() == 6);
    std::cout << "[PASS] test_voxel_neighbors" << std::endl;
}
