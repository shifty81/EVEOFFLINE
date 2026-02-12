#pragma once

#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include "rendering/mesh.h"

namespace atlas {

/**
 * Procedural mesh generation operations inspired by extrusion-based
 * polygon manipulation techniques.
 *
 * Core workflow:
 *   1. Generate an N-sided polygonal face
 *   2. Extrude the face along its normal to create new geometry
 *   3. Stitch adjacent faces to form closed surfaces
 *   4. Apply detail operations (bevel cuts, subdivisions, pyramidize)
 *   5. Repeat on any resulting face for recursive detail
 *
 * Reference: AlexSanfilippo/ProceduralMeshGeneration (GPL-3.0)
 */

// ─────────────────────────────────────────────────────────────────────
// Data structures
// ─────────────────────────────────────────────────────────────────────

/**
 * A polygonal face defined by ordered outer vertices (positions only).
 * Used as the unit of work for extrusion, stitching and detail operations.
 */
struct PolyFace {
    std::vector<glm::vec3> outerVertices;   ///< Ordered ring of positions
    glm::vec3 normal{0.0f, 1.0f, 0.0f};    ///< Face normal

    /** Number of sides (== outerVertices.size()). */
    int sides() const { return static_cast<int>(outerVertices.size()); }

    /** Recompute the normal from the first three outer vertices. */
    void recalculateNormal();

    /** Compute the centroid of outer vertices. */
    glm::vec3 centroid() const;
};

/**
 * Result of triangulating a set of PolyFaces into renderable geometry
 * compatible with atlas::Mesh / atlas::ShipPart.
 */
struct TriangulatedMesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

// ─────────────────────────────────────────────────────────────────────
// Polygon generation
// ─────────────────────────────────────────────────────────────────────

/**
 * Generate a regular N-sided polygon face centred at @p centre.
 *
 * @param sides     Number of sides (>= 3).
 * @param radius    Circumradius.
 * @param centre    Centre position.
 * @param normal    Direction the face will point towards.
 * @param scaleX    Optional non-uniform X scale for elliptical shapes.
 * @param scaleZ    Optional non-uniform Z scale.
 * @return          A PolyFace with populated outerVertices and normal.
 */
PolyFace generatePolygonFace(int sides, float radius,
                             const glm::vec3& centre = glm::vec3(0.0f),
                             const glm::vec3& normal = glm::vec3(0.0f, 1.0f, 0.0f),
                             float scaleX = 1.0f, float scaleZ = 1.0f);

/**
 * Generate an irregular N-sided polygon where each vertex has its own radius.
 *
 * @param radii  Per-vertex radii (size must equal @p sides).
 */
PolyFace generateIrregularPolygonFace(int sides,
                                      const std::vector<float>& radii,
                                      const glm::vec3& centre = glm::vec3(0.0f),
                                      const glm::vec3& normal = glm::vec3(0.0f, 1.0f, 0.0f),
                                      float scaleX = 1.0f, float scaleZ = 1.0f);

// ─────────────────────────────────────────────────────────────────────
// Face extrusion
// ─────────────────────────────────────────────────────────────────────

/**
 * Extrude a face along its normal (or a custom direction) producing a
 * new face offset by @p distance and optionally scaled.
 *
 * @param source    The face to extrude from.
 * @param distance  How far to extrude along @p direction.
 * @param scale     Uniform scale applied to the new face relative to its centroid.
 * @param direction Override extrusion direction (defaults to source normal).
 * @return          The newly created face.
 */
PolyFace extrudeFace(const PolyFace& source, float distance,
                     float scale = 1.0f,
                     const glm::vec3& direction = glm::vec3(0.0f));

// ─────────────────────────────────────────────────────────────────────
// Face stitching
// ─────────────────────────────────────────────────────────────────────

/**
 * Connect two faces that have the same number of sides, creating quad
 * faces for each pair of edges.  The resulting quads form the "walls"
 * between two polygonal caps.
 *
 * @return  Vector of quad PolyFaces (size == source.sides()).
 */
std::vector<PolyFace> stitchFaces(const PolyFace& faceA, const PolyFace& faceB);

// ─────────────────────────────────────────────────────────────────────
// Detail operations
// ─────────────────────────────────────────────────────────────────────

/**
 * Perform a bevel cut on a face: inset the face border and push the
 * inner face inward/outward, producing a recessed/protruding panel.
 *
 * @param face          The face to bevel.
 * @param borderSize    Fraction [0,1] of edge length kept as border.
 * @param depth         Signed distance to push the inner face along normal.
 * @return  Vector of new faces (border quads + inner face).
 */
std::vector<PolyFace> bevelCutFace(const PolyFace& face,
                                   float borderSize, float depth);

/**
 * Replace a face with a pyramid — connect every edge to a central
 * apex offset from the face centroid along the normal.
 *
 * @param face    The face to pyramidize.
 * @param height  Distance of the apex above the face.
 * @return        Vector of triangular faces forming the pyramid.
 */
std::vector<PolyFace> pyramidizeFace(const PolyFace& face, float height);

/**
 * Subdivide a quad face lengthwise into @p count equal strips.
 *
 * @param face    A quad face (4 outer vertices).
 * @param count   Number of strips (>= 2).
 * @return        Vector of quad sub-faces.
 */
std::vector<PolyFace> subdivideFaceLengthwise(const PolyFace& face, int count);

// ─────────────────────────────────────────────────────────────────────
// Bezier helpers
// ─────────────────────────────────────────────────────────────────────

/**
 * Evaluate a linear Bezier (lerp) between two points.
 */
glm::vec3 bezierLinear(const glm::vec3& a, const glm::vec3& b, float t);

/**
 * Evaluate a quadratic Bezier curve at parameter t.
 */
glm::vec3 bezierQuadratic(const glm::vec3& a, const glm::vec3& b,
                          const glm::vec3& c, float t);

/**
 * Evaluate a cubic Bezier curve at parameter t.
 */
glm::vec3 bezierCubic(const glm::vec3& a, const glm::vec3& b,
                      const glm::vec3& c, const glm::vec3& d, float t);

/**
 * Sample a quadratic Bezier curve at uniform intervals, returning
 * @p intervals+1 points along the curve.
 */
std::vector<glm::vec3> sampleBezierQuadratic(const glm::vec3& a,
                                             const glm::vec3& b,
                                             const glm::vec3& c,
                                             int intervals);

// ─────────────────────────────────────────────────────────────────────
// Segmented extrusion (spaceship hull builder)
// ─────────────────────────────────────────────────────────────────────

/**
 * Build a segmented hull by repeatedly extruding a starting polygon face
 * along the forward axis, varying the radius at each segment.
 *
 * This is the core spaceship hull algorithm ported from the reference
 * project.  The caller supplies per-segment radius multipliers (or
 * a seed to generate them randomly).
 *
 * @param sides             Number of sides for the cross-section polygon.
 * @param segments          Number of extrusion steps.
 * @param segmentLength     Length of each segment.
 * @param baseRadius        Starting radius.
 * @param radiusMultipliers Per-segment radius scale factors (size == segments).
 *                          If empty, all segments keep the base radius.
 * @param scaleX            Non-uniform X scale on the cross-section.
 * @param scaleZ            Non-uniform Z scale on the cross-section.
 * @param color             Vertex colour for the hull.
 * @return  TriangulatedMesh ready for atlas::Mesh construction.
 */
TriangulatedMesh buildSegmentedHull(int sides, int segments,
                                    float segmentLength, float baseRadius,
                                    const std::vector<float>& radiusMultipliers,
                                    float scaleX, float scaleZ,
                                    const glm::vec3& color);

/**
 * Generate random radius multipliers for a segmented hull using a seed.
 *
 * @param segments  Number of segments.
 * @param baseRadius Starting radius.
 * @param seed      Random seed (0 = use default sequence).
 * @return          Vector of per-segment multipliers.
 */
std::vector<float> generateRadiusMultipliers(int segments, float baseRadius,
                                             unsigned int seed);

// ─────────────────────────────────────────────────────────────────────
// Triangulation / conversion
// ─────────────────────────────────────────────────────────────────────

/**
 * Triangulate a single PolyFace using a fan from vertex 0.
 *
 * @param face  The polygonal face.
 * @param color Vertex colour.
 * @return      Vertices and indices ready for rendering.
 */
TriangulatedMesh triangulateFace(const PolyFace& face, const glm::vec3& color);

/**
 * Triangulate a collection of PolyFaces into a single mesh.
 */
TriangulatedMesh triangulateFaces(const std::vector<PolyFace>& faces,
                                  const glm::vec3& color);

} // namespace atlas
