#ifndef NEOCOM_PANEL_H
#define NEOCOM_PANEL_H

#include <string>
#include <functional>
#include <vector>

namespace UI {

/**
 * Neocom – the EVE Online-style vertical icon sidebar.
 *
 * Rendered as a narrow bar on the left side of the screen with icon buttons
 * for core services: Character Sheet, Inventory, Fitting, Market, Map,
 * Corporation, D-Scan, and Settings.
 *
 * Each button fires a user-supplied callback (typically a panel toggle in UIManager).
 */
class NeocomPanel {
public:
    NeocomPanel();
    ~NeocomPanel() = default;

    /** Render the sidebar (call each frame between BeginFrame/EndFrame). */
    void Render();

    /** Set visibility. */
    void SetVisible(bool v) { m_visible = v; }
    bool IsVisible() const { return m_visible; }

    /** Set collapsed (icon-only) vs expanded (icon + label). */
    void SetCollapsed(bool c) { m_collapsed = c; }
    bool IsCollapsed() const { return m_collapsed; }
    void ToggleCollapsed() { m_collapsed = !m_collapsed; }

    // ---- Service callbacks ----
    using ActionCallback = std::function<void()>;

    void SetCharacterSheetCallback(ActionCallback cb) { m_onCharacterSheet = std::move(cb); }
    void SetInventoryCallback(ActionCallback cb) { m_onInventory = std::move(cb); }
    void SetFittingCallback(ActionCallback cb) { m_onFitting = std::move(cb); }
    void SetMarketCallback(ActionCallback cb) { m_onMarket = std::move(cb); }
    void SetMapCallback(ActionCallback cb) { m_onMap = std::move(cb); }
    void SetCorporationCallback(ActionCallback cb) { m_onCorporation = std::move(cb); }
    void SetDScanCallback(ActionCallback cb) { m_onDScan = std::move(cb); }
    void SetMissionsCallback(ActionCallback cb) { m_onMissions = std::move(cb); }
    void SetSettingsCallback(ActionCallback cb) { m_onSettings = std::move(cb); }
    void SetChatCallback(ActionCallback cb) { m_onChat = std::move(cb); }
    void SetDronesCallback(ActionCallback cb) { m_onDrones = std::move(cb); }

private:
    bool m_visible = true;
    bool m_collapsed = true;  // Start collapsed (icons only)

    // Callbacks
    ActionCallback m_onCharacterSheet;
    ActionCallback m_onInventory;
    ActionCallback m_onFitting;
    ActionCallback m_onMarket;
    ActionCallback m_onMap;
    ActionCallback m_onCorporation;
    ActionCallback m_onDScan;
    ActionCallback m_onMissions;
    ActionCallback m_onSettings;
    ActionCallback m_onChat;
    ActionCallback m_onDrones;

    /** Helper to render a single sidebar button. Returns true if clicked. */
    bool RenderButton(const char* icon, const char* label, const char* tooltip);
};

} // namespace UI

#endif // NEOCOM_PANEL_H
