#include "ui/atlas/atlas_context.h"

namespace atlas {

AtlasContext::AtlasContext() = default;
AtlasContext::~AtlasContext() { shutdown(); }

bool AtlasContext::init() {
    return m_renderer.init();
}

void AtlasContext::shutdown() {
    m_renderer.shutdown();
}

void AtlasContext::beginFrame(const InputState& input) {
    m_prevMousePos = m_input.mousePos;  // save previous frame's position
    m_input = input;
    m_hotID = 0;  // reset hot each frame; widgets re-claim it
    m_mouseConsumed = false;  // reset consumed flag each frame
    m_renderer.begin(input.windowW, input.windowH);
}

void AtlasContext::endFrame() {
    m_renderer.end();
    // If mouse was released this frame, clear active widget
    if (m_input.mouseReleased[0]) {
        m_activeID = 0;
    }
}

// ── Interaction ─────────────────────────────────────────────────────

bool AtlasContext::isHovered(const Rect& r) const {
    return r.contains(m_input.mousePos);
}

void AtlasContext::setHot(WidgetID id) {
    // Only allow hot if no other widget is active (or this one is)
    if (m_activeID == 0 || m_activeID == id) {
        m_hotID = id;
    }
}

void AtlasContext::setActive(WidgetID id)  { m_activeID = id; }
void AtlasContext::clearActive()           { m_activeID = 0; }

bool AtlasContext::buttonBehavior(const Rect& r, WidgetID id) {
    bool hovered = isHovered(r);
    bool clicked = false;

    // If mouse is already consumed by a higher-priority widget, skip interaction
    if (m_mouseConsumed) {
        return false;
    }

    if (hovered) {
        setHot(id);
        if (m_input.mouseClicked[0]) {
            setActive(id);
        }
    }

    if (isActive(id) && m_input.mouseReleased[0]) {
        if (hovered) clicked = true;
        clearActive();
    }

    return clicked;
}

// ── ID stack ────────────────────────────────────────────────────────

void AtlasContext::pushID(const char* label) {
    WidgetID parent = m_idStack.empty() ? 0u : m_idStack.back();
    // Combine parent hash with label hash
    WidgetID h = hashID(label);
    h ^= parent * 2654435761u;  // Knuth multiplicative hash mix
    m_idStack.push_back(h);
}

void AtlasContext::popID() {
    if (!m_idStack.empty()) m_idStack.pop_back();
}

WidgetID AtlasContext::currentID(const char* label) const {
    WidgetID parent = m_idStack.empty() ? 0u : m_idStack.back();
    WidgetID h = hashID(label);
    h ^= parent * 2654435761u;
    return h;
}

Vec2 AtlasContext::getDragDelta() const {
    return m_input.mousePos - m_prevMousePos;
}

} // namespace atlas
