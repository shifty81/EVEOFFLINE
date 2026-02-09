/**
 * @file rml_ui_manager.h
 * @brief RmlUi-based UI manager for EVE OFFLINE Photon UI replication.
 *
 * This class provides the primary game UI framework using RmlUi to render
 * panels defined in RML (HTML-like) documents styled with RCSS (CSS-like)
 * stylesheets. It uses RmlUi's official GLFW platform and OpenGL 3 renderer
 * backends for a production-quality render pipeline.
 *
 * ImGui is retained for debug/developer overlays via the existing UIManager
 * class (ui_manager.h). Both systems can coexist in the render loop.
 *
 * Build with -DUSE_RMLUI=ON to enable RmlUi support.
 */

#ifndef RML_UI_MANAGER_H
#define RML_UI_MANAGER_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>

// Forward declarations
struct GLFWwindow;

#ifdef USE_RMLUI
namespace Rml {
    class Context;
    class ElementDocument;
    class RenderInterface;
    class SystemInterface;
}
class RenderInterface_GL3;
class SystemInterface_GLFW;
#endif

namespace eve {
    class Entity;
}

namespace UI {

/**
 * @brief Ship status data for the HUD display.
 */
struct RmlShipData {
    float shield_pct = 1.0f;
    float armor_pct = 1.0f;
    float hull_pct = 1.0f;
    float capacitor_pct = 1.0f;
    float velocity = 0.0f;
    float max_velocity = 100.0f;
};

/**
 * @brief Manages all RmlUi documents and contexts for the game UI.
 *
 * Uses RmlUi's official GLFW + OpenGL 3 backends for rendering.
 * Lifecycle:
 *   1. Construct
 *   2. Initialize(window) — sets up RmlUi, loads theme + documents
 *   3. Per frame: ProcessInput() → Update() → BeginFrame() → Render() → EndFrame()
 *   4. Shutdown() — tears down RmlUi context
 *
 * All methods must be called from the main/render thread.
 */
class RmlUiManager {
public:
    RmlUiManager();
    ~RmlUiManager();

    // Non-copyable
    RmlUiManager(const RmlUiManager&) = delete;
    RmlUiManager& operator=(const RmlUiManager&) = delete;

    // ---- Lifecycle ----
    bool Initialize(GLFWwindow* window, const std::string& resourcePath = "ui_resources");
    void Shutdown();

    // ---- Per-Frame ----
    void ProcessInput();
    void Update();
    void BeginFrame();
    void Render();
    void EndFrame();

    // ---- Ship Status ----
    void SetShipStatus(const RmlShipData& data);
    void SetShieldPercent(float pct);
    void SetArmorPercent(float pct);
    void SetHullPercent(float pct);
    void SetCapacitorPercent(float pct);
    void SetVelocity(float velocity);
    void SetMaxVelocity(float maxVelocity);
    void SetShipStatus(float shieldPct, float armorPct, float hullPct,
                       float capPct, float velocity, float maxVelocity);

    // ---- Target List ----
    void SetTarget(const std::string& id, const std::string& name,
                   float shieldPct, float armorPct, float hullPct,
                   float distance, bool isHostile, bool isActive);
    void RemoveTarget(const std::string& id);
    void ClearTargets();

    // ---- Overview ----
    void UpdateOverviewData(
        const std::unordered_map<std::string, std::shared_ptr<eve::Entity>>& entities,
        const glm::vec3& playerPos);
    void SetOverviewFilter(const std::string& filter);

    // ---- Panel Visibility ----
    void SetDocumentVisible(const std::string& name, bool visible);
    bool IsDocumentVisible(const std::string& name) const;
    void ToggleDocument(const std::string& name);

    // ---- Combat Log ----
    void AddCombatLogMessage(const std::string& message);

    // ---- Inventory ----
    void UpdateInventoryData(const std::vector<std::string>& names,
                             const std::vector<std::string>& types,
                             const std::vector<int>& quantities,
                             const std::vector<float>& volumes,
                             float capacityUsed, float capacityMax);

    // ---- D-Scan ----
    void UpdateDScanResults(const std::vector<std::string>& names,
                            const std::vector<std::string>& types,
                            const std::vector<float>& distances);

    // ---- State Queries ----
    bool IsInitialized() const { return initialized_; }

private:
    bool initialized_ = false;
    GLFWwindow* window_ = nullptr;
    std::string resourcePath_;

#ifdef USE_RMLUI
    Rml::Context* context_ = nullptr;
    std::unique_ptr<RenderInterface_GL3> renderInterface_;
    std::unique_ptr<SystemInterface_GLFW> systemInterface_;

    // Loaded documents by name
    std::unordered_map<std::string, Rml::ElementDocument*> documents_;

    // Ship data for updating elements
    RmlShipData shipData_;

    // Target data for the target list
    struct TargetInfo {
        std::string id;
        std::string name;
        float shieldPct = 1.0f;
        float armorPct = 1.0f;
        float hullPct = 1.0f;
        float distance = 0.0f;
        bool isHostile = false;
        bool isActive = false;
    };
    std::vector<TargetInfo> targets_;

    // Overview filter state
    std::string overviewFilter_ = "all";

    // Internal helpers
    bool LoadDocuments();
    void UpdateHudElements();
    void UpdateTargetListElements();
    void SetupInputCallbacks();
#endif
};

} // namespace UI

#endif // RML_UI_MANAGER_H
