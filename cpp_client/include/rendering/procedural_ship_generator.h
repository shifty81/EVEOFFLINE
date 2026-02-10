#pragma once

#include <string>
#include <vector>
#include <memory>
#include <random>
#include <glm/glm.hpp>
#include "rendering/mesh.h"
#include "rendering/procedural_mesh_ops.h"

namespace eve {

class Model;

/**
 * Parsed OBJ mesh data used as a seed for procedural generation.
 * Stores raw geometry in a format suitable for modification.
 */
struct OBJSeedMesh {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<unsigned int> indices;

    // Bounding box computed from positions
    glm::vec3 bbMin{0.0f};
    glm::vec3 bbMax{0.0f};

    // Derived dimensions
    float length() const { return bbMax.z - bbMin.z; }
    float width()  const { return bbMax.x - bbMin.x; }
    float height() const { return bbMax.y - bbMin.y; }

    /** Recompute bounding box from current positions. */
    void computeBounds();

    /** Centre the mesh at the origin. */
    void centreAtOrigin();

    /** Uniformly scale so the longest axis equals @p targetLength. */
    void normalizeScale(float targetLength);

    bool empty() const { return positions.empty(); }
};

/**
 * Mount point on a seed mesh where components can be attached.
 * Identified automatically from mesh topology or manually defined.
 */
struct MountPoint {
    glm::vec3 position;
    glm::vec3 normal;       // Outward direction
    float     radius;       // Available attachment area
    std::string category;   // "weapon", "engine", "antenna", "wing"
};

/**
 * Parameters controlling procedural ship generation from a seed OBJ mesh.
 * A deterministic seed ensures the same parameters always produce identical output.
 */
struct ProceduralShipParams {
    unsigned int seed = 0;          // 0 = random

    // Hull modification
    float lengthScale    = 1.0f;    // Stretch/compress along Z
    float widthScale     = 1.0f;    // Stretch/compress along X
    float heightScale    = 1.0f;    // Stretch/compress along Y

    // Extrusion detail — conservative defaults for clean small-ship silhouettes
    int   extrusionCount = 2;       // Number of faces to extrude for greebles
    float extrusionDepth = 0.10f;   // Maximum extrusion distance (fraction of size)

    // Noise displacement
    float noiseAmplitude = 0.0f;    // Surface noise strength (0 = off)
    float noiseFrequency = 4.0f;    // Noise detail frequency

    // Symmetry
    bool  enforceSymmetry = true;   // Mirror modifications across X axis

    // Module attachment
    int   engineCount     = 2;      // Number of engines to attach at rear
    int   weaponCount     = 2;      // Number of weapon turrets
    int   antennaCount    = 0;      // Number of antenna arrays

    // Colour override (zero = keep OBJ material colours)
    glm::vec3 primaryColor{0.0f};
    glm::vec3 accentColor{0.0f};
};

/**
 * Reference asset configuration.
 * Defines where seed OBJ models and textures are located.
 */
struct ReferenceAssetConfig {
    std::string objArchivePath;       // Path to OBJ archive (e.g. 99-intergalactic_spaceship-obj.rar)
    std::string textureArchivePath;   // Path to texture archive (e.g. 24-textures.zip)
    std::string extractedObjDir;      // Directory where extracted OBJ files reside
    std::string extractedTextureDir;  // Directory where extracted textures reside
};

// ─────────────────────────────────────────────────────────────────────
// ProceduralShipGenerator
// ─────────────────────────────────────────────────────────────────────

/**
 * Generates unique spaceship models by loading a base OBJ mesh and
 * applying seeded procedural modifications.
 *
 * Pipeline:
 *   1. Parse OBJ file into OBJSeedMesh
 *   2. Normalize and centre the mesh
 *   3. Detect mount points for engines, weapons, etc.
 *   4. Apply hull scaling and proportional adjustments
 *   5. Extrude selected faces for surface detail (greebles)
 *   6. Optionally apply noise displacement for organic variation
 *   7. Attach procedural engine/weapon/antenna modules at mount points
 *   8. Enforce symmetry by mirroring modifications across the X axis
 *   9. Recompute normals and output as eve::Model
 *
 * The same seed + OBJ file always produces the same ship, making the
 * system suitable for networked games where clients must agree on
 * ship appearance from a compact seed value.
 */
class ProceduralShipGenerator {
public:
    ProceduralShipGenerator();
    ~ProceduralShipGenerator();

    /**
     * Configure reference asset paths.
     * The generator will look in these directories for seed OBJ files
     * and texture files used during generation.
     */
    void setReferenceAssets(const ReferenceAssetConfig& config);

    /**
     * Get the current reference asset configuration.
     */
    const ReferenceAssetConfig& getReferenceAssets() const { return m_assetConfig; }

    // ── OBJ parsing ─────────────────────────────────────────────────

    /**
     * Parse a Wavefront OBJ file into an OBJSeedMesh.
     * Uses tinyobjloader internally.
     *
     * @param path  Absolute or relative path to the .obj file.
     * @return      Populated seed mesh, or empty mesh on failure.
     */
    static OBJSeedMesh parseOBJ(const std::string& path);

    // ── Mount point detection ───────────────────────────────────────

    /**
     * Detect mount points on a seed mesh by analysing geometry.
     * Engine mounts are placed at the rear (min-Z), weapon mounts on
     * the upper hull, antenna mounts at extremes.
     */
    static std::vector<MountPoint> detectMountPoints(const OBJSeedMesh& seed);

    // ── Procedural modification ─────────────────────────────────────

    /**
     * Apply hull scaling to a seed mesh.
     */
    static void applyHullScaling(OBJSeedMesh& mesh,
                                 float lengthScale, float widthScale, float heightScale);

    /**
     * Extrude selected faces on the mesh surface to create greeble detail.
     * Faces are selected based on normal direction and area, then extruded
     * along their normals by a distance derived from @p depth.
     *
     * @param mesh   Seed mesh to modify in place.
     * @param count  Number of extrusion operations.
     * @param depth  Maximum depth as a fraction of bounding box diagonal.
     * @param rng    Random engine for face selection.
     */
    static void applyExtrusions(OBJSeedMesh& mesh, int count, float depth,
                                std::mt19937& rng);

    /**
     * Apply simplex-style noise displacement to mesh vertices.
     * Vertices are displaced along their normals by a noise value.
     */
    static void applyNoiseDisplacement(OBJSeedMesh& mesh,
                                       float amplitude, float frequency,
                                       std::mt19937& rng);

    /**
     * Enforce bilateral symmetry across the X axis.
     * For each vertex at +X, its mirror at -X receives the same
     * procedural displacement (averaged if both already displaced).
     */
    static void enforceSymmetry(OBJSeedMesh& mesh);

    /**
     * Recompute smooth normals for the mesh based on face adjacency.
     */
    static void recomputeNormals(OBJSeedMesh& mesh);

    // ── Module generation ───────────────────────────────────────────

    /**
     * Generate an engine module (cylindrical thruster) at a mount point.
     */
    static OBJSeedMesh generateEngineModule(const MountPoint& mount,
                                            float size, std::mt19937& rng);

    /**
     * Generate a weapon turret module at a mount point.
     */
    static OBJSeedMesh generateWeaponModule(const MountPoint& mount,
                                            float size, std::mt19937& rng);

    /**
     * Generate an antenna array module at a mount point.
     */
    static OBJSeedMesh generateAntennaModule(const MountPoint& mount,
                                             float size, std::mt19937& rng);

    // ── Full pipeline ───────────────────────────────────────────────

    /**
     * Run the complete procedural generation pipeline.
     *
     * @param seedMesh  Base OBJ mesh to modify.
     * @param params    Generation parameters (seed, scales, counts, …).
     * @return          New Model ready for rendering, or nullptr on failure.
     */
    std::unique_ptr<Model> generate(const OBJSeedMesh& seedMesh,
                                    const ProceduralShipParams& params) const;

    /**
     * Convenience: load an OBJ file and run the full pipeline.
     *
     * @param objPath  Path to OBJ seed file.
     * @param params   Generation parameters.
     * @return         New Model, or nullptr on failure.
     */
    std::unique_ptr<Model> generateFromFile(const std::string& objPath,
                                            const ProceduralShipParams& params) const;

    /**
     * Find a suitable seed OBJ from the reference asset directory,
     * matching faction and ship class where possible.
     *
     * @param faction   Faction name (e.g. "Caldari").
     * @param shipClass Ship class (e.g. "frigate", "cruiser").
     * @return          Path to OBJ file, or empty string if none found.
     */
    std::string findSeedOBJ(const std::string& faction,
                            const std::string& shipClass) const;

    /**
     * Export a generated mesh back to OBJ format for caching or
     * external tool use.
     *
     * @param mesh     Seed mesh data to export.
     * @param path     Output file path (.obj).
     * @return         true on success.
     */
    static bool exportOBJ(const OBJSeedMesh& mesh, const std::string& path);

    // ── Texture helpers ─────────────────────────────────────────────

    /**
     * List available texture files from the reference texture directory.
     */
    std::vector<std::string> listAvailableTextures() const;

    /**
     * Find a texture matching a given material name or keyword.
     * Searches the 24-textures reference pack.
     */
    std::string findTexture(const std::string& keyword) const;

private:
    ReferenceAssetConfig m_assetConfig;

    /** Merge a module mesh into a target mesh. */
    static void mergeInto(OBJSeedMesh& target, const OBJSeedMesh& module);

    /** Simple hash-based 3D noise for displacement. */
    static float noise3D(float x, float y, float z);
};

} // namespace eve
