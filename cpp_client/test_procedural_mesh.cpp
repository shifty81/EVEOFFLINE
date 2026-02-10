/**
 * Test program for procedural mesh generation operations.
 *
 * Validates that the core mesh generation algorithms (polygon generation,
 * extrusion, stitching, bevel cuts, pyramidize, segmented hull) produce
 * correct geometry, matching the approach from the reference project
 * (AlexSanfilippo/ProceduralMeshGeneration).
 *
 * Key regression: extrusion must scale cross-section (X/Z) only, NOT
 * the extrusion-axis component, to prevent "squiggly" hull output.
 */

#include "rendering/procedural_mesh_ops.h"
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>

using namespace eve;

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

bool approxEqual(float a, float b, float epsilon = 0.001f) {
    return std::fabs(a - b) < epsilon;
}

bool approxEqualVec3(const glm::vec3& a, const glm::vec3& b, float eps = 0.001f) {
    return approxEqual(a.x, b.x, eps) &&
           approxEqual(a.y, b.y, eps) &&
           approxEqual(a.z, b.z, eps);
}

// ─── Polygon generation tests ──────────────────────────────────────────

void testPolygonGeneration() {
    std::cout << "\n=== Polygon Generation ===" << std::endl;

    // A square (4-sided polygon) with radius 1.0 at origin
    PolyFace square = generatePolygonFace(4, 1.0f);
    assertTrue(square.sides() == 4, "Square has 4 vertices");

    // All vertices should be at distance 1.0 from origin in the XZ plane
    for (int i = 0; i < square.sides(); ++i) {
        float dist = glm::length(glm::vec2(square.outerVertices[i].x,
                                            square.outerVertices[i].z));
        assertTrue(approxEqual(dist, 1.0f, 0.01f),
                   "Square vertex " + std::to_string(i) + " at radius 1.0");
    }

    // Triangle
    PolyFace tri = generatePolygonFace(3, 2.0f);
    assertTrue(tri.sides() == 3, "Triangle has 3 vertices");

    // Hexagon with scaleX
    PolyFace hex = generatePolygonFace(6, 1.0f, glm::vec3(0), glm::vec3(0,1,0), 2.0f, 1.0f);
    assertTrue(hex.sides() == 6, "Hexagon has 6 vertices");
}

// ─── Extrusion tests ───────────────────────────────────────────────────

void testExtrusion() {
    std::cout << "\n=== Extrusion ===" << std::endl;

    // Create a square face in the XZ plane with normal +Y
    PolyFace base = generatePolygonFace(4, 1.0f, glm::vec3(0),
                                         glm::vec3(0, 1, 0));

    // Extrude along +Y by 2.0 with no scaling
    PolyFace ext1 = extrudeFace(base, 2.0f, 1.0f, glm::vec3(0, 1, 0));
    assertTrue(ext1.sides() == 4, "Extruded face has same vertex count");

    // All extruded vertices should have Y ≈ 2.0
    for (int i = 0; i < ext1.sides(); ++i) {
        assertTrue(approxEqual(ext1.outerVertices[i].y, 2.0f, 0.01f),
                   "Extruded vertex " + std::to_string(i) + " at Y=2.0");
    }

    // Extrude with scale 0.5 — cross-section (X/Z) should halve but Y stays at 2.0
    PolyFace ext2 = extrudeFace(base, 2.0f, 0.5f, glm::vec3(0, 1, 0));
    for (int i = 0; i < ext2.sides(); ++i) {
        float baseX = base.outerVertices[i].x;
        float baseZ = base.outerVertices[i].z;
        assertTrue(approxEqual(ext2.outerVertices[i].x, baseX * 0.5f, 0.01f),
                   "Scaled extrude X halved for vertex " + std::to_string(i));
        assertTrue(approxEqual(ext2.outerVertices[i].z, baseZ * 0.5f, 0.01f),
                   "Scaled extrude Z halved for vertex " + std::to_string(i));
        assertTrue(approxEqual(ext2.outerVertices[i].y, 2.0f, 0.01f),
                   "Scaled extrude Y unchanged at 2.0 for vertex " + std::to_string(i));
    }
}

// ─── Key regression test: no axis-drift on repeated extrusion ──────────

void testRepeatedExtrusionNoDrift() {
    std::cout << "\n=== Repeated Extrusion — No Squiggly Drift ===" << std::endl;

    // Create a base polygon centered at origin
    glm::vec3 fwd(0, 1, 0);
    PolyFace face = generatePolygonFace(6, 1.0f, glm::vec3(0), fwd);

    // Extrude 10 times with varying scale factors
    float scales[] = {1.2f, 0.9f, 1.1f, 0.8f, 1.3f, 0.7f, 1.0f, 1.1f, 0.95f, 1.05f};
    PolyFace current = face;
    for (int i = 0; i < 10; ++i) {
        current = extrudeFace(current, 1.0f, scales[i], fwd);
    }

    // After 10 extrusions of length 1.0 along +Y, the centroid Y should be ≈ 10.0
    glm::vec3 cen = current.centroid();
    assertTrue(approxEqual(cen.y, 10.0f, 0.01f),
               "Centroid Y = 10.0 after 10 unit extrusions");

    // The centroid X and Z should still be at 0 (no lateral drift)
    assertTrue(approxEqual(cen.x, 0.0f, 0.01f),
               "Centroid X stays at 0 (no squiggly drift)");
    assertTrue(approxEqual(cen.z, 0.0f, 0.01f),
               "Centroid Z stays at 0 (no squiggly drift)");
}

// ─── Stitch tests ──────────────────────────────────────────────────────

void testStitching() {
    std::cout << "\n=== Face Stitching ===" << std::endl;

    PolyFace faceA = generatePolygonFace(4, 1.0f);
    PolyFace faceB = extrudeFace(faceA, 2.0f, 1.0f, glm::vec3(0, 1, 0));

    auto walls = stitchFaces(faceA, faceB);
    assertTrue(static_cast<int>(walls.size()) == 4,
               "4-sided stitch produces 4 wall quads");
    for (size_t i = 0; i < walls.size(); ++i) {
        assertTrue(walls[i].sides() == 4,
                   "Wall quad " + std::to_string(i) + " has 4 vertices");
    }
}

// ─── Bevel cut tests ──────────────────────────────────────────────────

void testBevelCut() {
    std::cout << "\n=== Bevel Cut ===" << std::endl;

    PolyFace hex = generatePolygonFace(6, 1.0f);
    auto result = bevelCutFace(hex, 0.3f, 0.5f);
    // Should produce N border quads + 1 inner face = 7 total
    assertTrue(static_cast<int>(result.size()) == 7,
               "6-sided bevel produces 7 faces (6 border + 1 inner)");
}

// ─── Pyramidize tests ─────────────────────────────────────────────────

void testPyramidize() {
    std::cout << "\n=== Pyramidize ===" << std::endl;

    PolyFace quad = generatePolygonFace(4, 1.0f);
    auto pyra = pyramidizeFace(quad, 1.0f);
    assertTrue(static_cast<int>(pyra.size()) == 4,
               "4-sided pyramid produces 4 triangles");
    for (size_t i = 0; i < pyra.size(); ++i) {
        assertTrue(pyra[i].sides() == 3,
                   "Pyramid face " + std::to_string(i) + " is a triangle");
    }
}

// ─── Subdivide quad ───────────────────────────────────────────────────

void testSubdivide() {
    std::cout << "\n=== Subdivide Lengthwise ===" << std::endl;

    PolyFace quad = generatePolygonFace(4, 1.0f);
    auto strips = subdivideFaceLengthwise(quad, 3);
    assertTrue(static_cast<int>(strips.size()) == 3,
               "Subdivide by 3 produces 3 strips");
    for (size_t i = 0; i < strips.size(); ++i) {
        assertTrue(strips[i].sides() == 4,
                   "Strip " + std::to_string(i) + " is a quad");
    }
}

// ─── Bezier helpers ───────────────────────────────────────────────────

void testBezier() {
    std::cout << "\n=== Bezier Helpers ===" << std::endl;

    glm::vec3 a(0, 0, 0), b(1, 2, 0), c(2, 0, 0);

    // Endpoints should match
    assertTrue(approxEqualVec3(bezierQuadratic(a, b, c, 0.0f), a),
               "Quadratic Bezier at t=0 equals start");
    assertTrue(approxEqualVec3(bezierQuadratic(a, b, c, 1.0f), c),
               "Quadratic Bezier at t=1 equals end");

    // Sample should return correct number of points
    auto samples = sampleBezierQuadratic(a, b, c, 10);
    assertTrue(static_cast<int>(samples.size()) == 11,
               "Sample quadratic with 10 intervals gives 11 points");
}

// ─── Segmented hull builder ───────────────────────────────────────────

void testSegmentedHull() {
    std::cout << "\n=== Segmented Hull Builder ===" << std::endl;

    int sides = 6;
    int segments = 4;
    float segLen = 1.0f;
    float baseRadius = 1.0f;

    auto mults = generateRadiusMultipliers(segments, baseRadius, 123u);
    assertTrue(static_cast<int>(mults.size()) == segments,
               "Radius multipliers count matches segments");

    TriangulatedMesh hull = buildSegmentedHull(
        sides, segments, segLen, baseRadius, mults, 1.0f, 1.0f,
        glm::vec3(0.5f));

    assertTrue(!hull.vertices.empty(), "Hull has vertices");
    assertTrue(!hull.indices.empty(), "Hull has indices");
    assertTrue(hull.indices.size() % 3 == 0, "Index count is multiple of 3");

    // With nose + thrusters + wall details, the hull should have
    // substantially more geometry than just the bare segments
    int wallOnlyFaces = sides * segments;  // bare wall quads
    int triangleCount = static_cast<int>(hull.indices.size()) / 3;
    assertTrue(triangleCount > wallOnlyFaces,
               "Hull triangle count (" + std::to_string(triangleCount) +
               ") exceeds bare wall count (" + std::to_string(wallOnlyFaces) +
               ") — nose/thruster/detail geometry present");

    // Verify no NaN/Inf in vertex positions
    bool hasNaN = false;
    for (const auto& v : hull.vertices) {
        if (std::isnan(v.position.x) || std::isnan(v.position.y) || std::isnan(v.position.z) ||
            std::isinf(v.position.x) || std::isinf(v.position.y) || std::isinf(v.position.z)) {
            hasNaN = true;
            break;
        }
    }
    assertTrue(!hasNaN, "No NaN/Inf in hull vertex positions");

    std::cout << "  Hull stats: " << hull.vertices.size() << " vertices, "
              << hull.indices.size() / 3 << " triangles" << std::endl;
}

// ─── Deterministic generation (seed produces same result) ─────────────

void testDeterministic() {
    std::cout << "\n=== Deterministic Generation ===" << std::endl;

    auto mults1 = generateRadiusMultipliers(4, 1.0f, 42u);
    auto mults2 = generateRadiusMultipliers(4, 1.0f, 42u);
    assertTrue(mults1 == mults2, "Same seed produces same radius multipliers");

    TriangulatedMesh hull1 = buildSegmentedHull(
        6, 4, 1.0f, 1.0f, mults1, 1.0f, 1.0f, glm::vec3(0.5f));
    TriangulatedMesh hull2 = buildSegmentedHull(
        6, 4, 1.0f, 1.0f, mults2, 1.0f, 1.0f, glm::vec3(0.5f));
    assertTrue(hull1.vertices.size() == hull2.vertices.size(),
               "Same params produce same vertex count");
    assertTrue(hull1.indices.size() == hull2.indices.size(),
               "Same params produce same index count");
}

// ─── Ship class hull generation (validates no squiggly triangles) ──────

void testShipClassHulls() {
    std::cout << "\n=== Ship Class Hull Generation ===" << std::endl;

    // Each entry: {sides, segments, segLength, baseRadius, scaleX, scaleZ, seed, label}
    struct ShipClassParams {
        int sides; int segments; float segLen; float baseR;
        float scaleX; float scaleZ; unsigned int seed;
        const char* label;
    };
    ShipClassParams classes[] = {
        {6,  4,  0.85f, 0.45f, 1.0f, 0.8f,  100u, "Frigate"},
        {6,  5,  1.0f,  0.35f, 0.8f, 0.7f,  200u, "Destroyer"},
        {6,  6,  1.0f,  0.65f, 1.2f, 0.8f,  300u, "Cruiser"},
        {6,  7,  1.2f,  0.8f,  1.1f, 0.9f,  500u, "Battlecruiser"},
        {8,  8,  1.5f,  1.0f,  1.2f, 0.85f, 600u, "Battleship"},
        {4,  5,  1.2f,  0.9f,  1.5f, 0.7f,  700u, "MiningBarge"},
        {8,  10, 1.5f,  1.2f,  1.6f, 0.6f,  800u, "Carrier"},
        {6,  8,  1.5f,  1.3f,  1.0f, 1.1f,  900u, "Dreadnought"},
        {8,  12, 2.0f,  1.8f,  1.1f, 0.9f,  1000u,"Titan"},
    };

    for (const auto& c : classes) {
        auto mults = generateRadiusMultipliers(c.segments, c.baseR, c.seed);
        TriangulatedMesh hull = buildSegmentedHull(
            c.sides, c.segments, c.segLen, c.baseR,
            mults, c.scaleX, c.scaleZ, glm::vec3(0.5f));

        assertTrue(!hull.vertices.empty(),
                   std::string(c.label) + " has vertices");
        assertTrue(!hull.indices.empty(),
                   std::string(c.label) + " has indices");
        assertTrue(hull.indices.size() % 3 == 0,
                   std::string(c.label) + " index count is multiple of 3 (valid GL_TRIANGLES)");

        // Verify no out-of-range indices (prevents random line artifacts)
        bool validIndices = true;
        unsigned int maxIdx = static_cast<unsigned int>(hull.vertices.size());
        for (auto idx : hull.indices) {
            if (idx >= maxIdx) { validIndices = false; break; }
        }
        assertTrue(validIndices,
                   std::string(c.label) + " all indices within vertex range");

        // Verify no degenerate triangles (area > 0) for first 10 triangles
        bool noDegenerate = true;
        int triCount = std::min(10, static_cast<int>(hull.indices.size() / 3));
        for (int t = 0; t < triCount; ++t) {
            glm::vec3 v0 = hull.vertices[hull.indices[t*3+0]].position;
            glm::vec3 v1 = hull.vertices[hull.indices[t*3+1]].position;
            glm::vec3 v2 = hull.vertices[hull.indices[t*3+2]].position;
            float area = glm::length(glm::cross(v1 - v0, v2 - v0)) * 0.5f;
            if (area < 1e-8f) { noDegenerate = false; break; }
        }
        assertTrue(noDegenerate,
                   std::string(c.label) + " no degenerate zero-area triangles");

        // Verify no NaN/Inf
        bool noNaN = true;
        for (const auto& v : hull.vertices) {
            if (std::isnan(v.position.x) || std::isnan(v.position.y) || std::isnan(v.position.z) ||
                std::isinf(v.position.x) || std::isinf(v.position.y) || std::isinf(v.position.z)) {
                noNaN = false; break;
            }
        }
        assertTrue(noNaN,
                   std::string(c.label) + " no NaN/Inf in positions");

        std::cout << "    " << c.label << ": " << hull.vertices.size()
                  << " verts, " << hull.indices.size() / 3 << " tris" << std::endl;
    }
}

// ─── Faction-specific sides variation ─────────────────────────────────

void testFactionSidesVariation() {
    std::cout << "\n=== Faction Sides Variation ===" << std::endl;

    // Different factions should produce hulls with different silhouettes
    int factionSides[] = {4, 6, 8, 12};  // Caldari, Minmatar, Amarr, Gallente
    const char* names[] = {"Caldari(4)", "Minmatar(6)", "Amarr(8)", "Gallente(12)"};

    for (int f = 0; f < 4; ++f) {
        auto mults = generateRadiusMultipliers(4, 0.5f, 42u);
        TriangulatedMesh hull = buildSegmentedHull(
            factionSides[f], 4, 1.0f, 0.5f,
            mults, 1.0f, 1.0f, glm::vec3(0.5f));
        assertTrue(!hull.vertices.empty(),
                   std::string(names[f]) + " hull generated");
        assertTrue(hull.indices.size() % 3 == 0,
                   std::string(names[f]) + " valid triangles");
    }
}

// ─── main ──────────────────────────────────────────────────────────────

int main() {
    std::cout << "[Test] Procedural Mesh Generation Test Suite" << std::endl;

    testPolygonGeneration();
    testExtrusion();
    testRepeatedExtrusionNoDrift();
    testStitching();
    testBevelCut();
    testPyramidize();
    testSubdivide();
    testBezier();
    testSegmentedHull();
    testDeterministic();
    testShipClassHulls();
    testFactionSidesVariation();

    std::cout << "\n=== Results ===" << std::endl;
    std::cout << testsPassed << " / " << testsRun << " tests passed" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}
