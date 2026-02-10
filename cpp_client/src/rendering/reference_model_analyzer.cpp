#include "rendering/reference_model_analyzer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <random>
#include <filesystem>

namespace eve {

// ────────────────────────────────────────────────────────────────────────
// Construction / destruction
// ────────────────────────────────────────────────────────────────────────

ReferenceModelAnalyzer::ReferenceModelAnalyzer() {}
ReferenceModelAnalyzer::~ReferenceModelAnalyzer() {}

// ────────────────────────────────────────────────────────────────────────
// Lightweight OBJ parser (vertex + face only, no OpenGL)
// ────────────────────────────────────────────────────────────────────────

bool ReferenceModelAnalyzer::parseOBJGeometry(
    const std::string& path,
    std::vector<glm::vec3>& outVertices,
    std::vector<std::vector<int>>& outFaces)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "[ReferenceModelAnalyzer] Failed to open: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {
            float x, y, z;
            if (iss >> x >> y >> z) {
                outVertices.emplace_back(x, y, z);
            }
        } else if (prefix == "f") {
            std::vector<int> faceVerts;
            std::string token;
            while (iss >> token) {
                // Handle formats: "v", "v/vt", "v/vt/vn", "v//vn"
                int vi = 0;
                size_t slashPos = token.find('/');
                if (slashPos != std::string::npos) {
                    vi = std::stoi(token.substr(0, slashPos));
                } else {
                    vi = std::stoi(token);
                }
                // OBJ indices are 1-based
                faceVerts.push_back(vi - 1);
            }
            if (faceVerts.size() >= 3) {
                outFaces.push_back(std::move(faceVerts));
            }
        }
    }

    return !outVertices.empty();
}

// ────────────────────────────────────────────────────────────────────────
// Cross-section profile computation
// ────────────────────────────────────────────────────────────────────────

std::vector<float> ReferenceModelAnalyzer::computeCrossSectionProfile(
    const std::vector<glm::vec3>& vertices,
    int longestAxis, float axisMin, float axisMax,
    int numSlices)
{
    std::vector<float> profile(numSlices, 0.0f);
    if (vertices.empty() || numSlices < 1) return profile;

    float axisRange = axisMax - axisMin;
    if (axisRange < 1e-7f) return profile;

    // Compute centroid for the two non-longest axes
    int otherAxis1 = (longestAxis + 1) % 3;
    int otherAxis2 = (longestAxis + 2) % 3;

    float centerA1 = 0.0f, centerA2 = 0.0f;
    for (const auto& v : vertices) {
        centerA1 += v[otherAxis1];
        centerA2 += v[otherAxis2];
    }
    centerA1 /= static_cast<float>(vertices.size());
    centerA2 /= static_cast<float>(vertices.size());

    // For each vertex, determine which slice it belongs to and track max radius
    for (const auto& v : vertices) {
        float t = (v[longestAxis] - axisMin) / axisRange;
        int slice = static_cast<int>(t * numSlices);
        if (slice >= numSlices) slice = numSlices - 1;
        if (slice < 0) slice = 0;

        float da1 = v[otherAxis1] - centerA1;
        float da2 = v[otherAxis2] - centerA2;
        float radius = std::sqrt(da1 * da1 + da2 * da2);

        if (radius > profile[slice]) {
            profile[slice] = radius;
        }
    }

    return profile;
}

// ────────────────────────────────────────────────────────────────────────
// Profile → radius multipliers
// ────────────────────────────────────────────────────────────────────────

std::vector<float> ReferenceModelAnalyzer::profileToRadiusMultipliers(
    const std::vector<float>& profile)
{
    if (profile.size() < 2) return {1.0f};

    std::vector<float> multipliers;
    multipliers.reserve(profile.size() - 1);

    for (size_t i = 1; i < profile.size(); ++i) {
        float prev = profile[i - 1];
        if (prev < 1e-7f) prev = 1e-7f;
        multipliers.push_back(profile[i] / prev);
    }

    return multipliers;
}

// ────────────────────────────────────────────────────────────────────────
// Faction / class inference from file name
// ────────────────────────────────────────────────────────────────────────

std::string ReferenceModelAnalyzer::inferFaction(const std::string& filename) {
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    if (lower.find("amarr") != std::string::npos) return "Amarr";
    if (lower.find("caldari") != std::string::npos) return "Caldari";
    if (lower.find("gallente") != std::string::npos) return "Gallente";
    if (lower.find("minmatar") != std::string::npos) return "Minmatar";
    if (lower.find("triglavian") != std::string::npos) return "Triglavian";
    if (lower.find("sansha") != std::string::npos) return "Sansha";
    if (lower.find("angel") != std::string::npos) return "Angel";
    if (lower.find("ore") != std::string::npos) return "ORE";
    if (lower.find("vulcan") != std::string::npos) return "Amarr";  // Vulcan class -> Amarr style
    if (lower.find("intergalactic") != std::string::npos ||
        lower.find("spaceship") != std::string::npos) return "Gallente";  // Sleek spaceship -> Gallente
    return "Unknown";
}

std::string ReferenceModelAnalyzer::inferClass(const std::string& filename, int vertexCount) {
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    // Try to match from filename
    if (lower.find("titan") != std::string::npos) return "Titan";
    if (lower.find("dreadnought") != std::string::npos) return "Dreadnought";
    if (lower.find("carrier") != std::string::npos) return "Carrier";
    if (lower.find("battleship") != std::string::npos) return "Battleship";
    if (lower.find("battlecruiser") != std::string::npos) return "Battlecruiser";
    if (lower.find("cruiser") != std::string::npos) return "Cruiser";
    if (lower.find("destroyer") != std::string::npos) return "Destroyer";
    if (lower.find("frigate") != std::string::npos) return "Frigate";

    // Infer from vertex count (using OBJ_MODEL_ANALYSIS.md ranges)
    if (vertexCount > 30000) return "Titan";
    if (vertexCount > 20000) return "Carrier";
    if (vertexCount > 15000) return "Dreadnought";
    if (vertexCount > 11000) return "Battleship";
    if (vertexCount > 9000) return "Cruiser";
    if (vertexCount > 7000) return "Destroyer";
    return "Frigate";
}

// ────────────────────────────────────────────────────────────────────────
// Analyze a single OBJ file
// ────────────────────────────────────────────────────────────────────────

bool ReferenceModelAnalyzer::analyzeOBJ(const std::string& objPath) {
    std::vector<glm::vec3> vertices;
    std::vector<std::vector<int>> faces;

    if (!parseOBJGeometry(objPath, vertices, faces)) {
        std::cerr << "[ReferenceModelAnalyzer] Failed to parse: " << objPath << std::endl;
        return false;
    }

    AnalyzedModelTraits traits;

    // Extract file name
    std::filesystem::path p(objPath);
    traits.name = p.stem().string();

    // Counts
    traits.vertexCount = static_cast<int>(vertices.size());
    traits.faceCount = static_cast<int>(faces.size());

    // Bounding box
    glm::vec3 bmin(std::numeric_limits<float>::max());
    glm::vec3 bmax(std::numeric_limits<float>::lowest());
    for (const auto& v : vertices) {
        bmin = glm::min(bmin, v);
        bmax = glm::max(bmax, v);
    }
    glm::vec3 extents = bmax - bmin;

    // Sort axes by extent to get length > width > height
    float dims[3] = {extents.x, extents.y, extents.z};
    int axisOrder[3] = {0, 1, 2};
    // Sort descending by extent
    if (dims[axisOrder[0]] < dims[axisOrder[1]]) std::swap(axisOrder[0], axisOrder[1]);
    if (dims[axisOrder[0]] < dims[axisOrder[2]]) std::swap(axisOrder[0], axisOrder[2]);
    if (dims[axisOrder[1]] < dims[axisOrder[2]]) std::swap(axisOrder[1], axisOrder[2]);

    traits.length = dims[axisOrder[0]];
    traits.width  = dims[axisOrder[1]];
    traits.height = dims[axisOrder[2]];

    traits.aspectLW = (traits.width > 1e-7f)  ? traits.length / traits.width  : 1.0f;
    traits.aspectLH = (traits.height > 1e-7f) ? traits.length / traits.height : 1.0f;

    // Cross-section profile (10 slices along longest axis)
    int longestAxis = axisOrder[0];
    constexpr int NUM_SLICES = 10;
    traits.crossSectionProfile = computeCrossSectionProfile(
        vertices, longestAxis,
        bmin[longestAxis], bmax[longestAxis],
        NUM_SLICES);

    // Find base radius (max of profile)
    traits.baseRadius = 0.0f;
    for (float r : traits.crossSectionProfile) {
        if (r > traits.baseRadius) traits.baseRadius = r;
    }
    if (traits.baseRadius < 1e-7f) traits.baseRadius = 1.0f;

    // Normalize the profile to base radius
    std::vector<float> normalizedProfile(traits.crossSectionProfile.size());
    for (size_t i = 0; i < traits.crossSectionProfile.size(); ++i) {
        normalizedProfile[i] = traits.crossSectionProfile[i] / traits.baseRadius;
    }
    traits.crossSectionProfile = normalizedProfile;

    // Compute radius multipliers from the normalized profile
    traits.radiusMultipliers = profileToRadiusMultipliers(normalizedProfile);

    // Face topology — find dominant polygon side count
    std::map<int, int> faceSideCounts;
    for (const auto& f : faces) {
        faceSideCounts[static_cast<int>(f.size())]++;
    }
    int maxCount = 0;
    traits.dominantFaceSides = 3;
    for (const auto& pair : faceSideCounts) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            traits.dominantFaceSides = pair.first;
        }
    }

    // Infer faction and class
    traits.inferredFaction = inferFaction(traits.name);
    traits.inferredClass = inferClass(traits.name, traits.vertexCount);

    std::cout << "[ReferenceModelAnalyzer] Analyzed: " << traits.name
              << " (" << traits.vertexCount << " verts, " << traits.faceCount << " faces"
              << ", L:W=" << traits.aspectLW << ", L:H=" << traits.aspectLH
              << ", faction=" << traits.inferredFaction
              << ", class=" << traits.inferredClass << ")" << std::endl;

    m_traits.push_back(std::move(traits));
    return true;
}

// ────────────────────────────────────────────────────────────────────────
// Analyze directory of OBJ files
// ────────────────────────────────────────────────────────────────────────

int ReferenceModelAnalyzer::analyzeDirectory(const std::string& dirPath) {
    int count = 0;
    if (!std::filesystem::exists(dirPath)) {
        std::cerr << "[ReferenceModelAnalyzer] Directory does not exist: " << dirPath << std::endl;
        return 0;
    }

    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
        if (!entry.is_regular_file()) continue;
        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == ".obj") {
            if (analyzeOBJ(entry.path().string())) {
                count++;
            }
        }
    }
    return count;
}

// ────────────────────────────────────────────────────────────────────────
// Analyze archive (extract then scan)
// ────────────────────────────────────────────────────────────────────────

int ReferenceModelAnalyzer::analyzeArchive(const std::string& archivePath,
                                            const std::string& extractDir) {
    namespace fs = std::filesystem;

    if (!fs::exists(archivePath)) {
        std::cerr << "[ReferenceModelAnalyzer] Archive not found: " << archivePath << std::endl;
        return 0;
    }

    // Validate paths contain no shell metacharacters to prevent injection
    auto isPathSafe = [](const std::string& path) -> bool {
        for (char c : path) {
            if (c == ';' || c == '|' || c == '&' || c == '$' ||
                c == '`' || c == '\n' || c == '\r') {
                return false;
            }
        }
        return true;
    };

    if (!isPathSafe(archivePath) || !isPathSafe(extractDir)) {
        std::cerr << "[ReferenceModelAnalyzer] Invalid characters in path" << std::endl;
        return 0;
    }

    // Create extraction directory
    fs::create_directories(extractDir);

    // Determine extraction command based on extension
    std::string ext = fs::path(archivePath).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    // Use canonical paths to ensure we operate on real filesystem locations
    std::string safeArchive = fs::canonical(archivePath).string();
    std::string safeExtract = fs::canonical(extractDir).string();

    std::string cmd;
    if (ext == ".zip") {
        cmd = "unzip -o -q \"" + safeArchive + "\" -d \"" + safeExtract + "\" 2>/dev/null";
    } else if (ext == ".rar" || ext == ".7z") {
        cmd = "7z x \"" + safeArchive + "\" -o\"" + safeExtract + "\" -y -bso0 -bsp0 2>/dev/null";
    } else {
        std::cerr << "[ReferenceModelAnalyzer] Unsupported archive format: " << ext << std::endl;
        return 0;
    }

    std::cout << "[ReferenceModelAnalyzer] Extracting archive: " << archivePath << std::endl;
    int ret = std::system(cmd.c_str());
    if (ret != 0) {
        std::cerr << "[ReferenceModelAnalyzer] Extraction failed for: " << archivePath << std::endl;
        return 0;
    }

    // Recursively find all .obj files in extracted directory
    int count = 0;
    for (const auto& entry : fs::recursive_directory_iterator(extractDir)) {
        if (!entry.is_regular_file()) continue;
        std::string fext = entry.path().extension().string();
        std::transform(fext.begin(), fext.end(), fext.begin(), ::tolower);
        if (fext == ".obj") {
            if (analyzeOBJ(entry.path().string())) {
                count++;
            }
        }
    }

    std::cout << "[ReferenceModelAnalyzer] Analyzed " << count << " models from archive" << std::endl;
    return count;
}

// ────────────────────────────────────────────────────────────────────────
// Accessors
// ────────────────────────────────────────────────────────────────────────

const AnalyzedModelTraits& ReferenceModelAnalyzer::getModelTraits(size_t index) const {
    static AnalyzedModelTraits empty;
    if (index < m_traits.size()) {
        return m_traits[index];
    }
    return empty;
}

const std::vector<AnalyzedModelTraits>& ReferenceModelAnalyzer::getAllTraits() const {
    return m_traits;
}

size_t ReferenceModelAnalyzer::getModelCount() const {
    return m_traits.size();
}

// ────────────────────────────────────────────────────────────────────────
// Compute learned generation parameters
// ────────────────────────────────────────────────────────────────────────

LearnedGenerationParams ReferenceModelAnalyzer::computeLearnedParams() const {
    LearnedGenerationParams params;

    if (m_traits.empty()) return params;

    params.modelCount = static_cast<int>(m_traits.size());

    // Compute averages and ranges
    float sumLW = 0, sumLH = 0;
    float minLW = std::numeric_limits<float>::max();
    float maxLW = std::numeric_limits<float>::lowest();
    int sumVerts = 0, sumFaces = 0;
    float sumBaseR = 0;

    for (const auto& t : m_traits) {
        sumLW += t.aspectLW;
        sumLH += t.aspectLH;
        if (t.aspectLW < minLW) minLW = t.aspectLW;
        if (t.aspectLW > maxLW) maxLW = t.aspectLW;
        sumVerts += t.vertexCount;
        sumFaces += t.faceCount;
        sumBaseR += t.baseRadius;
    }

    float n = static_cast<float>(m_traits.size());
    params.avgAspectLW = sumLW / n;
    params.minAspectLW = minLW;
    params.maxAspectLW = maxLW;
    params.avgAspectLH = sumLH / n;
    params.avgVertexCount = static_cast<int>(sumVerts / n);
    params.avgFaceCount = static_cast<int>(sumFaces / n);
    params.avgBaseRadius = sumBaseR / n;

    // Blend cross-section profiles: resample each to the same slice count,
    // then average point-by-point
    constexpr int BLEND_SLICES = 10;
    params.blendedProfile.assign(BLEND_SLICES, 0.0f);

    for (const auto& t : m_traits) {
        if (t.crossSectionProfile.empty()) continue;
        int srcSize = static_cast<int>(t.crossSectionProfile.size());
        for (int i = 0; i < BLEND_SLICES; ++i) {
            // Map blend index to source index
            float srcIdx = static_cast<float>(i) * (srcSize - 1) / (BLEND_SLICES - 1);
            int lo = static_cast<int>(srcIdx);
            int hi = std::min(lo + 1, srcSize - 1);
            float frac = srcIdx - lo;
            float val = t.crossSectionProfile[lo] * (1.0f - frac)
                      + t.crossSectionProfile[hi] * frac;
            params.blendedProfile[i] += val;
        }
    }
    for (float& v : params.blendedProfile) {
        v /= n;
    }

    // Compute blended radius multipliers from blended profile
    params.blendedRadiusMultipliers = profileToRadiusMultipliers(params.blendedProfile);

    return params;
}

// ────────────────────────────────────────────────────────────────────────
// Convert to ReferenceModelTraits (for ShipGenerationRules)
// ────────────────────────────────────────────────────────────────────────

ReferenceModelTraits ReferenceModelAnalyzer::toReferenceModelTraits() const {
    auto params = computeLearnedParams();

    ReferenceModelTraits traits;
    traits.avgAspectLW = params.avgAspectLW;
    traits.minAspectLW = params.minAspectLW;
    traits.maxAspectLW = params.maxAspectLW;
    traits.avgAspectLH = params.avgAspectLH;
    traits.avgVertexCount = params.avgVertexCount;
    traits.avgFaceCount = params.avgFaceCount;

    // Compute detail density multiplier relative to frigate baseline (6214 verts)
    constexpr int FRIGATE_BASELINE_VERTS = 6214;
    traits.detailDensityMultiplier = static_cast<float>(params.avgVertexCount)
                                   / static_cast<float>(FRIGATE_BASELINE_VERTS);

    return traits;
}

// ────────────────────────────────────────────────────────────────────────
// Generate learned radius multipliers
// ────────────────────────────────────────────────────────────────────────

std::vector<float> ReferenceModelAnalyzer::generateLearnedRadiusMultipliers(
    int segments, unsigned int seed) const
{
    auto params = computeLearnedParams();

    if (params.blendedProfile.empty() || segments < 1) {
        return std::vector<float>(segments, 1.0f);
    }

    // Resample the blended profile to the requested segment count + 1
    // (one value per ring, segments + 1 rings)
    int profileSize = static_cast<int>(params.blendedProfile.size());
    std::vector<float> resampledProfile(segments + 1);
    for (int i = 0; i <= segments; ++i) {
        float srcIdx = static_cast<float>(i) * (profileSize - 1) / segments;
        int lo = static_cast<int>(srcIdx);
        int hi = std::min(lo + 1, profileSize - 1);
        float frac = srcIdx - lo;
        resampledProfile[i] = params.blendedProfile[lo] * (1.0f - frac)
                            + params.blendedProfile[hi] * frac;
    }

    // Convert to multipliers (ratio between consecutive slices)
    std::vector<float> multipliers(segments);
    std::mt19937 rng(seed != 0 ? seed : 42u);
    std::uniform_real_distribution<float> jitter(-0.05f, 0.05f);

    for (int i = 0; i < segments; ++i) {
        float prev = resampledProfile[i];
        float curr = resampledProfile[i + 1];
        if (prev < 1e-7f) prev = 1e-7f;
        float mult = curr / prev;
        // Add slight jitter for variation
        mult *= (1.0f + jitter(rng));
        multipliers[i] = std::max(mult, 0.1f);
    }

    return multipliers;
}

} // namespace eve
