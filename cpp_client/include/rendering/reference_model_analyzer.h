#pragma once

#include <string>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "rendering/ship_generation_rules.h"

namespace eve {

/**
 * Geometric traits extracted from a single reference OBJ model.
 * These measurements describe the shape, proportions, and complexity
 * of a real ship model so the procedural engine can mimic it.
 */
struct AnalyzedModelTraits {
    std::string name;                   ///< Source file name
    std::string inferredFaction;        ///< Inferred faction (or "Unknown")
    std::string inferredClass;          ///< Inferred ship class (or "Unknown")

    // Bounding-box dimensions (along longest, middle, shortest axes)
    float length;                       ///< Longest axis extent
    float width;                        ///< Middle axis extent
    float height;                       ///< Shortest axis extent

    // Aspect ratios
    float aspectLW;                     ///< Length-to-width ratio
    float aspectLH;                     ///< Length-to-height ratio

    // Complexity
    int vertexCount;
    int faceCount;

    // Cross-section profile along the longest axis (normalized 0..1)
    // Each entry is the max radius at that slice position
    std::vector<float> crossSectionProfile;

    // Per-segment radius multipliers derived from the cross-section
    // These can be fed directly into buildSegmentedHull()
    std::vector<float> radiusMultipliers;

    // Base radius (average cross-section radius at the widest point)
    float baseRadius;

    // Dominant face topology (most common polygon side count)
    int dominantFaceSides;

    AnalyzedModelTraits()
        : length(0), width(0), height(0)
        , aspectLW(1.0f), aspectLH(1.0f)
        , vertexCount(0), faceCount(0)
        , baseRadius(1.0f)
        , dominantFaceSides(3)
    {}
};

/**
 * Aggregate traits computed from multiple analyzed models.
 * Used to define the "learned" generation parameters for the procedural engine.
 */
struct LearnedGenerationParams {
    // Averaged aspect ratios with min/max bounds
    float avgAspectLW;
    float minAspectLW;
    float maxAspectLW;
    float avgAspectLH;

    // Averaged complexity
    int avgVertexCount;
    int avgFaceCount;

    // Blended cross-section profile (averaged from all analyzed models)
    std::vector<float> blendedProfile;

    // Blended radius multipliers for buildSegmentedHull()
    std::vector<float> blendedRadiusMultipliers;

    // Average base radius
    float avgBaseRadius;

    // Number of models used to compute these params
    int modelCount;

    LearnedGenerationParams()
        : avgAspectLW(2.0f), minAspectLW(1.0f), maxAspectLW(5.0f)
        , avgAspectLH(3.0f)
        , avgVertexCount(8000), avgFaceCount(8000)
        , avgBaseRadius(1.0f)
        , modelCount(0)
    {}
};

/**
 * Analyzes reference OBJ models to extract geometric traits that inform
 * the procedural ship generation engine.
 *
 * The analyzer reads raw OBJ vertex/face data (without requiring OpenGL),
 * computes bounding boxes, cross-section profiles, and radius multiplier
 * patterns, then produces LearnedGenerationParams that can be used by
 * ShipPartLibrary and ShipGenerationRules.
 *
 * Usage:
 *   ReferenceModelAnalyzer analyzer;
 *   analyzer.analyzeOBJ("path/to/model.obj");
 *   analyzer.analyzeOBJ("path/to/another.obj");
 *   auto params = analyzer.computeLearnedParams();
 *   // params.blendedRadiusMultipliers feeds into buildSegmentedHull()
 */
class ReferenceModelAnalyzer {
public:
    ReferenceModelAnalyzer();
    ~ReferenceModelAnalyzer();

    /**
     * Analyze a single OBJ file and store its traits.
     * Uses lightweight vertex/face parsing (no OpenGL required).
     * @param objPath Path to the .obj file
     * @return true if analysis succeeded
     */
    bool analyzeOBJ(const std::string& objPath);

    /**
     * Analyze all OBJ files found in a directory (non-recursive).
     * @param dirPath Directory containing .obj files
     * @return Number of models successfully analyzed
     */
    int analyzeDirectory(const std::string& dirPath);

    /**
     * Extract OBJ files from supported archives (.zip, .rar, .7z) into
     * a temporary directory and analyze them.
     * @param archivePath Path to the archive file
     * @param extractDir Directory to extract files into
     * @return Number of models successfully analyzed
     */
    int analyzeArchive(const std::string& archivePath,
                       const std::string& extractDir);

    /**
     * Get traits for a specific analyzed model by index.
     */
    const AnalyzedModelTraits& getModelTraits(size_t index) const;

    /**
     * Get all analyzed model traits.
     */
    const std::vector<AnalyzedModelTraits>& getAllTraits() const;

    /**
     * Get the number of models analyzed so far.
     */
    size_t getModelCount() const;

    /**
     * Compute blended/learned generation parameters from all analyzed models.
     * This produces the aggregate parameters that the procedural engine uses.
     */
    LearnedGenerationParams computeLearnedParams() const;

    /**
     * Populate a ReferenceModelTraits struct (used by ShipGenerationRules)
     * from the learned parameters.
     */
    ReferenceModelTraits toReferenceModelTraits() const;

    /**
     * Generate radius multipliers suitable for buildSegmentedHull(),
     * learned from the analyzed models' cross-section profiles.
     * @param segments Number of segments desired
     * @param seed Random seed for variation within learned bounds
     * @return Per-segment radius multipliers
     */
    std::vector<float> generateLearnedRadiusMultipliers(int segments,
                                                         unsigned int seed = 0) const;

private:
    std::vector<AnalyzedModelTraits> m_traits;

    /**
     * Parse raw vertex and face data from an OBJ file.
     * Lightweight parser that reads only v and f lines.
     */
    static bool parseOBJGeometry(const std::string& path,
                                  std::vector<glm::vec3>& outVertices,
                                  std::vector<std::vector<int>>& outFaces);

    /**
     * Compute cross-section profile along the longest axis.
     * Divides the model into slices and measures the max radius at each.
     */
    static std::vector<float> computeCrossSectionProfile(
        const std::vector<glm::vec3>& vertices,
        int longestAxis, float axisMin, float axisMax,
        int numSlices);

    /**
     * Convert a cross-section profile into radius multipliers
     * suitable for buildSegmentedHull().
     */
    static std::vector<float> profileToRadiusMultipliers(
        const std::vector<float>& profile);

    /**
     * Infer faction name from file name (heuristic).
     */
    static std::string inferFaction(const std::string& filename);

    /**
     * Infer ship class from file name or geometry (heuristic).
     */
    static std::string inferClass(const std::string& filename, int vertexCount);
};

} // namespace eve
