#pragma once

/**
 * @file photon_renderer.h
 * @brief Low-level OpenGL 2D renderer for the Photon UI system
 *
 * Handles batched quad/triangle rendering with translucency, used by
 * all Photon widgets.  The renderer maintains its own shader, VAO and
 * VBO and expects an OpenGL 3.3+ core-profile context.
 */

#include "photon_types.h"
#include <vector>
#include <cstdint>

namespace photon {

/**
 * Per-vertex data pushed into the GPU batch buffer.
 */
struct UIVertex {
    float x, y;        // screen-space position
    float u, v;        // texture coordinates (0 for flat color)
    float r, g, b, a;  // vertex color
};

/**
 * PhotonRenderer — batched 2D renderer for UI primitives.
 *
 * Usage each frame:
 *   renderer.begin(windowW, windowH);
 *   renderer.drawRect(...);
 *   renderer.drawText(...);
 *   ...
 *   renderer.end();            // flushes GPU draw calls
 */
class PhotonRenderer {
public:
    PhotonRenderer();
    ~PhotonRenderer();

    /** Compile shaders and create GPU resources.  Call once at startup. */
    bool init();

    /** Release GPU resources.  Call once at shutdown. */
    void shutdown();

    /** Begin a new UI frame.  Sets up orthographic projection. */
    void begin(int windowW, int windowH);

    /** Flush all batched geometry and restore GL state. */
    void end();

    // ── Drawing primitives ──────────────────────────────────────────

    /** Solid filled rectangle. */
    void drawRect(const Rect& r, const Color& c);

    /** Filled rectangle with per-corner colors (top-L, top-R, bot-R, bot-L). */
    void drawRectGradient(const Rect& r,
                          const Color& topLeft, const Color& topRight,
                          const Color& botRight, const Color& botLeft);

    /** Filled rounded rectangle (approximate: split into centre + edge quads). */
    void drawRoundedRect(const Rect& r, const Color& c, float radius);

    /** Rectangle outline (1px default, or custom width). */
    void drawRectOutline(const Rect& r, const Color& c, float width = 1.0f);

    /** Rounded rectangle outline. */
    void drawRoundedRectOutline(const Rect& r, const Color& c,
                                float radius, float width = 1.0f);

    /** Horizontal line. */
    void drawLine(Vec2 a, Vec2 b, const Color& c, float width = 1.0f);

    /** Filled circle (N-gon approximation). */
    void drawCircle(Vec2 centre, float radius, const Color& c, int segments = 32);

    /** Circle outline. */
    void drawCircleOutline(Vec2 centre, float radius, const Color& c,
                           float width = 1.0f, int segments = 32);

    /** Filled arc (pie-slice).  Angles in radians, 0 = right, CCW. */
    void drawArc(Vec2 centre, float innerR, float outerR,
                 float startAngle, float endAngle,
                 const Color& c, int segments = 32);

    /** Horizontal progress bar with background. */
    void drawProgressBar(const Rect& r, float fraction,
                         const Color& fg, const Color& bg);

    /** Simple ASCII text (built-in 8×13 bitmap font).
     *  Returns the width in pixels of the rendered string. */
    float drawText(const std::string& text, Vec2 pos,
                   const Color& c, float scale = 1.0f);

    /** Measure text width without drawing. */
    float measureText(const std::string& text, float scale = 1.0f) const;

    /** Set a clip rectangle (scissor).  Pass empty Rect to clear. */
    void pushClip(const Rect& r);
    void popClip();

private:
    // OpenGL handles
    uint32_t m_shaderProgram = 0;
    uint32_t m_vao = 0;
    uint32_t m_vbo = 0;
    uint32_t m_fontTexture = 0;
    int      m_uniformProj = -1;
    int      m_uniformUseTex = -1;
    int      m_uniformTex = -1;

    // Batch buffer
    std::vector<UIVertex> m_vertices;
    static constexpr size_t MAX_VERTICES = 65536;

    // State
    int  m_windowW = 1280;
    int  m_windowH = 720;
    bool m_inFrame = false;

    // Clip stack
    std::vector<Rect> m_clipStack;

    // Helpers
    void flush();
    void addQuad(float x0, float y0, float x1, float y1,
                 float u0, float v0, float u1, float v1,
                 const Color& c);
    void addQuadGradient(float x0, float y0, float x1, float y1,
                         const Color& tl, const Color& tr,
                         const Color& br, const Color& bl);
    void addTriangle(float x0, float y0,
                     float x1, float y1,
                     float x2, float y2,
                     const Color& c);
    void buildFontTexture();
};

} // namespace photon
