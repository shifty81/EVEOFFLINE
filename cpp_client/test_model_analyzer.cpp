/**
 * Test program for the ReferenceModelAnalyzer.
 *
 * Extracts and analyzes the 3D models uploaded in the testing/ directory:
 *   - testing/99-intergalactic_spaceship-obj.rar  (Intergalactic Spaceship OBJ)
 *   - testing/qy0sx26192io-VulcanDkyrClass.zip    (Vulcan Dkyr Class OBJ)
 *
 * Validates that the analyzer correctly extracts geometric traits (aspect
 * ratios, cross-section profiles, radius multipliers) and that the learned
 * parameters can drive the procedural hull builder to produce valid geometry.
 */

#include "rendering/reference_model_analyzer.h"
#include "rendering/procedural_mesh_ops.h"
#include "rendering/ship_generation_rules.h"
#include <iostream>
#include <cassert>
#include <string>
#include <cmath>
#include <filesystem>

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

bool approxEqual(float a, float b, float epsilon = 0.1f) {
    return std::fabs(a - b) < epsilon;
}

// ─── Locate testing directory ──────────────────────────────────────────

std::string findTestingDir() {
    // Try common locations relative to executable and source tree
    std::vector<std::string> candidates = {
        "testing",
        "../testing",
        "../../testing",
        "../../../testing",
    };

    // Also try absolute path from the repository root
    const char* envRoot = std::getenv("REPO_ROOT");
    std::string repoRoot = envRoot ? envRoot : "";
    if (!repoRoot.empty()) {
        candidates.insert(candidates.begin(), repoRoot + "/testing");
    }

    for (const auto& dir : candidates) {
        if (std::filesystem::exists(dir) &&
            std::filesystem::is_directory(dir)) {
            return dir;
        }
    }
    return "";
}

// ─── Test: OBJ parsing ────────────────────────────────────────────────

void testOBJParsing(const std::string& testingDir) {
    std::cout << "\n=== OBJ Parsing & Analysis ===" << std::endl;

    ReferenceModelAnalyzer analyzer;

    // Analyze the two uploaded model archives
    std::string spaceshipArchive = testingDir + "/99-intergalactic_spaceship-obj.rar";
    std::string vulcanArchive    = testingDir + "/qy0sx26192io-VulcanDkyrClass.zip";

    std::string extractDir = "/tmp/eve_model_analysis";
    std::filesystem::create_directories(extractDir);

    int count1 = analyzer.analyzeArchive(spaceshipArchive, extractDir + "/spaceship");
    assertTrue(count1 >= 1, "Intergalactic Spaceship archive contains at least 1 OBJ");

    int count2 = analyzer.analyzeArchive(vulcanArchive, extractDir + "/vulcan");
    assertTrue(count2 >= 1, "Vulcan Dkyr Class archive contains at least 1 OBJ");

    assertTrue(analyzer.getModelCount() >= 2,
               "Analyzer has at least 2 models total (got " +
               std::to_string(analyzer.getModelCount()) + ")");
}

// ─── Test: Trait extraction ───────────────────────────────────────────

void testTraitExtraction(const ReferenceModelAnalyzer& analyzer) {
    std::cout << "\n=== Trait Extraction ===" << std::endl;

    for (size_t i = 0; i < analyzer.getModelCount(); ++i) {
        const auto& traits = analyzer.getModelTraits(i);
        std::string prefix = traits.name + ": ";

        assertTrue(traits.vertexCount > 0,
                   prefix + "has vertices (" + std::to_string(traits.vertexCount) + ")");
        assertTrue(traits.faceCount > 0,
                   prefix + "has faces (" + std::to_string(traits.faceCount) + ")");

        assertTrue(traits.length > 0.0f,
                   prefix + "length > 0 (" + std::to_string(traits.length) + ")");
        assertTrue(traits.width > 0.0f,
                   prefix + "width > 0 (" + std::to_string(traits.width) + ")");
        assertTrue(traits.height > 0.0f,
                   prefix + "height > 0 (" + std::to_string(traits.height) + ")");

        assertTrue(traits.aspectLW >= 1.0f,
                   prefix + "L:W >= 1.0 (" + std::to_string(traits.aspectLW) + ")");
        assertTrue(traits.aspectLH >= 1.0f,
                   prefix + "L:H >= 1.0 (" + std::to_string(traits.aspectLH) + ")");

        assertTrue(!traits.crossSectionProfile.empty(),
                   prefix + "has cross-section profile (" +
                   std::to_string(traits.crossSectionProfile.size()) + " slices)");

        assertTrue(!traits.radiusMultipliers.empty(),
                   prefix + "has radius multipliers (" +
                   std::to_string(traits.radiusMultipliers.size()) + " values)");

        assertTrue(traits.baseRadius > 0.0f,
                   prefix + "baseRadius > 0 (" + std::to_string(traits.baseRadius) + ")");

        assertTrue(traits.dominantFaceSides >= 3,
                   prefix + "dominant face sides >= 3 (" +
                   std::to_string(traits.dominantFaceSides) + ")");

        // Print summary
        std::cout << "    " << traits.name << " summary:" << std::endl;
        std::cout << "      Dims: " << traits.length << " x " << traits.width
                  << " x " << traits.height << std::endl;
        std::cout << "      L:W=" << traits.aspectLW << " L:H=" << traits.aspectLH << std::endl;
        std::cout << "      Verts=" << traits.vertexCount << " Faces=" << traits.faceCount << std::endl;
        std::cout << "      Faction=" << traits.inferredFaction
                  << " Class=" << traits.inferredClass << std::endl;
        std::cout << "      Profile: [";
        for (size_t j = 0; j < traits.crossSectionProfile.size(); ++j) {
            if (j > 0) std::cout << ", ";
            std::cout << std::fixed;
            std::cout.precision(3);
            std::cout << traits.crossSectionProfile[j];
        }
        std::cout << "]" << std::endl;
    }
}

// ─── Test: Learned parameters computation ─────────────────────────────

void testLearnedParams(const ReferenceModelAnalyzer& analyzer) {
    std::cout << "\n=== Learned Generation Parameters ===" << std::endl;

    auto params = analyzer.computeLearnedParams();

    assertTrue(params.modelCount >= 2,
               "Learned from at least 2 models (got " +
               std::to_string(params.modelCount) + ")");

    assertTrue(params.avgAspectLW > 1.0f,
               "Average L:W > 1.0 (" + std::to_string(params.avgAspectLW) + ")");
    assertTrue(params.minAspectLW > 0.0f,
               "Min L:W > 0 (" + std::to_string(params.minAspectLW) + ")");
    assertTrue(params.maxAspectLW >= params.minAspectLW,
               "Max L:W >= Min L:W");

    assertTrue(params.avgVertexCount > 0,
               "Average vertex count > 0 (" + std::to_string(params.avgVertexCount) + ")");

    assertTrue(!params.blendedProfile.empty(),
               "Blended profile has data (" +
               std::to_string(params.blendedProfile.size()) + " slices)");

    assertTrue(!params.blendedRadiusMultipliers.empty(),
               "Blended radius multipliers computed (" +
               std::to_string(params.blendedRadiusMultipliers.size()) + " values)");

    // Print learned params
    std::cout << "    Avg L:W = " << params.avgAspectLW
              << " [" << params.minAspectLW << " - " << params.maxAspectLW << "]" << std::endl;
    std::cout << "    Avg L:H = " << params.avgAspectLH << std::endl;
    std::cout << "    Avg verts = " << params.avgVertexCount
              << ", Avg faces = " << params.avgFaceCount << std::endl;
    std::cout << "    Avg base radius = " << params.avgBaseRadius << std::endl;

    std::cout << "    Blended profile: [";
    for (size_t i = 0; i < params.blendedProfile.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << std::fixed;
        std::cout.precision(3);
        std::cout << params.blendedProfile[i];
    }
    std::cout << "]" << std::endl;

    std::cout << "    Blended multipliers: [";
    for (size_t i = 0; i < params.blendedRadiusMultipliers.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << std::fixed;
        std::cout.precision(3);
        std::cout << params.blendedRadiusMultipliers[i];
    }
    std::cout << "]" << std::endl;
}

// ─── Test: ReferenceModelTraits conversion ────────────────────────────

void testReferenceModelTraits(const ReferenceModelAnalyzer& analyzer) {
    std::cout << "\n=== ReferenceModelTraits Conversion ===" << std::endl;

    auto refTraits = analyzer.toReferenceModelTraits();

    assertTrue(refTraits.avgAspectLW > 1.0f,
               "ReferenceModelTraits avgAspectLW > 1.0");
    assertTrue(refTraits.avgVertexCount > 0,
               "ReferenceModelTraits avgVertexCount > 0");
    assertTrue(refTraits.detailDensityMultiplier > 0.0f,
               "Detail density multiplier > 0 (" +
               std::to_string(refTraits.detailDensityMultiplier) + ")");

    std::cout << "    Detail density multiplier: " << refTraits.detailDensityMultiplier
              << "x (relative to frigate baseline)" << std::endl;
}

// ─── Test: Learned radius multipliers ─────────────────────────────────

void testLearnedRadiusMultipliers(const ReferenceModelAnalyzer& analyzer) {
    std::cout << "\n=== Learned Radius Multipliers ===" << std::endl;

    // Generate for different segment counts
    int segmentCounts[] = {4, 6, 8, 10};
    for (int segs : segmentCounts) {
        auto mults = analyzer.generateLearnedRadiusMultipliers(segs, 42u);
        assertTrue(static_cast<int>(mults.size()) == segs,
                   "Multipliers count matches segments (" + std::to_string(segs) + ")");

        bool allPositive = true;
        for (float m : mults) {
            if (m <= 0.0f) { allPositive = false; break; }
        }
        assertTrue(allPositive,
                   "All multipliers are positive for " + std::to_string(segs) + " segments");
    }

    // Deterministic: same seed → same result
    auto mults1 = analyzer.generateLearnedRadiusMultipliers(6, 123u);
    auto mults2 = analyzer.generateLearnedRadiusMultipliers(6, 123u);
    assertTrue(mults1 == mults2, "Same seed produces same learned multipliers");
}

// ─── Test: Generate hull from learned parameters ──────────────────────

void testLearnedHullGeneration(const ReferenceModelAnalyzer& analyzer) {
    std::cout << "\n=== Hull Generation from Learned Parameters ===" << std::endl;

    auto params = analyzer.computeLearnedParams();

    // Generate hulls with different faction styles using learned profile
    struct FactionConfig {
        int sides;
        const char* name;
    };
    FactionConfig factions[] = {
        {4,  "Veyren-style (learned)"},
        {6,  "Keldari-style (learned)"},
        {8,  "Solari-style (learned)"},
        {12, "Aurelian-style (learned)"},
    };

    int segments = 6;
    float segLen = 1.0f;

    for (const auto& f : factions) {
        auto mults = analyzer.generateLearnedRadiusMultipliers(segments, 42u);

        TriangulatedMesh hull = buildSegmentedHull(
            f.sides, segments, segLen, params.avgBaseRadius,
            mults, 1.0f, 1.0f, glm::vec3(0.5f));

        assertTrue(!hull.vertices.empty(),
                   std::string(f.name) + " hull has vertices");
        assertTrue(!hull.indices.empty(),
                   std::string(f.name) + " hull has indices");
        assertTrue(hull.indices.size() % 3 == 0,
                   std::string(f.name) + " index count is multiple of 3");

        // Verify no NaN/Inf
        bool noNaN = true;
        for (const auto& v : hull.vertices) {
            if (std::isnan(v.position.x) || std::isnan(v.position.y) ||
                std::isnan(v.position.z) || std::isinf(v.position.x) ||
                std::isinf(v.position.y) || std::isinf(v.position.z)) {
                noNaN = false;
                break;
            }
        }
        assertTrue(noNaN, std::string(f.name) + " no NaN/Inf in positions");

        // Verify no out-of-range indices
        bool validIndices = true;
        unsigned int maxIdx = static_cast<unsigned int>(hull.vertices.size());
        for (auto idx : hull.indices) {
            if (idx >= maxIdx) { validIndices = false; break; }
        }
        assertTrue(validIndices,
                   std::string(f.name) + " all indices within vertex range");

        std::cout << "    " << f.name << ": "
                  << hull.vertices.size() << " verts, "
                  << hull.indices.size() / 3 << " tris" << std::endl;
    }
}

// ─── Test: Multiple seeds produce variation ───────────────────────────

void testLearnedVariation(const ReferenceModelAnalyzer& analyzer) {
    std::cout << "\n=== Learned Variation (different seeds) ===" << std::endl;

    auto mults1 = analyzer.generateLearnedRadiusMultipliers(6, 100u);
    auto mults2 = analyzer.generateLearnedRadiusMultipliers(6, 200u);

    bool different = (mults1 != mults2);
    assertTrue(different, "Different seeds produce different multipliers");

    // Both should produce valid hulls
    for (unsigned int seed : {100u, 200u, 300u}) {
        auto mults = analyzer.generateLearnedRadiusMultipliers(6, seed);
        TriangulatedMesh hull = buildSegmentedHull(
            6, 6, 1.0f, 1.0f, mults, 1.0f, 1.0f, glm::vec3(0.5f));
        assertTrue(!hull.vertices.empty(),
                   "Seed " + std::to_string(seed) + " produces valid hull");
    }
}

// ─── main ──────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    std::cout << "[Test] Reference Model Analyzer Test Suite" << std::endl;
    std::cout << "Analyzes uploaded models in testing/ directory and validates" << std::endl;
    std::cout << "that learned parameters produce valid procedural geometry." << std::endl;

    // Find testing directory
    std::string testingDir = findTestingDir();
    if (testingDir.empty()) {
        // Allow override via command line
        if (argc > 1) {
            testingDir = argv[1];
        }
    }

    if (testingDir.empty() || !std::filesystem::exists(testingDir)) {
        std::cerr << "ERROR: Cannot find testing/ directory." << std::endl;
        std::cerr << "Run from the repository root or pass the path as argument." << std::endl;
        std::cerr << "Usage: " << argv[0] << " [path/to/testing]" << std::endl;
        return 1;
    }

    std::cout << "Using testing directory: " << testingDir << std::endl;

    // Run analysis
    ReferenceModelAnalyzer analyzer;

    std::string extractDir = "/tmp/eve_model_analysis";
    std::filesystem::create_directories(extractDir);

    // Analyze both uploaded archives
    analyzer.analyzeArchive(testingDir + "/99-intergalactic_spaceship-obj.rar",
                           extractDir + "/spaceship");
    analyzer.analyzeArchive(testingDir + "/qy0sx26192io-VulcanDkyrClass.zip",
                           extractDir + "/vulcan");

    // Run test suites
    testOBJParsing(testingDir);
    testTraitExtraction(analyzer);
    testLearnedParams(analyzer);
    testReferenceModelTraits(analyzer);
    testLearnedRadiusMultipliers(analyzer);
    testLearnedHullGeneration(analyzer);
    testLearnedVariation(analyzer);

    // Summary
    std::cout << "\n=== Results ===" << std::endl;
    std::cout << testsPassed << " / " << testsRun << " tests passed" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}
