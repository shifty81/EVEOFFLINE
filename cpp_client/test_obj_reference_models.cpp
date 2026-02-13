/**
 * Test program for OBJ reference model integration.
 *
 * Validates that:
 * 1. Reference OBJ files are found by findSeedOBJ()
 * 2. Large OBJ files are parseable via parseOBJ()
 * 3. Modular OBJ parts are parseable
 * 4. Mount points are detected from seed meshes
 * 5. Seed meshes can be centred, normalized, and scaled
 *
 * Run from the repository root so paths resolve correctly.
 * Does NOT require OpenGL — tests only the geometry pipeline.
 */

#include "rendering/procedural_ship_generator.h"
#include <iostream>
#include <string>
#include <fstream>
#include <cmath>

using namespace atlas;

// ─── Test helpers ──────────────────────────────────────────────────────

int testsRun = 0;
int testsPassed = 0;

void assertTrue(bool condition, const std::string& testName) {
    testsRun++;
    if (condition) {
        testsPassed++;
        std::cout << "  \xe2\x9c\x93 " << testName << std::endl;
    } else {
        std::cout << "  \xe2\x9c\x97 " << testName << " FAILED" << std::endl;
    }
}

bool fileExists(const std::string& path) {
    std::ifstream f(path);
    return f.good();
}

// ─── findSeedOBJ tests ────────────────────────────────────────────────

void testFindSeedOBJ() {
    std::cout << "\n=== findSeedOBJ Discovery ===" << std::endl;

    ProceduralShipGenerator gen;
    ReferenceAssetConfig cfg;
    cfg.extractedObjDir = "cpp_client/assets/reference_models";
    gen.setReferenceAssets(cfg);

    // Small ships should find Intergalactic Spaceship
    std::string frigateSeed = gen.findSeedOBJ("Veyren", "frigate");
    assertTrue(!frigateSeed.empty(), "Frigate seed OBJ found");
    if (!frigateSeed.empty()) {
        assertTrue(frigateSeed.find("Intergalactic") != std::string::npos,
                   "Frigate maps to Intergalactic Spaceship");
    }

    std::string cruiserSeed = gen.findSeedOBJ("Solari", "cruiser");
    assertTrue(!cruiserSeed.empty(), "Cruiser seed OBJ found");

    // Capital ships should find Vulcan Dkyr Class
    std::string bsSeed = gen.findSeedOBJ("Keldari", "battleship");
    assertTrue(!bsSeed.empty(), "Battleship seed OBJ found");
    if (!bsSeed.empty()) {
        assertTrue(bsSeed.find("Vulcan") != std::string::npos,
                   "Battleship maps to Vulcan Dkyr Class");
    }

    std::string titanSeed = gen.findSeedOBJ("Aurelian", "titan");
    assertTrue(!titanSeed.empty(), "Titan seed OBJ found");

    // Test lowercase class names
    std::string carrierSeed = gen.findSeedOBJ("Veyren", "carrier");
    assertTrue(!carrierSeed.empty(), "Carrier seed OBJ found (lowercase)");

    std::string dreadSeed = gen.findSeedOBJ("Keldari", "dreadnought");
    assertTrue(!dreadSeed.empty(), "Dreadnought seed OBJ found (lowercase)");
}

// ─── parseOBJ tests ───────────────────────────────────────────────────

void testParseReferenceOBJs() {
    std::cout << "\n=== Parse Reference OBJ Files ===" << std::endl;

    // Test Intergalactic Spaceship
    std::string spaceship = "cpp_client/assets/reference_models/Intergalactic_Spaceship-(Wavefront).obj";
    if (fileExists(spaceship)) {
        OBJSeedMesh mesh = ProceduralShipGenerator::parseOBJ(spaceship);
        assertTrue(!mesh.empty(), "Intergalactic Spaceship parsed successfully");
        assertTrue(mesh.positions.size() > 20000, "Spaceship has >20K vertices (got " +
                   std::to_string(mesh.positions.size()) + ")");
        assertTrue(!mesh.indices.empty(), "Spaceship has indices");
        assertTrue(mesh.indices.size() % 3 == 0, "Spaceship index count is multiple of 3");

        // Compute bounds
        mesh.computeBounds();
        float length = mesh.length();
        float width  = mesh.width();
        float height = mesh.height();
        assertTrue(length > 0.0f && width > 0.0f && height > 0.0f,
                   "Spaceship has positive bounding box dimensions");
        std::cout << "    Spaceship: " << mesh.positions.size() << " verts, "
                  << mesh.indices.size() / 3 << " tris, bbox "
                  << width << "x" << height << "x" << length << std::endl;
    } else {
        std::cout << "  (skipped: Intergalactic Spaceship not extracted)" << std::endl;
    }

    // Test Vulcan Dkyr Class
    std::string vulcan = "cpp_client/assets/reference_models/Vulcan Dkyr Class/VulcanDKyrClass.obj";
    if (fileExists(vulcan)) {
        OBJSeedMesh mesh = ProceduralShipGenerator::parseOBJ(vulcan);
        assertTrue(!mesh.empty(), "Vulcan Dkyr Class parsed successfully");
        assertTrue(mesh.positions.size() > 50000, "Vulcan has >50K vertices (got " +
                   std::to_string(mesh.positions.size()) + ")");
        assertTrue(!mesh.indices.empty(), "Vulcan has indices");
        assertTrue(mesh.indices.size() % 3 == 0, "Vulcan index count is multiple of 3");

        mesh.computeBounds();
        std::cout << "    Vulcan: " << mesh.positions.size() << " verts, "
                  << mesh.indices.size() / 3 << " tris, bbox "
                  << mesh.width() << "x" << mesh.height() << "x" << mesh.length()
                  << std::endl;
    } else {
        std::cout << "  (skipped: Vulcan Dkyr Class not extracted)" << std::endl;
    }
}

void testParseModularOBJs() {
    std::cout << "\n=== Parse Modular OBJ Parts ===" << std::endl;

    struct ModuleInfo {
        std::string path;
        std::string name;
        int minVerts;
    };

    std::vector<ModuleInfo> modules = {
        {"cpp_client/assets/reference_models/modules/core_s.obj", "Small Core", 8},
        {"cpp_client/assets/reference_models/modules/engine_s.obj", "Small Engine", 8},
        {"cpp_client/assets/reference_models/modules/weapon_s.obj", "Small Weapon", 8},
        {"cpp_client/assets/reference_models/modules/wing_s.obj", "Small Wing", 4},
        {"cpp_client/assets/reference_models/modules/core_m.obj", "Medium Core", 8},
        {"cpp_client/assets/reference_models/modules/spine_m.obj", "Medium Spine", 8},
        {"cpp_client/assets/reference_models/modules/engine_block_m.obj", "Medium Engine", 8},
        {"cpp_client/assets/reference_models/modules/turret_m.obj", "Medium Turret", 8},
        {"cpp_client/assets/reference_models/modules/hangar_m.obj", "Medium Hangar", 8},
    };

    for (const auto& mod : modules) {
        if (fileExists(mod.path)) {
            OBJSeedMesh mesh = ProceduralShipGenerator::parseOBJ(mod.path);
            assertTrue(!mesh.empty(), mod.name + " parsed successfully");
            assertTrue(static_cast<int>(mesh.positions.size()) >= mod.minVerts,
                       mod.name + " has >= " + std::to_string(mod.minVerts) +
                       " verts (got " + std::to_string(mesh.positions.size()) + ")");
        } else {
            std::cout << "  (skipped: " << mod.name << " not found at " << mod.path << ")" << std::endl;
        }
    }
}

// ─── Mount point detection test ───────────────────────────────────────

void testMountPointDetection() {
    std::cout << "\n=== Mount Point Detection ===" << std::endl;

    // Use Intergalactic Spaceship as test target
    std::string spaceship = "cpp_client/assets/reference_models/Intergalactic_Spaceship-(Wavefront).obj";
    if (!fileExists(spaceship)) {
        std::cout << "  (skipped: reference model not extracted)" << std::endl;
        return;
    }

    OBJSeedMesh mesh = ProceduralShipGenerator::parseOBJ(spaceship);
    assertTrue(!mesh.empty(), "Loaded seed for mount detection");

    mesh.centreAtOrigin();
    mesh.normalizeScale(100.0f);

    auto mounts = ProceduralShipGenerator::detectMountPoints(mesh);
    assertTrue(!mounts.empty(), "Mount points detected (got " +
               std::to_string(mounts.size()) + ")");

    // Should find engine and weapon mounts
    int engines = 0, weapons = 0, antennae = 0;
    for (const auto& mp : mounts) {
        if (mp.category == "engine") engines++;
        else if (mp.category == "weapon") weapons++;
        else if (mp.category == "antenna") antennae++;
    }

    assertTrue(engines > 0, "Engine mount(s) detected (" + std::to_string(engines) + ")");
    assertTrue(weapons > 0, "Weapon mount(s) detected (" + std::to_string(weapons) + ")");
    std::cout << "    Mounts: " << engines << " engine, " << weapons << " weapon, "
              << antennae << " antenna" << std::endl;
}

// ─── Seed mesh processing pipeline test ───────────────────────────────

void testSeedMeshProcessing() {
    std::cout << "\n=== Seed Mesh Processing Pipeline ===" << std::endl;

    std::string spaceship = "cpp_client/assets/reference_models/Intergalactic_Spaceship-(Wavefront).obj";
    if (!fileExists(spaceship)) {
        std::cout << "  (skipped: reference model not extracted)" << std::endl;
        return;
    }

    OBJSeedMesh mesh = ProceduralShipGenerator::parseOBJ(spaceship);
    assertTrue(!mesh.empty(), "Loaded seed for processing test");

    // Centre and normalize
    mesh.centreAtOrigin();
    mesh.normalizeScale(100.0f);
    mesh.computeBounds();

    // After normalization, longest axis should be ~100
    float maxDim = std::max({mesh.length(), mesh.width(), mesh.height()});
    assertTrue(std::abs(maxDim - 100.0f) < 5.0f,
               "Normalized longest axis ~100 (got " + std::to_string(maxDim) + ")");

    // Centre should be near origin
    glm::vec3 centre = (mesh.bbMin + mesh.bbMax) * 0.5f;
    float centreDist = glm::length(centre);
    assertTrue(centreDist < 5.0f,
               "Centred near origin (dist=" + std::to_string(centreDist) + ")");

    // Apply hull scaling
    float preLength = mesh.length();
    ProceduralShipGenerator::applyHullScaling(mesh, 1.2f, 0.8f, 1.0f);
    mesh.computeBounds();
    assertTrue(mesh.length() > preLength * 1.1f,
               "Hull stretching increased length (pre=" + std::to_string(preLength) +
               " post=" + std::to_string(mesh.length()) + ")");

    // Test with Vulcan Dkyr Class
    std::string vulcan = "cpp_client/assets/reference_models/Vulcan Dkyr Class/VulcanDKyrClass.obj";
    if (fileExists(vulcan)) {
        OBJSeedMesh vmesh = ProceduralShipGenerator::parseOBJ(vulcan);
        assertTrue(!vmesh.empty(), "Vulcan seed loaded for processing");
        vmesh.centreAtOrigin();
        vmesh.normalizeScale(100.0f);
        vmesh.computeBounds();
        float vmax = std::max({vmesh.length(), vmesh.width(), vmesh.height()});
        assertTrue(std::abs(vmax - 100.0f) < 5.0f,
                   "Vulcan normalized longest axis ~100 (got " + std::to_string(vmax) + ")");
    }
}

// ─── Main ─────────────────────────────────────────────────────────────

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "OBJ Reference Model Integration Tests" << std::endl;
    std::cout << "========================================" << std::endl;

    testFindSeedOBJ();
    testParseReferenceOBJs();
    testParseModularOBJs();
    testMountPointDetection();
    testSeedMeshProcessing();

    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << testsPassed << " / " << testsRun << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}
