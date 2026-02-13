#include "rendering/procedural_ship_generator.h"
#include "rendering/model.h"
#include "rendering/mesh.h"

#include <tiny_obj_loader.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace atlas {

// ─────────────────────────────────────────────────────────────────────
// OBJSeedMesh helpers
// ─────────────────────────────────────────────────────────────────────

void OBJSeedMesh::computeBounds() {
    if (positions.empty()) {
        bbMin = bbMax = glm::vec3(0.0f);
        return;
    }
    bbMin = bbMax = positions[0];
    for (const auto& p : positions) {
        bbMin = glm::min(bbMin, p);
        bbMax = glm::max(bbMax, p);
    }
}

void OBJSeedMesh::centreAtOrigin() {
    computeBounds();
    glm::vec3 centre = (bbMin + bbMax) * 0.5f;
    for (auto& p : positions) {
        p -= centre;
    }
    bbMin -= centre;
    bbMax -= centre;
}

void OBJSeedMesh::normalizeScale(float targetLength) {
    computeBounds();
    float maxExtent = std::max({width(), height(), length()});
    if (maxExtent < 1e-6f) return;
    float s = targetLength / maxExtent;
    for (auto& p : positions) {
        p *= s;
    }
    bbMin *= s;
    bbMax *= s;
}

// ─────────────────────────────────────────────────────────────────────
// ProceduralShipGenerator
// ─────────────────────────────────────────────────────────────────────

ProceduralShipGenerator::ProceduralShipGenerator() {
    // Set default reference asset paths — archives live in testing/,
    // extracted content goes to assets/reference_models/
    m_assetConfig.objArchivePath     = "testing/99-intergalactic_spaceship-obj.rar";
    m_assetConfig.textureArchivePath = "testing/24-textures.zip";
    m_assetConfig.extractedObjDir    = "assets/reference_models";
    m_assetConfig.extractedTextureDir = "assets/reference_models/textures";
}

ProceduralShipGenerator::~ProceduralShipGenerator() = default;

void ProceduralShipGenerator::setReferenceAssets(const ReferenceAssetConfig& config) {
    m_assetConfig = config;
}

// ─────────────────────────────────────────────────────────────────────
// OBJ parsing
// ─────────────────────────────────────────────────────────────────────

OBJSeedMesh ProceduralShipGenerator::parseOBJ(const std::string& path) {
    OBJSeedMesh seed;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::string baseDir;
    auto lastSlash = path.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        baseDir = path.substr(0, lastSlash + 1);
    }

    bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                               path.c_str(), baseDir.c_str());
    if (!ok) {
        std::cerr << "[ProceduralShipGenerator] Failed to load OBJ: " << path
                  << "\n  error: " << err << std::endl;
        return seed;
    }
    if (!warn.empty()) {
        std::cout << "[ProceduralShipGenerator] OBJ warning: " << warn << std::endl;
    }

    // Reserve approximate space
    size_t totalVerts = 0;
    for (const auto& shape : shapes) {
        totalVerts += shape.mesh.indices.size();
    }
    seed.positions.reserve(totalVerts);
    seed.normals.reserve(totalVerts);
    seed.uvs.reserve(totalVerts);
    seed.indices.reserve(totalVerts);

    // Flatten into per-vertex data (OBJ allows separate pos/normal/uv indices)
    unsigned int vertexOffset = 0;
    for (const auto& shape : shapes) {
        size_t indexOffset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            int fv = shape.mesh.num_face_vertices[f];
            for (int v = 0; v < fv; ++v) {
                tinyobj::index_t idx = shape.mesh.indices[indexOffset + v];

                // Position (required)
                glm::vec3 pos(
                    attrib.vertices[3 * idx.vertex_index + 0],
                    attrib.vertices[3 * idx.vertex_index + 1],
                    attrib.vertices[3 * idx.vertex_index + 2]
                );
                seed.positions.push_back(pos);

                // Normal (optional)
                if (idx.normal_index >= 0 &&
                    3 * idx.normal_index + 2 < static_cast<int>(attrib.normals.size())) {
                    seed.normals.push_back(glm::vec3(
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                    ));
                } else {
                    seed.normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
                }

                // Texture coordinates (optional)
                if (idx.texcoord_index >= 0 &&
                    2 * idx.texcoord_index + 1 < static_cast<int>(attrib.texcoords.size())) {
                    seed.uvs.push_back(glm::vec2(
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        attrib.texcoords[2 * idx.texcoord_index + 1]
                    ));
                } else {
                    seed.uvs.push_back(glm::vec2(0.0f));
                }

                seed.indices.push_back(vertexOffset++);
            }
            indexOffset += fv;
        }
    }

    seed.computeBounds();

    std::cout << "[ProceduralShipGenerator] Parsed OBJ: " << path
              << " (" << seed.positions.size() << " verts, "
              << seed.indices.size() / 3 << " tris)" << std::endl;

    return seed;
}

// ─────────────────────────────────────────────────────────────────────
// Mount point detection
// ─────────────────────────────────────────────────────────────────────

std::vector<MountPoint> ProceduralShipGenerator::detectMountPoints(const OBJSeedMesh& seed) {
    std::vector<MountPoint> mounts;
    if (seed.empty()) return mounts;

    float len = seed.length();
    float wid = seed.width();
    float hgt = seed.height();
    float avgSize = (len + wid + hgt) / 3.0f;
    float mountRadius = avgSize * 0.05f;

    glm::vec3 centre = (seed.bbMin + seed.bbMax) * 0.5f;

    // Engine mounts: rear of ship (lowest Z values)
    float rearZ = seed.bbMin.z + len * 0.05f;
    {
        MountPoint m;
        m.position = glm::vec3(centre.x - wid * 0.15f, centre.y, rearZ);
        m.normal   = glm::vec3(0.0f, 0.0f, -1.0f);
        m.radius   = mountRadius;
        m.category = "engine";
        mounts.push_back(m);
    }
    {
        MountPoint m;
        m.position = glm::vec3(centre.x + wid * 0.15f, centre.y, rearZ);
        m.normal   = glm::vec3(0.0f, 0.0f, -1.0f);
        m.radius   = mountRadius;
        m.category = "engine";
        mounts.push_back(m);
    }

    // Weapon mounts: upper hull, forward and midship
    float topY = seed.bbMax.y - hgt * 0.05f;
    {
        MountPoint m;
        m.position = glm::vec3(centre.x, topY, centre.z + len * 0.2f);
        m.normal   = glm::vec3(0.0f, 1.0f, 0.0f);
        m.radius   = mountRadius;
        m.category = "weapon";
        mounts.push_back(m);
    }
    {
        MountPoint m;
        m.position = glm::vec3(centre.x, topY, centre.z - len * 0.1f);
        m.normal   = glm::vec3(0.0f, 1.0f, 0.0f);
        m.radius   = mountRadius;
        m.category = "weapon";
        mounts.push_back(m);
    }

    // Wing / lateral mounts
    {
        MountPoint m;
        m.position = glm::vec3(seed.bbMax.x, centre.y, centre.z);
        m.normal   = glm::vec3(1.0f, 0.0f, 0.0f);
        m.radius   = mountRadius;
        m.category = "wing";
        mounts.push_back(m);
    }
    {
        MountPoint m;
        m.position = glm::vec3(seed.bbMin.x, centre.y, centre.z);
        m.normal   = glm::vec3(-1.0f, 0.0f, 0.0f);
        m.radius   = mountRadius;
        m.category = "wing";
        mounts.push_back(m);
    }

    // Antenna mount: top-forward
    {
        MountPoint m;
        m.position = glm::vec3(centre.x, topY, seed.bbMax.z - len * 0.1f);
        m.normal   = glm::vec3(0.0f, 1.0f, 0.2f);
        m.radius   = mountRadius * 0.5f;
        m.category = "antenna";
        mounts.push_back(m);
    }

    return mounts;
}

// ─────────────────────────────────────────────────────────────────────
// Hull scaling
// ─────────────────────────────────────────────────────────────────────

void ProceduralShipGenerator::applyHullScaling(OBJSeedMesh& mesh,
                                               float lengthScale,
                                               float widthScale,
                                               float heightScale) {
    glm::vec3 centre = (mesh.bbMin + mesh.bbMax) * 0.5f;
    for (auto& p : mesh.positions) {
        p.x = centre.x + (p.x - centre.x) * widthScale;
        p.y = centre.y + (p.y - centre.y) * heightScale;
        p.z = centre.z + (p.z - centre.z) * lengthScale;
    }
    mesh.computeBounds();
}

// ─────────────────────────────────────────────────────────────────────
// Extrusion detail (greebles)
// ─────────────────────────────────────────────────────────────────────

void ProceduralShipGenerator::applyExtrusions(OBJSeedMesh& mesh, int count,
                                              float depth, std::mt19937& rng) {
    if (mesh.indices.size() < 3 || count <= 0) return;

    mesh.computeBounds();
    float diagLen = glm::length(mesh.bbMax - mesh.bbMin);
    float maxDisp = diagLen * depth;

    size_t triCount = mesh.indices.size() / 3;
    std::uniform_int_distribution<size_t> triDist(0, triCount - 1);
    std::uniform_real_distribution<float> depthDist(maxDisp * 0.3f, maxDisp);

    for (int i = 0; i < count; ++i) {
        size_t triIdx = triDist(rng);
        unsigned int i0 = mesh.indices[triIdx * 3 + 0];
        unsigned int i1 = mesh.indices[triIdx * 3 + 1];
        unsigned int i2 = mesh.indices[triIdx * 3 + 2];

        if (i0 >= mesh.positions.size() || i1 >= mesh.positions.size() ||
            i2 >= mesh.positions.size()) {
            continue;
        }

        glm::vec3 v0 = mesh.positions[i0];
        glm::vec3 v1 = mesh.positions[i1];
        glm::vec3 v2 = mesh.positions[i2];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

        if (std::isnan(faceNormal.x)) continue;

        // Compute face centroid and offset new vertices along face normal
        glm::vec3 centroid = (v0 + v1 + v2) / 3.0f;
        float d = depthDist(rng);

        // Create 3 new vertices (extruded copy of the triangle)
        glm::vec3 n0 = (i0 < mesh.normals.size()) ? mesh.normals[i0] : faceNormal;
        glm::vec3 n1 = (i1 < mesh.normals.size()) ? mesh.normals[i1] : faceNormal;
        glm::vec3 n2 = (i2 < mesh.normals.size()) ? mesh.normals[i2] : faceNormal;

        glm::vec2 uv0 = (i0 < mesh.uvs.size()) ? mesh.uvs[i0] : glm::vec2(0.0f);
        glm::vec2 uv1 = (i1 < mesh.uvs.size()) ? mesh.uvs[i1] : glm::vec2(0.0f);
        glm::vec2 uv2 = (i2 < mesh.uvs.size()) ? mesh.uvs[i2] : glm::vec2(0.0f);

        // Scale inward slightly (90% of original face) to create a ledge
        float insetScale = 0.9f;
        glm::vec3 ev0 = centroid + (v0 - centroid) * insetScale + faceNormal * d;
        glm::vec3 ev1 = centroid + (v1 - centroid) * insetScale + faceNormal * d;
        glm::vec3 ev2 = centroid + (v2 - centroid) * insetScale + faceNormal * d;

        unsigned int base = static_cast<unsigned int>(mesh.positions.size());

        // Add extruded cap
        mesh.positions.push_back(ev0);
        mesh.positions.push_back(ev1);
        mesh.positions.push_back(ev2);
        mesh.normals.push_back(faceNormal);
        mesh.normals.push_back(faceNormal);
        mesh.normals.push_back(faceNormal);
        mesh.uvs.push_back(uv0);
        mesh.uvs.push_back(uv1);
        mesh.uvs.push_back(uv2);

        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);

        // Add side walls (3 quads = 6 tris connecting original face to extruded face)
        unsigned int origVerts[3] = { i0, i1, i2 };
        unsigned int extVerts[3]  = { base, base + 1, base + 2 };
        for (int e = 0; e < 3; ++e) {
            int next = (e + 1) % 3;
            // Side quad as two triangles
            mesh.indices.push_back(origVerts[e]);
            mesh.indices.push_back(origVerts[next]);
            mesh.indices.push_back(extVerts[next]);

            mesh.indices.push_back(origVerts[e]);
            mesh.indices.push_back(extVerts[next]);
            mesh.indices.push_back(extVerts[e]);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────
// Noise displacement
// ─────────────────────────────────────────────────────────────────────

float ProceduralShipGenerator::noise3D(float x, float y, float z) {
    // Simple hash-based noise (deterministic, no external dependency)
    int ix = static_cast<int>(std::floor(x));
    int iy = static_cast<int>(std::floor(y));
    int iz = static_cast<int>(std::floor(z));
    float fx = x - std::floor(x);
    float fy = y - std::floor(y);
    float fz = z - std::floor(z);

    // Smoothstep interpolation
    fx = fx * fx * (3.0f - 2.0f * fx);
    fy = fy * fy * (3.0f - 2.0f * fy);
    fz = fz * fz * (3.0f - 2.0f * fz);

    auto hash = [](int a, int b, int c) -> float {
        int h = a * 374761393 + b * 668265263 + c * 1274126177;
        h = (h ^ (h >> 13)) * 1274126177;
        return static_cast<float>(h & 0x7FFFFFFF) / static_cast<float>(0x7FFFFFFF);
    };

    // Trilinear interpolation of 8 corner hash values
    float c000 = hash(ix,     iy,     iz);
    float c100 = hash(ix + 1, iy,     iz);
    float c010 = hash(ix,     iy + 1, iz);
    float c110 = hash(ix + 1, iy + 1, iz);
    float c001 = hash(ix,     iy,     iz + 1);
    float c101 = hash(ix + 1, iy,     iz + 1);
    float c011 = hash(ix,     iy + 1, iz + 1);
    float c111 = hash(ix + 1, iy + 1, iz + 1);

    float x00 = c000 + fx * (c100 - c000);
    float x10 = c010 + fx * (c110 - c010);
    float x01 = c001 + fx * (c101 - c001);
    float x11 = c011 + fx * (c111 - c011);

    float xy0 = x00 + fy * (x10 - x00);
    float xy1 = x01 + fy * (x11 - x01);

    return xy0 + fz * (xy1 - xy0);  // Returns [0, 1]
}

void ProceduralShipGenerator::applyNoiseDisplacement(OBJSeedMesh& mesh,
                                                     float amplitude,
                                                     float frequency,
                                                     std::mt19937& /*rng*/) {
    if (amplitude <= 0.0f) return;

    mesh.computeBounds();
    float diagLen = glm::length(mesh.bbMax - mesh.bbMin);
    float maxDisp = diagLen * amplitude;

    for (size_t i = 0; i < mesh.positions.size(); ++i) {
        glm::vec3 p = mesh.positions[i] * frequency;
        float noiseVal = noise3D(p.x, p.y, p.z) * 2.0f - 1.0f;  // Map to [-1, 1]

        glm::vec3 n = (i < mesh.normals.size())
                          ? mesh.normals[i]
                          : glm::vec3(0.0f, 1.0f, 0.0f);
        if (glm::length(n) < 1e-6f) n = glm::vec3(0.0f, 1.0f, 0.0f);

        mesh.positions[i] += glm::normalize(n) * (noiseVal * maxDisp);
    }
    mesh.computeBounds();
}

// ─────────────────────────────────────────────────────────────────────
// Symmetry enforcement
// ─────────────────────────────────────────────────────────────────────

void ProceduralShipGenerator::enforceSymmetry(OBJSeedMesh& mesh) {
    if (mesh.positions.empty()) return;

    // Build a spatial map: for each vertex, find its mirror partner
    float tolerance = glm::length(mesh.bbMax - mesh.bbMin) * 0.001f;

    for (size_t i = 0; i < mesh.positions.size(); ++i) {
        glm::vec3& pi = mesh.positions[i];
        if (std::abs(pi.x) < tolerance) {
            pi.x = 0.0f;  // Snap centre-line vertices
            continue;
        }
        // Find mirror partner
        glm::vec3 mirror(-pi.x, pi.y, pi.z);
        for (size_t j = i + 1; j < mesh.positions.size(); ++j) {
            if (glm::length(mesh.positions[j] - mirror) < tolerance) {
                // Average Y and Z, mirror X
                float avgY = (pi.y + mesh.positions[j].y) * 0.5f;
                float avgZ = (pi.z + mesh.positions[j].z) * 0.5f;
                pi.y = avgY;
                pi.z = avgZ;
                mesh.positions[j].x = -pi.x;
                mesh.positions[j].y = avgY;
                mesh.positions[j].z = avgZ;
                break;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────
// Recompute normals
// ─────────────────────────────────────────────────────────────────────

void ProceduralShipGenerator::recomputeNormals(OBJSeedMesh& mesh) {
    mesh.normals.assign(mesh.positions.size(), glm::vec3(0.0f));

    // Accumulate face normals at each vertex
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        unsigned int i0 = mesh.indices[i + 0];
        unsigned int i1 = mesh.indices[i + 1];
        unsigned int i2 = mesh.indices[i + 2];
        if (i0 >= mesh.positions.size() || i1 >= mesh.positions.size() ||
            i2 >= mesh.positions.size()) {
            continue;
        }

        glm::vec3 e1 = mesh.positions[i1] - mesh.positions[i0];
        glm::vec3 e2 = mesh.positions[i2] - mesh.positions[i0];
        glm::vec3 fn = glm::cross(e1, e2);

        mesh.normals[i0] += fn;
        mesh.normals[i1] += fn;
        mesh.normals[i2] += fn;
    }

    for (auto& n : mesh.normals) {
        float len = glm::length(n);
        if (len > 1e-6f) {
            n /= len;
        } else {
            n = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────
// Module generation
// ─────────────────────────────────────────────────────────────────────

OBJSeedMesh ProceduralShipGenerator::generateEngineModule(const MountPoint& mount,
                                                          float size,
                                                          std::mt19937& rng) {
    OBJSeedMesh mod;
    // Generate a simple cylinder / cone engine nozzle
    int segments = 12;
    float radius = size * 0.4f;
    float length = size * 1.2f;

    std::uniform_real_distribution<float> jitter(0.9f, 1.1f);
    float rJitter = jitter(rng);
    radius *= rJitter;

    // Front ring and rear ring
    for (int ring = 0; ring < 2; ++ring) {
        float z = (ring == 0) ? 0.0f : -length;
        float r = (ring == 0) ? radius : radius * 0.7f;  // Slightly tapered rear
        for (int s = 0; s < segments; ++s) {
            float angle = static_cast<float>(s) / segments * 2.0f * static_cast<float>(M_PI);
            float x = std::cos(angle) * r;
            float y = std::sin(angle) * r;

            mod.positions.push_back(mount.position + glm::vec3(x, y, z));
            mod.normals.push_back(glm::normalize(glm::vec3(x, y, 0.0f)));
            mod.uvs.push_back(glm::vec2(
                static_cast<float>(s) / segments,
                static_cast<float>(ring)
            ));
        }
    }

    // Stitch rings into quads
    for (int s = 0; s < segments; ++s) {
        int next = (s + 1) % segments;
        unsigned int a0 = static_cast<unsigned int>(s);
        unsigned int a1 = static_cast<unsigned int>(next);
        unsigned int b0 = static_cast<unsigned int>(s + segments);
        unsigned int b1 = static_cast<unsigned int>(next + segments);

        mod.indices.push_back(a0); mod.indices.push_back(a1); mod.indices.push_back(b1);
        mod.indices.push_back(a0); mod.indices.push_back(b1); mod.indices.push_back(b0);
    }

    mod.computeBounds();
    return mod;
}

OBJSeedMesh ProceduralShipGenerator::generateWeaponModule(const MountPoint& mount,
                                                          float size,
                                                          std::mt19937& rng) {
    OBJSeedMesh mod;
    // Turret base (flat cylinder) + barrel (elongated box)
    float baseR = size * 0.3f;
    float baseH = size * 0.15f;
    float barrelLen = size * 0.8f;
    float barrelW = size * 0.08f;

    std::uniform_real_distribution<float> jitter(0.85f, 1.15f);
    float bJitter = jitter(rng);
    barrelLen *= bJitter;

    // Base disc (simplified as octagon top + bottom)
    int sides = 8;
    for (int ring = 0; ring < 2; ++ring) {
        float y = mount.position.y + (ring == 0 ? 0.0f : baseH);
        for (int s = 0; s < sides; ++s) {
            float angle = static_cast<float>(s) / sides * 2.0f * static_cast<float>(M_PI);
            float x = mount.position.x + std::cos(angle) * baseR;
            float z = mount.position.z + std::sin(angle) * baseR;

            mod.positions.push_back(glm::vec3(x, y, z));
            mod.normals.push_back(glm::vec3(0.0f, (ring == 0) ? -1.0f : 1.0f, 0.0f));
            mod.uvs.push_back(glm::vec2(0.0f));
        }
    }

    // Stitch base cylinder
    for (int s = 0; s < sides; ++s) {
        int next = (s + 1) % sides;
        unsigned int a0 = static_cast<unsigned int>(s);
        unsigned int a1 = static_cast<unsigned int>(next);
        unsigned int b0 = static_cast<unsigned int>(s + sides);
        unsigned int b1 = static_cast<unsigned int>(next + sides);

        mod.indices.push_back(a0); mod.indices.push_back(b1); mod.indices.push_back(a1);
        mod.indices.push_back(a0); mod.indices.push_back(b0); mod.indices.push_back(b1);
    }

    // Barrel (simple box extending forward along Z from base top)
    unsigned int barrelBase = static_cast<unsigned int>(mod.positions.size());
    float by = mount.position.y + baseH;
    float bx = mount.position.x;
    float bz = mount.position.z;

    // 8 vertices of a box
    glm::vec3 barrelVerts[8] = {
        {bx - barrelW, by,            bz},
        {bx + barrelW, by,            bz},
        {bx + barrelW, by + barrelW * 2, bz},
        {bx - barrelW, by + barrelW * 2, bz},
        {bx - barrelW, by,            bz + barrelLen},
        {bx + barrelW, by,            bz + barrelLen},
        {bx + barrelW, by + barrelW * 2, bz + barrelLen},
        {bx - barrelW, by + barrelW * 2, bz + barrelLen}
    };

    for (int v = 0; v < 8; ++v) {
        mod.positions.push_back(barrelVerts[v]);
        mod.normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        mod.uvs.push_back(glm::vec2(0.0f));
    }

    // 6 faces × 2 tris = 12 tris for a box
    unsigned int boxIdx[36] = {
        0,1,2, 0,2,3,   // bottom
        4,6,5, 4,7,6,   // top
        0,4,5, 0,5,1,   // front
        2,6,7, 2,7,3,   // back
        0,3,7, 0,7,4,   // left
        1,5,6, 1,6,2    // right
    };
    for (int t = 0; t < 36; ++t) {
        mod.indices.push_back(barrelBase + boxIdx[t]);
    }

    mod.computeBounds();
    return mod;
}

OBJSeedMesh ProceduralShipGenerator::generateAntennaModule(const MountPoint& mount,
                                                           float size,
                                                           std::mt19937& rng) {
    OBJSeedMesh mod;
    // Simple thin vertical rod with a small crossbar
    float rodH = size * 0.6f;
    float rodR = size * 0.02f;
    float crossLen = size * 0.2f;

    std::uniform_real_distribution<float> jitter(0.8f, 1.2f);
    rodH *= jitter(rng);

    // Rod: 4 vertices (line-like thin box)
    glm::vec3 base = mount.position;
    unsigned int b = 0;

    // Thin box for rod
    glm::vec3 rodVerts[8] = {
        base + glm::vec3(-rodR, 0,    -rodR),
        base + glm::vec3( rodR, 0,    -rodR),
        base + glm::vec3( rodR, 0,     rodR),
        base + glm::vec3(-rodR, 0,     rodR),
        base + glm::vec3(-rodR, rodH, -rodR),
        base + glm::vec3( rodR, rodH, -rodR),
        base + glm::vec3( rodR, rodH,  rodR),
        base + glm::vec3(-rodR, rodH,  rodR),
    };

    for (int v = 0; v < 8; ++v) {
        mod.positions.push_back(rodVerts[v]);
        mod.normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        mod.uvs.push_back(glm::vec2(0.0f));
    }

    unsigned int boxIdx[36] = {
        0,1,2, 0,2,3,
        4,6,5, 4,7,6,
        0,4,5, 0,5,1,
        2,6,7, 2,7,3,
        0,3,7, 0,7,4,
        1,5,6, 1,6,2
    };
    for (int t = 0; t < 36; ++t) {
        mod.indices.push_back(b + boxIdx[t]);
    }

    // Crossbar at top
    unsigned int cb = static_cast<unsigned int>(mod.positions.size());
    glm::vec3 top = base + glm::vec3(0.0f, rodH * 0.9f, 0.0f);
    glm::vec3 crossVerts[8] = {
        top + glm::vec3(-crossLen, -rodR, -rodR),
        top + glm::vec3( crossLen, -rodR, -rodR),
        top + glm::vec3( crossLen, -rodR,  rodR),
        top + glm::vec3(-crossLen, -rodR,  rodR),
        top + glm::vec3(-crossLen,  rodR, -rodR),
        top + glm::vec3( crossLen,  rodR, -rodR),
        top + glm::vec3( crossLen,  rodR,  rodR),
        top + glm::vec3(-crossLen,  rodR,  rodR),
    };
    for (int v = 0; v < 8; ++v) {
        mod.positions.push_back(crossVerts[v]);
        mod.normals.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
        mod.uvs.push_back(glm::vec2(0.0f));
    }
    for (int t = 0; t < 36; ++t) {
        mod.indices.push_back(cb + boxIdx[t]);
    }

    mod.computeBounds();
    return mod;
}

// ─────────────────────────────────────────────────────────────────────
// Mesh merging
// ─────────────────────────────────────────────────────────────────────

void ProceduralShipGenerator::mergeInto(OBJSeedMesh& target, const OBJSeedMesh& module) {
    unsigned int offset = static_cast<unsigned int>(target.positions.size());

    target.positions.insert(target.positions.end(),
                            module.positions.begin(), module.positions.end());
    target.normals.insert(target.normals.end(),
                          module.normals.begin(), module.normals.end());
    target.uvs.insert(target.uvs.end(),
                      module.uvs.begin(), module.uvs.end());

    for (unsigned int idx : module.indices) {
        target.indices.push_back(idx + offset);
    }
}

// ─────────────────────────────────────────────────────────────────────
// Full pipeline
// ─────────────────────────────────────────────────────────────────────

std::unique_ptr<Model> ProceduralShipGenerator::generate(
        const OBJSeedMesh& seedMesh,
        const ProceduralShipParams& params) const {
    if (seedMesh.empty()) {
        std::cerr << "[ProceduralShipGenerator] Cannot generate from empty seed mesh"
                  << std::endl;
        return nullptr;
    }

    // Work on a copy so the original seed is not modified
    OBJSeedMesh mesh = seedMesh;

    // Deterministic RNG
    unsigned int actualSeed = (params.seed != 0)
                                  ? params.seed
                                  : static_cast<unsigned int>(
                                        std::hash<std::string>{}("eve_default"));
    std::mt19937 rng(actualSeed);

    // 1. Centre and normalize
    mesh.centreAtOrigin();
    mesh.normalizeScale(100.0f);  // Normalize to ~100 units

    // 2. Apply hull scaling
    applyHullScaling(mesh, params.lengthScale, params.widthScale, params.heightScale);

    // 3. Detect mount points (before extrusions alter geometry)
    auto mounts = detectMountPoints(mesh);

    // 4. Apply extrusions for surface detail
    if (params.extrusionCount > 0) {
        applyExtrusions(mesh, params.extrusionCount, params.extrusionDepth, rng);
    }

    // 5. Apply noise displacement
    if (params.noiseAmplitude > 0.0f) {
        applyNoiseDisplacement(mesh, params.noiseAmplitude, params.noiseFrequency, rng);
    }

    // 6. Enforce symmetry
    if (params.enforceSymmetry) {
        enforceSymmetry(mesh);
    }

    // 7. Attach modules
    float moduleSize = glm::length(mesh.bbMax - mesh.bbMin) * 0.08f;

    // Engines
    int enginesAttached = 0;
    for (const auto& mp : mounts) {
        if (mp.category == "engine" && enginesAttached < params.engineCount) {
            auto engineMod = generateEngineModule(mp, moduleSize, rng);
            mergeInto(mesh, engineMod);
            ++enginesAttached;
        }
    }

    // Weapons
    int weaponsAttached = 0;
    for (const auto& mp : mounts) {
        if (mp.category == "weapon" && weaponsAttached < params.weaponCount) {
            auto weaponMod = generateWeaponModule(mp, moduleSize * 0.7f, rng);
            mergeInto(mesh, weaponMod);
            ++weaponsAttached;
        }
    }

    // Antennae
    int antennaeAttached = 0;
    for (const auto& mp : mounts) {
        if (mp.category == "antenna" && antennaeAttached < params.antennaCount) {
            auto antMod = generateAntennaModule(mp, moduleSize * 0.5f, rng);
            mergeInto(mesh, antMod);
            ++antennaeAttached;
        }
    }

    // 8. Recompute normals after all modifications
    recomputeNormals(mesh);

    // 9. Convert to atlas::Model
    auto model = std::make_unique<Model>();

    // Build Vertex array
    std::vector<Vertex> vertices;
    vertices.reserve(mesh.positions.size());

    glm::vec3 color = (glm::length(params.primaryColor) > 0.01f)
                          ? params.primaryColor
                          : glm::vec3(0.6f, 0.65f, 0.7f);  // Default steel grey

    for (size_t i = 0; i < mesh.positions.size(); ++i) {
        Vertex vert;
        vert.position  = mesh.positions[i];
        vert.normal    = (i < mesh.normals.size()) ? mesh.normals[i] : glm::vec3(0,1,0);
        vert.texCoords = (i < mesh.uvs.size())     ? mesh.uvs[i]     : glm::vec2(0);
        vert.color     = color;
        vertices.push_back(vert);
    }

    auto meshPtr = std::make_unique<Mesh>(vertices, mesh.indices);
    model->addMesh(std::move(meshPtr));

    std::cout << "[ProceduralShipGenerator] Generated ship: "
              << mesh.positions.size() << " verts, "
              << mesh.indices.size() / 3 << " tris (seed=" << actualSeed << ")"
              << std::endl;

    return model;
}

std::unique_ptr<Model> ProceduralShipGenerator::generateFromFile(
        const std::string& objPath,
        const ProceduralShipParams& params) const {
    OBJSeedMesh seed = parseOBJ(objPath);
    if (seed.empty()) return nullptr;
    return generate(seed, params);
}

// ─────────────────────────────────────────────────────────────────────
// Seed OBJ search
// ─────────────────────────────────────────────────────────────────────

std::string ProceduralShipGenerator::findSeedOBJ(const std::string& faction,
                                                 const std::string& shipClass) const {
    // Search directories for OBJ files from reference assets.
    // Try multiple paths to handle different working directory contexts
    // (running from repo root, build dir, or IDE).
    std::vector<std::string> searchDirs = {
        m_assetConfig.extractedObjDir,
        "assets/reference_models",
        "../assets/reference_models",
        "cpp_client/assets/reference_models",
        "../../cpp_client/assets/reference_models",
        "models/ships",
        "../models/ships",
        "data/ships/obj_models"
    };

    for (const auto& dir : searchDirs) {
        // Try faction-specific match first
        std::vector<std::string> candidates = {
            dir + "/" + faction + "_" + shipClass + ".obj",
            dir + "/" + shipClass + ".obj"
        };
        for (const auto& candidate : candidates) {
            std::ifstream test(candidate);
            if (test.good()) {
                return candidate;
            }
        }
    }

    // Fall back to known reference models shipped with the project.
    // The Intergalactic Spaceship OBJ serves as a high-quality seed for
    // smaller ship classes (frigates through cruisers).  The Vulcan Dkyr
    // Class OBJ is a heavier capital-style mesh suited for battleships,
    // carriers, dreadnoughts, and titans.
    std::vector<std::string> prefixes = {
        m_assetConfig.extractedObjDir,
        "assets/reference_models",
        "../assets/reference_models",
        "cpp_client/assets/reference_models",
        "../../cpp_client/assets/reference_models"
    };

    // Determine which reference model to use based on ship class.
    // Capital and large ships use the high-detail Vulcan Dkyr Class mesh;
    // smaller ships use the Intergalactic Spaceship.
    bool isCapital = (shipClass == "battleship" || shipClass == "Battleship" ||
                      shipClass == "carrier" || shipClass == "Carrier" ||
                      shipClass == "dreadnought" || shipClass == "Dreadnought" ||
                      shipClass == "titan" || shipClass == "Titan" ||
                      shipClass == "Marauder" || shipClass == "marauder");

    for (const auto& prefix : prefixes) {
        std::string path;
        if (isCapital) {
            path = prefix + "/Vulcan Dkyr Class/VulcanDKyrClass.obj";
        } else {
            path = prefix + "/Intergalactic_Spaceship-(Wavefront).obj";
        }
        std::ifstream test(path);
        if (test.good()) {
            return path;
        }
    }

    // Last resort: try the other reference model if the preferred one isn't found
    for (const auto& prefix : prefixes) {
        std::string path;
        if (isCapital) {
            // Capitals: fall back to Intergalactic Spaceship if no Vulcan
            path = prefix + "/Intergalactic_Spaceship-(Wavefront).obj";
        } else {
            // Small ships: fall back to Vulcan if no Intergalactic
            path = prefix + "/Vulcan Dkyr Class/VulcanDKyrClass.obj";
        }
        std::ifstream test(path);
        if (test.good()) {
            return path;
        }
    }

    return "";
}

// ─────────────────────────────────────────────────────────────────────
// OBJ export
// ─────────────────────────────────────────────────────────────────────

bool ProceduralShipGenerator::exportOBJ(const OBJSeedMesh& mesh,
                                        const std::string& path) {
    std::ofstream out(path);
    if (!out.is_open()) {
        std::cerr << "[ProceduralShipGenerator] Cannot open for writing: " << path
                  << std::endl;
        return false;
    }

    out << "# Generated by EVE OFFLINE ProceduralShipGenerator\n";
    out << "# Vertices: " << mesh.positions.size() << "\n";
    out << "# Faces: " << mesh.indices.size() / 3 << "\n\n";

    // Vertices
    for (const auto& p : mesh.positions) {
        out << "v " << p.x << " " << p.y << " " << p.z << "\n";
    }

    // Normals
    for (const auto& n : mesh.normals) {
        out << "vn " << n.x << " " << n.y << " " << n.z << "\n";
    }

    // Texture coordinates
    for (const auto& uv : mesh.uvs) {
        out << "vt " << uv.x << " " << uv.y << "\n";
    }

    // Faces (OBJ uses 1-based indices)
    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        unsigned int a = mesh.indices[i + 0] + 1;
        unsigned int b = mesh.indices[i + 1] + 1;
        unsigned int c = mesh.indices[i + 2] + 1;
        out << "f " << a << "/" << a << "/" << a
            << " " << b << "/" << b << "/" << b
            << " " << c << "/" << c << "/" << c << "\n";
    }

    out.close();
    std::cout << "[ProceduralShipGenerator] Exported OBJ: " << path << std::endl;
    return true;
}

// ─────────────────────────────────────────────────────────────────────
// Texture helpers
// ─────────────────────────────────────────────────────────────────────

std::vector<std::string> ProceduralShipGenerator::listAvailableTextures() const {
    std::vector<std::string> textures;
    // In a full implementation, this would iterate the extracted texture directory.
    // For now, return known texture patterns from the 24-textures reference pack.
    std::vector<std::string> knownPatterns = {
        "diffuse", "normal", "roughness", "metallic",
        "glow", "emissive", "ao", "paint"
    };
    for (const auto& pattern : knownPatterns) {
        std::string path = m_assetConfig.extractedTextureDir + "/" + pattern + ".dds";
        std::ifstream test(path);
        if (test.good()) {
            textures.push_back(path);
        }
    }
    return textures;
}

std::string ProceduralShipGenerator::findTexture(const std::string& keyword) const {
    std::string dir = m_assetConfig.extractedTextureDir;
    // Try direct match
    std::vector<std::string> extensions = {".dds", ".png", ".jpg", ".tga", ".bmp"};
    for (const auto& ext : extensions) {
        std::string path = dir + "/" + keyword + ext;
        std::ifstream test(path);
        if (test.good()) {
            return path;
        }
    }
    return "";
}

} // namespace atlas
