/**
 * @file rml_event_listener.h
 * @brief RmlUi event listener for connecting RML button/element events to C++ callbacks
 *
 * Provides a reusable EventListener that maps RmlUi element events (click,
 * submit, change) to std::function callbacks, and an EventInstaller helper
 * to wire them up by element ID.
 *
 * Usage:
 *   RmlEventInstaller::Install(document, "btn-stop", "click",
 *       [&](Rml::Event&) { game.stopShip(); });
 */

#ifndef RML_EVENT_LISTENER_H
#define RML_EVENT_LISTENER_H

#include <functional>
#include <string>
#include <vector>
#include <memory>

#ifdef USE_RMLUI
#include <RmlUi/Core.h>

namespace UI {

/**
 * @brief Generic RmlUi event listener backed by a std::function callback.
 *
 * Instances are created via RmlEventInstaller and attached to RML elements.
 * The listener calls the stored callback when the specified event fires.
 */
class RmlCallbackListener : public Rml::EventListener {
public:
    using Callback = std::function<void(Rml::Event&)>;

    explicit RmlCallbackListener(Callback cb) : m_callback(std::move(cb)) {}

    void ProcessEvent(Rml::Event& event) override {
        if (m_callback) {
            m_callback(event);
        }
    }

private:
    Callback m_callback;
};

/**
 * @brief Helper to install event listeners on RML document elements by ID.
 *
 * Owns all created listeners and cleans them up on destruction.
 * Typical lifetime: same as the RmlUiManager or the document.
 */
class RmlEventInstaller {
public:
    RmlEventInstaller() = default;
    ~RmlEventInstaller() = default;

    // Non-copyable
    RmlEventInstaller(const RmlEventInstaller&) = delete;
    RmlEventInstaller& operator=(const RmlEventInstaller&) = delete;

    /**
     * Attach an event listener to the element with the given ID in the document.
     *
     * @param doc       RmlUi document containing the target element.
     * @param elementId ID attribute of the target element.
     * @param eventName RmlUi event name (e.g. "click", "submit", "change").
     * @param callback  Function called when the event fires.
     * @return true if the element was found and the listener attached.
     */
    bool Install(Rml::ElementDocument* doc, const std::string& elementId,
                 const std::string& eventName,
                 RmlCallbackListener::Callback callback) {
        if (!doc) return false;

        auto* element = doc->GetElementById(elementId);
        if (!element) return false;

        auto listener = std::make_unique<RmlCallbackListener>(std::move(callback));
        element->AddEventListener(eventName, listener.get());
        m_listeners.push_back(std::move(listener));
        return true;
    }

    /** Remove all listeners (they will be freed). */
    void Clear() { m_listeners.clear(); }

    /** Number of installed listeners. */
    size_t Count() const { return m_listeners.size(); }

private:
    std::vector<std::unique_ptr<RmlCallbackListener>> m_listeners;
};

} // namespace UI

#else // !USE_RMLUI

namespace UI {

class RmlCallbackListener {};

class RmlEventInstaller {
public:
    void Clear() {}
    size_t Count() const { return 0; }
};

} // namespace UI

#endif // USE_RMLUI

#endif // RML_EVENT_LISTENER_H
