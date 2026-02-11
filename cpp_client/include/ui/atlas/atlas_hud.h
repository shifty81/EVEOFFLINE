#pragma once

/**
 * @file atlas_hud.h
 * @brief Full EVE-style HUD layout compositor using Atlas widgets
 *
 * AtlasHUD assembles all individual Atlas widgets into the complete
 * EVE Online-style game HUD layout:
 *
 *   ┌─────────┬───────────────────────────────────────┬──────────────┐
 *   │ Neocom  │   Locked Target Cards (top-center)    │ Selected     │
 *   │ sidebar │                                       │ Item panel   │
 *   │         │                                       ├──────────────┤
 *   │         │          3D Space View                │ Overview     │
 *   │         │                                       │ panel        │
 *   │         │                                       │              │
 *   │         │        ┌──── Ship HUD ────┐           │              │
 *   │         │        │ Status arcs      │           │              │
 *   │         │        │ Capacitor ring   │           │              │
 *   │         │        │ Module rack      │           │              │
 *   │         │        │ Speed indicator  │           │              │
 *   │         │        └──────────────────┘           │              │
 *   └─────────┴───────────────────────────────────────┴──────────────┘
 *
 * Usage:
 *   atlas::AtlasHUD hud;
 *   hud.init(ctx);
 *   // Each frame:
 *   hud.update(ctx, shipData, targetData, overviewData);
 */

#include "atlas_context.h"
#include "atlas_widgets.h"
#include <vector>
#include <string>

namespace atlas {

/**
 * Ship status data fed into the HUD each frame.
 */
struct ShipHUDData {
    float shieldPct   = 1.0f;
    float armorPct    = 1.0f;
    float hullPct     = 1.0f;
    float capacitorPct = 1.0f;
    float currentSpeed = 0.0f;
    float maxSpeed     = 250.0f;
    int   capSegments  = 16;

    // Module rack (up to 8 high, 8 mid, 8 low slots)
    struct ModuleInfo {
        bool   fitted    = false;
        bool   active    = false;
        float  cooldown  = 0.0f;    // 0-1 fraction remaining
        Color  color     = {0.5f, 0.5f, 0.5f, 1.0f};
        float  overheat  = 0.0f;    // 0-1 heat damage level (1.0 = burnt out)
    };
    std::vector<ModuleInfo> highSlots;
    std::vector<ModuleInfo> midSlots;
    std::vector<ModuleInfo> lowSlots;
};

/**
 * AtlasHUD — assembles Atlas widgets into a complete EVE-style HUD.
 *
 * All layout is computed automatically based on window size.
 * Panels are movable via PanelState when unlocked.
 */
class AtlasHUD {
public:
    AtlasHUD();
    ~AtlasHUD();

    /** Initialise panel states with default positions. Call once. */
    void init(int windowW, int windowH);

    /**
     * Draw the complete HUD for one frame.
     *
     * @param ctx          Atlas context (must be between beginFrame/endFrame).
     * @param ship         Ship status data.
     * @param targets      Locked target list.
     * @param overview     Overview entries.
     * @param selectedItem Currently selected item info (may be empty name).
     */
    void update(AtlasContext& ctx,
                const ShipHUDData& ship,
                const std::vector<TargetCardInfo>& targets,
                const std::vector<OverviewEntry>& overview,
                const SelectedItemInfo& selectedItem);

    // ── Panel visibility toggles ────────────────────────────────────

    void toggleOverview()      { m_overviewState.open = !m_overviewState.open; }
    void toggleSelectedItem()  { m_selectedItemState.open = !m_selectedItemState.open; }

    bool isOverviewOpen()      const { return m_overviewState.open; }
    bool isSelectedItemOpen()  const { return m_selectedItemState.open; }

    // ── Sidebar callback ──────────────────────────────────────────────

    /** Set callback for sidebar icon clicks. */
    void setSidebarCallback(const std::function<void(int)>& cb) { m_sidebarCallback = cb; }

    // ── Module click callback ───────────────────────────────────────

    /** Set callback for module slot clicks (slot index passed). */
    void setModuleCallback(const std::function<void(int)>& cb) { m_moduleCallback = cb; }

    // ── Selected item action callbacks ──────────────────────────────

    /** Set callback for selected item action buttons (orbit, approach, warp, info). */
    void setSelectedItemOrbitCb(const std::function<void()>& cb)    { m_selOrbitCb = cb; }
    void setSelectedItemApproachCb(const std::function<void()>& cb) { m_selApproachCb = cb; }
    void setSelectedItemWarpCb(const std::function<void()>& cb)     { m_selWarpCb = cb; }
    void setSelectedItemInfoCb(const std::function<void()>& cb)     { m_selInfoCb = cb; }

    // ── Movement mode indicator ─────────────────────────────────────

    /** Set the currently active movement mode text (empty to hide). */
    void setModeIndicator(const std::string& text) { m_modeText = text; }

    // ── Info panel ──────────────────────────────────────────────────

    /** Show the info panel for an entity. */
    void showInfoPanel(const InfoPanelData& data);

    /** Close the info panel. */
    void closeInfoPanel() { m_infoPanelState.open = false; }

    /** Check if the info panel is open. */
    bool isInfoPanelOpen() const { return m_infoPanelState.open; }

    // ── Overview tab API ────────────────────────────────────────────

    /** Get the active overview tab index. */
    int  getActiveOverviewTab() const { return m_overviewActiveTab; }

    /** Set the active overview tab index. */
    void setActiveOverviewTab(int tab) { m_overviewActiveTab = tab; }

private:
    // Panel states (persistent across frames)
    PanelState m_overviewState;
    PanelState m_selectedItemState;
    PanelState m_infoPanelState;

    // Sidebar config
    float m_sidebarWidth = 40.0f;
    int   m_sidebarIcons = 8;

    // Callbacks
    std::function<void(int)> m_sidebarCallback;
    std::function<void(int)> m_moduleCallback;
    std::function<void()>    m_selOrbitCb;
    std::function<void()>    m_selApproachCb;
    std::function<void()>    m_selWarpCb;
    std::function<void()>    m_selInfoCb;

    // Internal layout helpers
    void drawShipHUD(AtlasContext& ctx, const ShipHUDData& ship);
    void drawTargetCards(AtlasContext& ctx,
                        const std::vector<TargetCardInfo>& targets);
    void drawOverviewPanel(AtlasContext& ctx,
                          const std::vector<OverviewEntry>& entries);
    void drawSelectedItemPanel(AtlasContext& ctx,
                              const SelectedItemInfo& info);
    void drawModeIndicator(AtlasContext& ctx);
    void drawInfoPanel(AtlasContext& ctx);

    // Animation state
    float m_displayCapFrac = 1.0f;   // smoothed capacitor display value
    float m_time           = 0.0f;   // accumulated time for pulse animations

    // Mode indicator
    std::string m_modeText;

    // Overview tab state
    int m_overviewActiveTab = 0;

    // Info panel data
    InfoPanelData m_infoPanelData;
};

} // namespace atlas
