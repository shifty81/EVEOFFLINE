#ifndef RML_UI_MANAGER_H
#define RML_UI_MANAGER_H

/**
 * @file rml_ui_manager.h
 * @brief RmlUi-based UI manager for EVE OFFLINE Photon UI replication.
 *
 * This class replaces ImGui as the primary game UI framework, using RmlUi
 * to render panels defined in RML (HTML-like) documents styled with RCSS
 * (CSS-like) stylesheets.
 *
 * ImGui is retained for debug/developer overlays via the existing UIManager
 * class (ui_manager.h). Both systems can coexist in the render loop.
 *
 * Build with -DUSE_RMLUI=ON to enable RmlUi support.
 */

#include <string>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

// Forward declarations
struct GLFWwindow;

namespace Rml {
    class Context;
    class ElementDocument;
}

namespace eve {
    class Entity;
}

namespace UI {

// Forward declarations for render/system interfaces
class RmlRenderInterface;
class RmlSystemInterface;
class RmlFileInterface;

/**
 * @brief Manages all RmlUi documents and contexts for the game UI.
 *
 * The RmlUiManager owns the RmlUi context and all loaded documents. It
 * handles initialization, input forwarding, and per-frame updates. Game
 * code updates the UI by calling setter methods that push data into the
 * RmlUi data model.
 *
 * Lifecycle:
 *   1. Construct
 *   2. Initialize(window) — sets up RmlUi, loads theme + documents
 *   3. Per frame: ProcessInput() → Update() → Render()
 *   4. Shutdown() — tears down RmlUi context
 *
 * All methods must be called from the main/render thread.
 */
class RmlUiManager {
public:
    RmlUiManager();
    ~RmlUiManager();

    // Non-copyable, non-movable
    RmlUiManager(const RmlUiManager&) = delete;
    RmlUiManager& operator=(const RmlUiManager&) = delete;

    // ----------------------------------------------------------------
    // Lifecycle
    // ----------------------------------------------------------------

    /**
     * @brief Initialize RmlUi with the given GLFW window and OpenGL context.
     * @param window A valid GLFW window (must have an active GL context).
     * @param resourcePath Base path for ui_resources/ (RML, RCSS, fonts).
     * @return true on success.
     */
    bool Initialize(GLFWwindow* window, const std::string& resourcePath = "ui_resources");

    /** @brief Shut down RmlUi and release all resources. */
    void Shutdown();

    // ----------------------------------------------------------------
    // Per-Frame
    // ----------------------------------------------------------------

    /** @brief Forward GLFW input events to RmlUi (call before Update). */
    void ProcessInput();

    /** @brief Advance RmlUi animations and layout (call once per frame). */
    void Update();

    /** @brief Render all visible RmlUi documents (call after 3D scene). */
    void Render();

    // ----------------------------------------------------------------
    // Ship Status (HUD)
    // ----------------------------------------------------------------

    /** @brief Update shield percentage (0.0 – 1.0). */
    void SetShieldPercent(float pct);

    /** @brief Update armor percentage (0.0 – 1.0). */
    void SetArmorPercent(float pct);

    /** @brief Update hull percentage (0.0 – 1.0). */
    void SetHullPercent(float pct);

    /** @brief Update capacitor percentage (0.0 – 1.0). */
    void SetCapacitorPercent(float pct);

    /** @brief Update current velocity (m/s). */
    void SetVelocity(float velocity);

    /** @brief Update maximum velocity (m/s). */
    void SetMaxVelocity(float maxVelocity);

    /** @brief Convenience: set all ship values at once. */
    void SetShipStatus(float shieldPct, float armorPct, float hullPct,
                       float capPct, float velocity, float maxVelocity);

    // ----------------------------------------------------------------
    // Target List
    // ----------------------------------------------------------------

    /** @brief Add or update a locked target in the target list. */
    void SetTarget(const std::string& id, const std::string& name,
                   float shieldPct, float armorPct, float hullPct,
                   float distance, bool isHostile, bool isActive);

    /** @brief Remove a target from the list. */
    void RemoveTarget(const std::string& id);

    /** @brief Clear all targets. */
    void ClearTargets();

    // ----------------------------------------------------------------
    // Overview
    // ----------------------------------------------------------------

    /**
     * @brief Rebuild the overview table from the current entity set.
     * @param entities Map of entity ID → Entity pointer.
     * @param playerPos Player's current world position (for distance calc).
     */
    void UpdateOverviewData(
        const std::unordered_map<std::string, std::shared_ptr<eve::Entity>>& entities,
        const glm::vec3& playerPos);

    /** @brief Set the active overview filter tab (all, hostile, friendly, neutral). */
    void SetOverviewFilter(const std::string& filter);

    // ----------------------------------------------------------------
    // Panel Visibility
    // ----------------------------------------------------------------

    /** @brief Show or hide a named document (e.g. "fitting", "overview"). */
    void SetDocumentVisible(const std::string& name, bool visible);

    /** @brief Check if a named document is currently visible. */
    bool IsDocumentVisible(const std::string& name) const;

    /** @brief Toggle visibility of a named document. */
    void ToggleDocument(const std::string& name);

    // ----------------------------------------------------------------
    // Combat Log
    // ----------------------------------------------------------------

    /** @brief Append a message to the combat log. */
    void AddCombatLogMessage(const std::string& message);

    // ----------------------------------------------------------------
    // State Queries
    // ----------------------------------------------------------------

    /** @brief Returns true if RmlUi is initialized and ready. */
    bool IsInitialized() const { return initialized_; }

    /** @brief Get the underlying RmlUi context (advanced usage). */
    Rml::Context* GetContext() { return context_; }

private:
    bool initialized_ = false;
    GLFWwindow* window_ = nullptr;
    Rml::Context* context_ = nullptr;

    // RmlUi interface implementations
    std::unique_ptr<RmlRenderInterface> renderInterface_;
    std::unique_ptr<RmlSystemInterface> systemInterface_;
    std::unique_ptr<RmlFileInterface> fileInterface_;

    // Loaded documents by name
    std::unordered_map<std::string, Rml::ElementDocument*> documents_;

    // Resource base path
    std::string resourcePath_;

    // ----------------------------------------------------------------
    // Internal helpers
    // ----------------------------------------------------------------

    /** @brief Load core documents (HUD, overview, etc.). */
    bool LoadDocuments();

    /** @brief Register custom element instancers (circular gauges, etc.). */
    void RegisterCustomElements();

    /** @brief Set up data model bindings for live game state. */
    void SetupDataBindings();
};

} // namespace UI

#endif // RML_UI_MANAGER_H
