#include "ui/photon/photon_context.h"

namespace photon {

PhotonContext::PhotonContext() = default;
PhotonContext::~PhotonContext() { shutdown(); }

bool PhotonContext::init() {
    return m_renderer.init();
}

void PhotonContext::shutdown() {
    m_renderer.shutdown();
}

void PhotonContext::beginFrame(const InputState& input) {
    m_input = input;
    m_hotID = 0;  // reset hot each frame; widgets re-claim it
    m_renderer.begin(input.windowW, input.windowH);
}

void PhotonContext::endFrame() {
    m_renderer.end();
    // If mouse was released this frame, clear active widget
    if (m_input.mouseReleased[0]) {
        m_activeID = 0;
    }
}

// ── Interaction ─────────────────────────────────────────────────────

bool PhotonContext::isHovered(const Rect& r) const {
    return r.contains(m_input.mousePos);
}

void PhotonContext::setHot(WidgetID id) {
    // Only allow hot if no other widget is active (or this one is)
    if (m_activeID == 0 || m_activeID == id) {
        m_hotID = id;
    }
}

void PhotonContext::setActive(WidgetID id)  { m_activeID = id; }
void PhotonContext::clearActive()           { m_activeID = 0; }

bool PhotonContext::buttonBehavior(const Rect& r, WidgetID id) {
    bool hovered = isHovered(r);
    bool clicked = false;

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

void PhotonContext::pushID(const char* label) {
    WidgetID parent = m_idStack.empty() ? 0u : m_idStack.back();
    // Combine parent hash with label hash
    WidgetID h = hashID(label);
    h ^= parent * 2654435761u;  // Knuth multiplicative hash mix
    m_idStack.push_back(h);
}

void PhotonContext::popID() {
    if (!m_idStack.empty()) m_idStack.pop_back();
}

WidgetID PhotonContext::currentID(const char* label) const {
    WidgetID parent = m_idStack.empty() ? 0u : m_idStack.back();
    WidgetID h = hashID(label);
    h ^= parent * 2654435761u;
    return h;
}

Vec2 PhotonContext::getDragDelta() const {
    // Drag position tracking is handled externally via PanelState.
    // This method is reserved for future per-frame delta computation.
    return {0.0f, 0.0f};
}

} // namespace photon
