#include "rendering/procedural_mesh_ops.h"
#include <random>
#include <stdexcept>

namespace eve {

// ────────────────────────────────────────────────────────────────────────
// Mathematical constants
// ────────────────────────────────────────────────────────────────────────
static constexpr float kPI = 3.14159265358979323846f;

// ────────────────────────────────────────────────────────────────────────
// PolyFace helpers
// ────────────────────────────────────────────────────────────────────────

void PolyFace::recalculateNormal() {
    if (outerVertices.size() < 3) return;
    glm::vec3 edge1 = outerVertices[1] - outerVertices[0];
    glm::vec3 edge2 = outerVertices[2] - outerVertices[0];
    glm::vec3 n = glm::cross(edge1, edge2);
    float len = glm::length(n);
    normal = (len > 1e-7f) ? n / len : glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 PolyFace::centroid() const {
    glm::vec3 c(0.0f);
    if (outerVertices.empty()) return c;
    for (const auto& v : outerVertices) c += v;
    return c / static_cast<float>(outerVertices.size());
}

// ────────────────────────────────────────────────────────────────────────
// Polygon generation
// ────────────────────────────────────────────────────────────────────────

PolyFace generatePolygonFace(int sides, float radius,
                             const glm::vec3& centre,
                             const glm::vec3& normal,
                             float scaleX, float scaleZ) {
    if (sides < 3) sides = 3;

    PolyFace face;
    face.normal = glm::normalize(normal);

    float angleStep = 2.0f * kPI / static_cast<float>(sides);
    float initialAngle = kPI / static_cast<float>(sides);

    // Build a local coordinate frame on the polygon plane.
    // Choose an arbitrary 'up' that isn't parallel to normal.
    glm::vec3 up = (std::abs(glm::dot(face.normal, glm::vec3(0, 1, 0))) < 0.99f)
                   ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    glm::vec3 tangent  = glm::normalize(glm::cross(up, face.normal));
    glm::vec3 binormal = glm::cross(face.normal, tangent);

    face.outerVertices.reserve(sides);
    for (int i = 0; i < sides; ++i) {
        float angle = initialAngle + angleStep * static_cast<float>(-i - 1);
        float x = std::cos(angle) * radius * scaleX;
        float z = std::sin(angle) * radius * scaleZ;
        face.outerVertices.push_back(centre + tangent * x + binormal * z);
    }
    return face;
}

PolyFace generateIrregularPolygonFace(int sides,
                                      const std::vector<float>& radii,
                                      const glm::vec3& centre,
                                      const glm::vec3& normal,
                                      float scaleX, float scaleZ) {
    if (sides < 3) sides = 3;

    PolyFace face;
    face.normal = glm::normalize(normal);

    float angleStep = 2.0f * kPI / static_cast<float>(sides);
    float initialAngle = kPI / static_cast<float>(sides);

    glm::vec3 up = (std::abs(glm::dot(face.normal, glm::vec3(0, 1, 0))) < 0.99f)
                   ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    glm::vec3 tangent  = glm::normalize(glm::cross(up, face.normal));
    glm::vec3 binormal = glm::cross(face.normal, tangent);

    face.outerVertices.reserve(sides);
    for (int i = 0; i < sides; ++i) {
        float r = (i < static_cast<int>(radii.size())) ? radii[i] : 1.0f;
        float angle = initialAngle + angleStep * static_cast<float>(-i - 1);
        float x = std::cos(angle) * r * scaleX;
        float z = std::sin(angle) * r * scaleZ;
        face.outerVertices.push_back(centre + tangent * x + binormal * z);
    }
    face.recalculateNormal();
    return face;
}

// ────────────────────────────────────────────────────────────────────────
// Face extrusion
// ────────────────────────────────────────────────────────────────────────

PolyFace extrudeFace(const PolyFace& source, float distance,
                     float scale, const glm::vec3& direction) {
    glm::vec3 dir = (glm::length(direction) > 1e-7f)
                    ? glm::normalize(direction)
                    : source.normal;
    glm::vec3 offset = dir * distance;
    glm::vec3 cen = source.centroid();

    PolyFace extruded;
    extruded.outerVertices.reserve(source.outerVertices.size());
    for (const auto& v : source.outerVertices) {
        // Scale relative to centroid, then translate
        glm::vec3 scaled = cen + (v - cen) * scale;
        extruded.outerVertices.push_back(scaled + offset);
    }
    extruded.recalculateNormal();
    return extruded;
}

// ────────────────────────────────────────────────────────────────────────
// Face stitching
// ────────────────────────────────────────────────────────────────────────

std::vector<PolyFace> stitchFaces(const PolyFace& faceA, const PolyFace& faceB) {
    int n = faceA.sides();
    if (n != faceB.sides() || n < 3) return {};

    std::vector<PolyFace> quads;
    quads.reserve(n);
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        PolyFace quad;
        quad.outerVertices = {
            faceB.outerVertices[i],
            faceA.outerVertices[i],
            faceA.outerVertices[j],
            faceB.outerVertices[j],
        };
        quad.recalculateNormal();
        quads.push_back(std::move(quad));
    }
    return quads;
}

// ────────────────────────────────────────────────────────────────────────
// Bevel cut
// ────────────────────────────────────────────────────────────────────────

std::vector<PolyFace> bevelCutFace(const PolyFace& face,
                                   float borderSize, float depth) {
    if (face.sides() < 3) return {face};

    glm::vec3 cen = face.centroid();

    // Build the inner (inset) face
    PolyFace inner;
    inner.outerVertices.reserve(face.outerVertices.size());
    float insetFactor = 1.0f - std::clamp(borderSize, 0.0f, 1.0f);
    for (const auto& v : face.outerVertices) {
        inner.outerVertices.push_back(cen + (v - cen) * insetFactor);
    }
    // Push inner face along normal
    for (auto& v : inner.outerVertices) {
        v += face.normal * depth;
    }
    inner.recalculateNormal();

    // Build border quads between outer and inner rings
    int n = face.sides();
    std::vector<PolyFace> result;
    result.reserve(n + 1);
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        PolyFace quad;
        quad.outerVertices = {
            face.outerVertices[i],
            face.outerVertices[j],
            inner.outerVertices[j],
            inner.outerVertices[i],
        };
        quad.recalculateNormal();
        result.push_back(std::move(quad));
    }
    result.push_back(inner);
    return result;
}

// ────────────────────────────────────────────────────────────────────────
// Pyramidize
// ────────────────────────────────────────────────────────────────────────

std::vector<PolyFace> pyramidizeFace(const PolyFace& face, float height) {
    if (face.sides() < 3) return {face};

    glm::vec3 apex = face.centroid() + face.normal * height;
    int n = face.sides();
    std::vector<PolyFace> tris;
    tris.reserve(n);
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        PolyFace tri;
        tri.outerVertices = {
            face.outerVertices[i],
            face.outerVertices[j],
            apex,
        };
        tri.recalculateNormal();
        tris.push_back(std::move(tri));
    }
    return tris;
}

// ────────────────────────────────────────────────────────────────────────
// Subdivide face lengthwise (quads only)
// ────────────────────────────────────────────────────────────────────────

std::vector<PolyFace> subdivideFaceLengthwise(const PolyFace& face, int count) {
    if (face.sides() != 4 || count < 2) return {face};

    const auto& v = face.outerVertices;
    // v[0]─v[3] is "top" edge, v[1]─v[2] is "bottom" edge
    std::vector<PolyFace> strips;
    strips.reserve(count);
    for (int i = 0; i < count; ++i) {
        float t0 = static_cast<float>(i)     / static_cast<float>(count);
        float t1 = static_cast<float>(i + 1) / static_cast<float>(count);
        PolyFace strip;
        strip.outerVertices = {
            glm::mix(v[0], v[3], t0),
            glm::mix(v[1], v[2], t0),
            glm::mix(v[1], v[2], t1),
            glm::mix(v[0], v[3], t1),
        };
        strip.recalculateNormal();
        strips.push_back(std::move(strip));
    }
    return strips;
}

// ────────────────────────────────────────────────────────────────────────
// Bezier helpers
// ────────────────────────────────────────────────────────────────────────

glm::vec3 bezierLinear(const glm::vec3& a, const glm::vec3& b, float t) {
    return glm::mix(a, b, t);
}

glm::vec3 bezierQuadratic(const glm::vec3& a, const glm::vec3& b,
                          const glm::vec3& c, float t) {
    glm::vec3 ab = glm::mix(a, b, t);
    glm::vec3 bc = glm::mix(b, c, t);
    return glm::mix(ab, bc, t);
}

glm::vec3 bezierCubic(const glm::vec3& a, const glm::vec3& b,
                      const glm::vec3& c, const glm::vec3& d, float t) {
    glm::vec3 ab  = glm::mix(a, b, t);
    glm::vec3 bc  = glm::mix(b, c, t);
    glm::vec3 cd  = glm::mix(c, d, t);
    glm::vec3 abc = glm::mix(ab, bc, t);
    glm::vec3 bcd = glm::mix(bc, cd, t);
    return glm::mix(abc, bcd, t);
}

std::vector<glm::vec3> sampleBezierQuadratic(const glm::vec3& a,
                                             const glm::vec3& b,
                                             const glm::vec3& c,
                                             int intervals) {
    if (intervals < 1) intervals = 1;
    std::vector<glm::vec3> points;
    points.reserve(intervals + 1);
    for (int i = 0; i <= intervals; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(intervals);
        points.push_back(bezierQuadratic(a, b, c, t));
    }
    return points;
}

// ────────────────────────────────────────────────────────────────────────
// Triangulation
// ────────────────────────────────────────────────────────────────────────

TriangulatedMesh triangulateFace(const PolyFace& face, const glm::vec3& color) {
    TriangulatedMesh mesh;
    int n = face.sides();
    if (n < 3) return mesh;

    // Compute a shared face normal
    glm::vec3 norm = face.normal;

    unsigned int baseIdx = 0;
    mesh.vertices.reserve(n);
    for (int i = 0; i < n; ++i) {
        Vertex v;
        v.position = face.outerVertices[i];
        v.normal   = norm;
        v.texCoords = glm::vec2(0.0f); // basic UV
        v.color    = color;
        mesh.vertices.push_back(v);
    }

    // Fan triangulation from vertex 0
    mesh.indices.reserve((n - 2) * 3);
    for (int i = 1; i < n - 1; ++i) {
        mesh.indices.push_back(baseIdx);
        mesh.indices.push_back(baseIdx + i);
        mesh.indices.push_back(baseIdx + i + 1);
    }
    return mesh;
}

TriangulatedMesh triangulateFaces(const std::vector<PolyFace>& faces,
                                  const glm::vec3& color) {
    TriangulatedMesh combined;
    for (const auto& face : faces) {
        TriangulatedMesh faceMesh = triangulateFace(face, color);
        unsigned int offset = static_cast<unsigned int>(combined.vertices.size());
        combined.vertices.insert(combined.vertices.end(),
                                 faceMesh.vertices.begin(),
                                 faceMesh.vertices.end());
        for (auto idx : faceMesh.indices) {
            combined.indices.push_back(offset + idx);
        }
    }
    return combined;
}

// ────────────────────────────────────────────────────────────────────────
// Radius multiplier generation
// ────────────────────────────────────────────────────────────────────────

std::vector<float> generateRadiusMultipliers(int segments, float baseRadius,
                                             unsigned int seed) {
    std::mt19937 rng(seed != 0 ? seed : 42u);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    float radiusMax = baseRadius * (0.1029f * segments) + baseRadius;
    float radiusMin = baseRadius * (0.0294f * segments) + 0.5f;

    std::vector<float> multipliers;
    multipliers.reserve(segments);
    float lastRadius = baseRadius;
    for (int i = 0; i < segments; ++i) {
        float newRadius = dist(rng) * (radiusMax - radiusMin) + radiusMin;
        multipliers.push_back(newRadius / lastRadius);
        lastRadius = newRadius;
    }
    return multipliers;
}

// ────────────────────────────────────────────────────────────────────────
// Segmented hull builder
// ────────────────────────────────────────────────────────────────────────

TriangulatedMesh buildSegmentedHull(int sides, int segments,
                                    float segmentLength, float baseRadius,
                                    const std::vector<float>& radiusMultipliers,
                                    float scaleX, float scaleZ,
                                    const glm::vec3& color) {
    // The hull is built along the +Y axis (forward direction).
    // Start with a base polygon at Y = 0.
    glm::vec3 fwd(0.0f, 1.0f, 0.0f);
    glm::vec3 centre(0.0f);

    PolyFace baseFace = generatePolygonFace(sides, baseRadius, centre, fwd,
                                            scaleX, scaleZ);

    std::vector<PolyFace> capFaces;     // top/bottom cap faces
    std::vector<PolyFace> wallFaces;    // stitched wall quads

    capFaces.push_back(baseFace);       // bottom cap

    float currentRadius = baseRadius;
    PolyFace previous = baseFace;

    for (int i = 0; i < segments; ++i) {
        float mult = (i < static_cast<int>(radiusMultipliers.size()))
                     ? radiusMultipliers[i] : 1.0f;
        currentRadius *= mult;

        // Extrude produces the next cross-section ring
        PolyFace next = extrudeFace(previous, segmentLength, mult, fwd);

        // Stitch the walls between the two rings
        auto walls = stitchFaces(previous, next);
        wallFaces.insert(wallFaces.end(), walls.begin(), walls.end());

        previous = next;
    }

    capFaces.push_back(previous);       // top cap (nose/rear)

    // Triangulate everything into one mesh
    std::vector<PolyFace> allFaces;
    allFaces.reserve(capFaces.size() + wallFaces.size());
    allFaces.insert(allFaces.end(), capFaces.begin(), capFaces.end());
    allFaces.insert(allFaces.end(), wallFaces.begin(), wallFaces.end());

    return triangulateFaces(allFaces, color);
}

} // namespace eve
