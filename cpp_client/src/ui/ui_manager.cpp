#include "ui/ui_manager.h"
#include "ui/hud_panels.h"
#include "ui/target_list.h"
#include "ui/inventory_panel.h"
#include "ui/fitting_panel.h"
#include "ui/mission_panel.h"
#include "ui/overview_panel.h"
#include "ui/market_panel.h"
#include "ui/dscan_panel.h"
#include "ui/sidebar_panel.h"
#include "ui/chat_panel.h"
#include "ui/drone_control_panel.h"
#include "ui/notification_manager.h"
#include "ui/probe_scanner_panel.h"
#include "core/entity.h"
#include <algorithm>
#include <iostream>
#include <cstdio>

namespace UI {

static float safePct(float current, float max) {
    return max > 0.0f ? current / max : 0.0f;
}

// ============================================================================
// Construction / Destruction
// ============================================================================

UIManager::UIManager()
    : show_ship_status_(true)
    , show_target_info_(true)
    , show_speed_panel_(true)
    , show_combat_log_(true)
    , show_target_list_(true)
{
    // Create legacy panel objects for data storage
    m_targetList = std::make_unique<TargetList>();
    m_inventoryPanel = std::make_unique<InventoryPanel>();
    m_fittingPanel = std::make_unique<FittingPanel>();
    m_missionPanel = std::make_unique<MissionPanel>();
    m_overviewPanel = std::make_unique<OverviewPanel>();
    m_marketPanel = std::make_unique<MarketPanel>();
    m_dscanPanel = std::make_unique<DScanPanel>();
    m_sidebarPanel = std::make_unique<SidebarPanel>();
    m_chatPanel = std::make_unique<ChatPanel>();
    m_droneControlPanel = std::make_unique<DroneControlPanel>();
    m_notificationManager = std::make_unique<NotificationManager>();
    m_probeScannerPanel = std::make_unique<ProbeScannerPanel>();

    // Initialize module slots to default empty state
    for (int i = 0; i < MAX_MODULE_SLOTS; ++i) {
        m_moduleSlots[i] = ModuleSlotState();
    }
}

UIManager::~UIManager() {
    Shutdown();
}

// ============================================================================
// Initialization / Shutdown
// ============================================================================

bool UIManager::Initialize(int windowW, int windowH) {
    m_windowW = windowW;
    m_windowH = windowH;

    // Initialize Atlas rendering context (compiles shaders, allocates GPU buffers)
    if (!m_ctx.init()) {
        std::cerr << "[UIManager] Error: Failed to initialize Atlas context" << std::endl;
        return false;
    }

    // Initialize HUD layout with window dimensions
    m_hud.init(windowW, windowH);

    // Set up Sidebar icon callbacks
    m_hud.setSidebarCallback([this](int icon) {
        switch (icon) {
            case 0: ToggleInventory(); break;
            case 1: ToggleFitting(); break;
            case 2: ToggleMarket(); break;
            case 3: ToggleMission(); break;
            case 4: ToggleDScan(); break;
            case 5: ToggleMap(); break;
            case 6: ToggleChat(); break;
            case 7: ToggleDrones(); break;
        }
    });

    // Wire legacy SidebarPanel callbacks
    if (m_sidebarPanel) {
        m_sidebarPanel->SetInventoryCallback([this]() { ToggleInventory(); });
        m_sidebarPanel->SetFittingCallback([this]() { ToggleFitting(); });
        m_sidebarPanel->SetMarketCallback([this]() { ToggleMarket(); });
        m_sidebarPanel->SetMissionsCallback([this]() { ToggleMission(); });
        m_sidebarPanel->SetDScanCallback([this]() { ToggleDScan(); });
        m_sidebarPanel->SetMapCallback([this]() { ToggleMap(); });
        m_sidebarPanel->SetChatCallback([this]() { ToggleChat(); });
        m_sidebarPanel->SetDronesCallback([this]() { ToggleDrones(); });
    }

    // Initialize default data
    ship_status_ = ShipStatus();
    target_info_ = TargetInfo();

    // Set up panel configs with default positions
    InitPanelConfigs(windowW, windowH);

    // Create default layout presets (writes only if files don't exist)
    m_layoutManager.CreateDefaultPresets(windowW, windowH);

    std::cout << "[UIManager] Atlas UI initialized successfully" << std::endl;
    return true;
}

void UIManager::InitPanelConfigs(int windowW, int windowH) {
    auto addPanel = [&](const std::string& id, const std::string& title,
                        float x, float y, float w, float h, bool visible) {
        PanelConfig cfg;
        cfg.title = title;
        cfg.state.bounds = atlas::Rect(x, y, w, h);
        cfg.state.open = visible;
        m_panelConfigs[id] = cfg;
    };

    addPanel("inventory",      "Inventory",      50,  300, 350, 400, false);
    addPanel("fitting",        "Fitting",        420, 300, 400, 450, false);
    addPanel("mission",        "Missions",       50,  50,  400, 350, false);
    addPanel("overview",       "Overview",       static_cast<float>(windowW - 390), 50, 380, 400, true);
    addPanel("market",         "Market",         420, 50,  450, 500, false);
    addPanel("dscan",          "D-Scan",         static_cast<float>(windowW - 360), 460, 350, 300, false);
    addPanel("chat",           "Chat",           60,  420, 380, 300, false);
    addPanel("drones",         "Drones",         60,  300, 320, 400, false);
    addPanel("probe_scanner",  "Probe Scanner",  420, 50,  400, 350, false);
}

void UIManager::Shutdown() {
    m_ctx.shutdown();
}

// ============================================================================
// Frame Management
// ============================================================================

void UIManager::BeginFrame(const atlas::InputState& input) {
    m_ctx.beginFrame(input);
}

void UIManager::EndFrame() {
    m_ctx.endFrame();
}

// ============================================================================
// Main Render
// ============================================================================

void UIManager::Render() {
    const auto& theme = m_ctx.theme();

    // --- Build ShipHUDData from current ship status ---
    atlas::ShipHUDData shipData;
    shipData.shieldPct    = safePct(ship_status_.shields, ship_status_.shields_max);
    shipData.armorPct     = safePct(ship_status_.armor, ship_status_.armor_max);
    shipData.hullPct      = safePct(ship_status_.hull, ship_status_.hull_max);
    shipData.capacitorPct = safePct(ship_status_.capacitor, ship_status_.capacitor_max);
    shipData.currentSpeed = ship_status_.velocity;
    shipData.maxSpeed     = ship_status_.max_velocity;

    // Populate module slots
    for (int i = 0; i < m_moduleSlotCount && i < MAX_MODULE_SLOTS; ++i) {
        atlas::ShipHUDData::ModuleInfo mi;
        mi.fitted   = m_moduleSlots[i].fitted;
        mi.active   = m_moduleSlots[i].active;
        mi.cooldown = m_moduleSlots[i].cooldown_pct;
        mi.overheat = m_moduleSlots[i].overheated ? 1.0f : 0.0f;
        mi.color    = mi.active ? theme.accentPrimary : atlas::Color(0.5f, 0.5f, 0.5f, 1.0f);
        shipData.highSlots.push_back(mi);
    }

    // --- Build target cards from locked target info ---
    std::vector<atlas::TargetCardInfo> targets;
    if (target_info_.is_locked) {
        atlas::TargetCardInfo tc;
        tc.name      = target_info_.name;
        tc.shieldPct = safePct(target_info_.shields, target_info_.shields_max);
        tc.armorPct  = safePct(target_info_.armor, target_info_.armor_max);
        tc.hullPct   = safePct(target_info_.hull, target_info_.hull_max);
        tc.distance  = target_info_.distance;
        tc.isHostile = target_info_.is_hostile;
        tc.isActive  = true;
        targets.push_back(tc);
    }

    // --- Build overview entries (empty for now — filled by OverviewPanel data) ---
    std::vector<atlas::OverviewEntry> overview;

    // --- Build selected item info ---
    atlas::SelectedItemInfo selectedInfo;
    if (!m_selectedItem.isEmpty()) {
        selectedInfo.name     = m_selectedItem.name;
        if (m_selectedItem.distance >= 1000.0f) {
            selectedInfo.distance     = m_selectedItem.distance / 1000.0f;
            selectedInfo.distanceUnit = "km";
        } else {
            selectedInfo.distance     = m_selectedItem.distance;
            selectedInfo.distanceUnit = "m";
        }
    }

    // --- Render the main HUD (ship status, target cards, overview, selected item) ---
    m_hud.update(m_ctx, shipData, targets, overview, selectedInfo);

    // --- Render alert stack above the HUD ---
    RenderAlertStack();

    // --- Render dockable panels ---
    for (auto& [id, config] : m_panelConfigs) {
        if (!config.state.open) continue;
        RenderDockablePanel(id);
    }

    // --- Render combat log ---
    if (show_combat_log_) {
        RenderCombatLogPanel();
    }

    // --- Render star map overlay ---
    if (m_showStarMap) {
        RenderStarMapPanel();
    }
}

// ============================================================================
// Atlas-based Panel Rendering
// ============================================================================

void UIManager::RenderDockablePanel(const std::string& id) {
    auto it = m_panelConfigs.find(id);
    if (it == m_panelConfigs.end()) return;

    PanelConfig& cfg = it->second;
    atlas::PanelFlags flags;
    flags.locked = m_interfaceLocked;
    flags.compactMode = m_compactMode;

    if (atlas::panelBeginStateful(m_ctx, cfg.title.c_str(), cfg.state, flags)) {
        const auto& theme = m_ctx.theme();
        const atlas::Rect& b = cfg.state.bounds;
        float y = b.y + theme.headerHeight + theme.padding;
        float x = b.x + theme.padding;
        float contentW = b.w - theme.padding * 2.0f;

        // Stub content — label + separator indicating panel name
        atlas::label(m_ctx, atlas::Vec2(x, y), cfg.title, theme.textPrimary);
        y += 20.0f;
        atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
        y += theme.itemSpacing + 4.0f;
        atlas::label(m_ctx, atlas::Vec2(x, y),
                      "Panel content (Atlas stub)", theme.textSecondary);
    }
    atlas::panelEnd(m_ctx);
}

void UIManager::RenderCombatLogPanel() {
    const auto& theme = m_ctx.theme();
    const auto& input = m_ctx.input();

    float panelW = 450.0f;
    float panelH = 200.0f;
    float posX = 320.0f;
    float posY = static_cast<float>(input.windowH) - panelH - 60.0f;

    atlas::Rect bounds(posX, posY, panelW, panelH);
    atlas::PanelFlags flags;
    flags.locked = m_interfaceLocked;

    bool* openPtr = &show_combat_log_;
    if (atlas::panelBegin(m_ctx, "Combat Log", bounds, flags, openPtr)) {
        float y = bounds.y + theme.headerHeight + theme.padding;
        float x = bounds.x + theme.padding;

        for (const auto& msg : combat_log_) {
            atlas::label(m_ctx, atlas::Vec2(x, y), msg, theme.textSecondary);
            y += 16.0f;
            if (y > bounds.bottom() - theme.padding) break;
        }
    }
    atlas::panelEnd(m_ctx);
}

void UIManager::RenderStarMapPanel() {
    if (!m_showStarMap) return;

    const auto& theme = m_ctx.theme();
    const auto& input = m_ctx.input();

    float mapW = static_cast<float>(input.windowW) * 0.7f;
    float mapH = static_cast<float>(input.windowH) * 0.7f;
    float posX = (static_cast<float>(input.windowW) - mapW) * 0.5f;
    float posY = (static_cast<float>(input.windowH) - mapH) * 0.5f;

    atlas::Rect bounds(posX, posY, mapW, mapH);
    atlas::PanelFlags flags;
    flags.showClose = true;

    if (atlas::panelBegin(m_ctx, "Star Map (F10)", bounds, flags, &m_showStarMap)) {
        auto& r = m_ctx.renderer();
        float y = bounds.y + theme.headerHeight + theme.padding;
        float x = bounds.x + theme.padding;
        float contentW = bounds.w - theme.padding * 2.0f;

        atlas::label(m_ctx, atlas::Vec2(x, y),
                      "STAR MAP - Astralis", theme.accentPrimary);
        y += 22.0f;
        atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
        y += 10.0f;

        // Draw dark map background
        atlas::Rect mapArea(x, y, contentW, bounds.bottom() - y - theme.padding - 20.0f);
        r.drawRect(mapArea, atlas::Color(0.02f, 0.03f, 0.06f, 0.95f));

        // Grid lines
        float gridStep = 60.0f;
        atlas::Color gridColor(0.078f, 0.118f, 0.196f, 0.24f);
        for (float gx = 0; gx < mapArea.w; gx += gridStep) {
            r.drawLine(atlas::Vec2(mapArea.x + gx, mapArea.y),
                       atlas::Vec2(mapArea.x + gx, mapArea.bottom()),
                       gridColor);
        }
        for (float gy = 0; gy < mapArea.h; gy += gridStep) {
            r.drawLine(atlas::Vec2(mapArea.x, mapArea.y + gy),
                       atlas::Vec2(mapArea.right(), mapArea.y + gy),
                       gridColor);
        }

        // System nodes
        struct MapNode { const char* name; float rx, ry; float security; };
        MapNode nodes[] = {
            {"Thyrkstad",  0.50f, 0.40f, 1.0f},
            {"Perimeter",  0.60f, 0.35f, 0.9f},
            {"Solari",     0.70f, 0.60f, 1.0f},
            {"Aurendis",   0.30f, 0.55f, 0.9f},
            {"Rancer",     0.45f, 0.25f, 0.4f},
            {"Hek",        0.35f, 0.30f, 0.5f},
        };
        constexpr int nodeCount = 6;
        int connections[][2] = {{0,1}, {1,4}, {4,5}, {3,5}};

        // Connections
        for (auto& conn : connections) {
            atlas::Vec2 p1(mapArea.x + nodes[conn[0]].rx * mapArea.w,
                            mapArea.y + nodes[conn[0]].ry * mapArea.h);
            atlas::Vec2 p2(mapArea.x + nodes[conn[1]].rx * mapArea.w,
                            mapArea.y + nodes[conn[1]].ry * mapArea.h);
            r.drawLine(p1, p2, theme.accentDim.withAlpha(0.6f), 1.5f);
        }

        // Nodes
        for (int i = 0; i < nodeCount; ++i) {
            atlas::Vec2 pos(mapArea.x + nodes[i].rx * mapArea.w,
                             mapArea.y + nodes[i].ry * mapArea.h);
            atlas::Color nodeColor = nodes[i].security >= 0.5f
                ? theme.accentPrimary : theme.warning;
            r.drawCircle(pos, 6.0f, nodeColor);
            r.drawCircleOutline(pos, 8.0f, nodeColor.withAlpha(0.3f));
            r.drawText(nodes[i].name, atlas::Vec2(pos.x + 10, pos.y - 6),
                       theme.textPrimary);

            char secBuf[16];
            std::snprintf(secBuf, sizeof(secBuf), "%.1f", nodes[i].security);
            atlas::Color secColor = nodes[i].security >= 0.5f
                ? theme.success.withAlpha(0.7f) : theme.danger.withAlpha(0.7f);
            r.drawText(secBuf, atlas::Vec2(pos.x + 10, pos.y + 8), secColor);
        }

        // Legend
        float legendY = mapArea.bottom() + 5.0f;
        atlas::label(m_ctx, atlas::Vec2(x, legendY),
                      "Click system to set destination | Scroll to zoom",
                      theme.textSecondary);
    }
    atlas::panelEnd(m_ctx);
}

void UIManager::RenderAlertStack() {
    if (m_alerts.empty()) return;

    const auto& theme = m_ctx.theme();
    const auto& input = m_ctx.input();

    float centerX = static_cast<float>(input.windowW) * 0.5f;
    float baseY = static_cast<float>(input.windowH) - 330.0f;

    for (size_t i = 0; i < m_alerts.size(); ++i) {
        const HUDAlert& alert = m_alerts[i];

        // Choose color by priority
        atlas::Color color;
        switch (alert.priority) {
            case HUDAlertPriority::CRITICAL: color = theme.danger;  break;
            case HUDAlertPriority::WARNING:  color = theme.warning; break;
            default:                         color = theme.accentPrimary; break;
        }

        float alertW = m_ctx.renderer().measureText(alert.message) + 24.0f;
        float y = baseY - static_cast<float>(i) * 28.0f;
        float x = centerX - alertW * 0.5f;

        // Fade out in the last second
        float alpha = 1.0f;
        if (alert.duration - alert.elapsed < 1.0f) {
            alpha = alert.duration - alert.elapsed;
        }

        atlas::Rect alertRect(x, y, alertW, 24.0f);
        m_ctx.renderer().drawRoundedRect(alertRect,
            theme.bgPanel.withAlpha(0.85f * alpha), 3.0f);
        m_ctx.renderer().drawRoundedRectOutline(alertRect,
            color.withAlpha(0.6f * alpha), 3.0f);
        m_ctx.renderer().drawText(alert.message,
            atlas::Vec2(x + 12.0f, y + 5.0f), color.withAlpha(alpha));
    }
}

// ============================================================================
// Data Setters
// ============================================================================

void UIManager::SetShipStatus(const ShipStatus& status) {
    ship_status_ = status;
}

void UIManager::SetTargetInfo(const TargetInfo& target) {
    target_info_ = target;
}

void UIManager::AddCombatLogMessage(const std::string& message) {
    combat_log_.push_back(message);
    if (combat_log_.size() > MAX_COMBAT_LOG_MESSAGES) {
        combat_log_.erase(combat_log_.begin());
    }
}

void UIManager::SetPlayerPosition(const glm::vec3& position) {
    m_playerPosition = position;
}

// ============================================================================
// Target Management
// ============================================================================

void UIManager::UpdateTargets(
    const std::unordered_map<std::string, std::shared_ptr<::eve::Entity>>& entities)
{
    if (m_targetList) {
        m_targetList->updateTargets(entities);
    }
    if (m_overviewPanel) {
        m_overviewPanel->UpdateEntities(entities, m_playerPosition);
    }
}

void UIManager::AddTarget(const std::string& entityId) {
    if (m_targetList) {
        m_targetList->addTarget(entityId);
    }
}

void UIManager::RemoveTarget(const std::string& entityId) {
    if (m_targetList) {
        m_targetList->removeTarget(entityId);
    }
}

// ============================================================================
// Panel Visibility
// ============================================================================

void UIManager::SetPanelVisible(const std::string& panel_name, bool visible) {
    if (panel_name == "ship_status") {
        show_ship_status_ = visible;
    } else if (panel_name == "target_info") {
        show_target_info_ = visible;
    } else if (panel_name == "speed") {
        show_speed_panel_ = visible;
    } else if (panel_name == "combat_log") {
        show_combat_log_ = visible;
    } else {
        auto it = m_panelConfigs.find(panel_name);
        if (it != m_panelConfigs.end()) {
            it->second.state.open = visible;
        }
    }
}

bool UIManager::IsPanelVisible(const std::string& panel_name) const {
    if (panel_name == "ship_status") return show_ship_status_;
    if (panel_name == "target_info") return show_target_info_;
    if (panel_name == "speed") return show_speed_panel_;
    if (panel_name == "combat_log") return show_combat_log_;

    auto it = m_panelConfigs.find(panel_name);
    if (it != m_panelConfigs.end()) {
        return it->second.state.open;
    }
    return false;
}

// ============================================================================
// Panel Toggles
// ============================================================================

void UIManager::ToggleInventory() {
    auto it = m_panelConfigs.find("inventory");
    if (it != m_panelConfigs.end()) it->second.state.open = !it->second.state.open;
}

void UIManager::ToggleFitting() {
    auto it = m_panelConfigs.find("fitting");
    if (it != m_panelConfigs.end()) it->second.state.open = !it->second.state.open;
}

void UIManager::ToggleMission() {
    auto it = m_panelConfigs.find("mission");
    if (it != m_panelConfigs.end()) it->second.state.open = !it->second.state.open;
}

void UIManager::ToggleOverview() {
    auto it = m_panelConfigs.find("overview");
    if (it != m_panelConfigs.end()) it->second.state.open = !it->second.state.open;
}

void UIManager::ToggleMarket() {
    auto it = m_panelConfigs.find("market");
    if (it != m_panelConfigs.end()) it->second.state.open = !it->second.state.open;
}

void UIManager::ToggleDScan() {
    auto it = m_panelConfigs.find("dscan");
    if (it != m_panelConfigs.end()) it->second.state.open = !it->second.state.open;
}

void UIManager::ToggleMap() {
    m_showStarMap = !m_showStarMap;
}

void UIManager::ToggleChat() {
    auto it = m_panelConfigs.find("chat");
    if (it != m_panelConfigs.end()) it->second.state.open = !it->second.state.open;
}

void UIManager::ToggleDrones() {
    auto it = m_panelConfigs.find("drones");
    if (it != m_panelConfigs.end()) it->second.state.open = !it->second.state.open;
}

void UIManager::ToggleProbeScanner() {
    auto it = m_panelConfigs.find("probe_scanner");
    if (it != m_panelConfigs.end()) it->second.state.open = !it->second.state.open;
}

// ============================================================================
// Interface Lock
// ============================================================================

void UIManager::SetInterfaceLocked(bool locked) {
    m_interfaceLocked = locked;
}

bool UIManager::IsInterfaceLocked() const {
    return m_interfaceLocked;
}

void UIManager::ToggleInterfaceLock() {
    m_interfaceLocked = !m_interfaceLocked;
}

// ============================================================================
// Selected Item
// ============================================================================

void UIManager::SetSelectedItem(const SelectedItemData& item) {
    m_selectedItem = item;
}

void UIManager::ClearSelectedItem() {
    m_selectedItem = SelectedItemData();
}

// ============================================================================
// HUD Alert Management
// ============================================================================

void UIManager::AddAlert(const std::string& message, HUDAlertPriority priority,
                         float duration)
{
    m_alerts.erase(
        std::remove_if(m_alerts.begin(), m_alerts.end(),
            [&message](const HUDAlert& a) { return a.message == message; }),
        m_alerts.end());

    m_alerts.emplace_back(message, priority, duration);

    std::sort(m_alerts.begin(), m_alerts.end(),
              [](const HUDAlert& a, const HUDAlert& b) {
                  return static_cast<int>(a.priority) > static_cast<int>(b.priority);
              });

    if (m_alerts.size() > MAX_ALERTS) {
        m_alerts.resize(MAX_ALERTS);
    }
}

void UIManager::ClearAlerts() {
    m_alerts.clear();
}

void UIManager::UpdateAlerts(float deltaTime) {
    for (auto& alert : m_alerts) {
        alert.elapsed += deltaTime;
    }
    m_alerts.erase(
        std::remove_if(m_alerts.begin(), m_alerts.end(),
            [](const HUDAlert& a) { return a.elapsed >= a.duration; }),
        m_alerts.end());
}

// ============================================================================
// Module Rack Data Binding
// ============================================================================

void UIManager::SetModuleSlots(const ModuleSlotState slots[], int count) {
    m_moduleSlotCount = std::min(count, MAX_MODULE_SLOTS);
    for (int i = 0; i < m_moduleSlotCount; ++i) {
        m_moduleSlots[i] = slots[i];
    }
}

// ============================================================================
// Compact Mode
// ============================================================================

void UIManager::SetCompactMode(bool enabled) {
    m_compactMode = enabled;
    // Atlas Theme metrics are used at render time via PanelFlags::compactMode
}

void UIManager::ToggleCompactMode() {
    SetCompactMode(!m_compactMode);
}

// ============================================================================
// Per-Panel Opacity (Phase 4.10)
// ============================================================================

void UIManager::SetPanelOpacity(const std::string& panel_name, float opacity) {
    auto it = m_panelConfigs.find(panel_name);
    if (it != m_panelConfigs.end()) {
        it->second.opacity = std::max(0.15f, std::min(1.0f, opacity));
    }
}

float UIManager::GetPanelOpacity(const std::string& panel_name) const {
    auto it = m_panelConfigs.find(panel_name);
    if (it != m_panelConfigs.end()) {
        return it->second.opacity;
    }
    return 0.92f;
}

// ============================================================================
// Layout Management (Phase 4.10)
// ============================================================================

std::unordered_map<std::string, PanelLayout> UIManager::ExportPanelLayouts() const {
    std::unordered_map<std::string, PanelLayout> layouts;
    for (const auto& [id, cfg] : m_panelConfigs) {
        PanelLayout pl;
        pl.id = id;
        pl.x = cfg.state.bounds.x;
        pl.y = cfg.state.bounds.y;
        pl.w = cfg.state.bounds.w;
        pl.h = cfg.state.bounds.h;
        pl.visible = cfg.state.open;
        pl.minimized = cfg.state.minimized;
        pl.opacity = cfg.opacity;
        layouts[id] = pl;
    }
    return layouts;
}

void UIManager::ImportPanelLayouts(const std::unordered_map<std::string, PanelLayout>& layouts) {
    for (const auto& [id, pl] : layouts) {
        auto it = m_panelConfigs.find(id);
        if (it != m_panelConfigs.end()) {
            it->second.state.bounds = atlas::Rect(pl.x, pl.y, pl.w, pl.h);
            it->second.state.open = pl.visible;
            it->second.state.minimized = pl.minimized;
            it->second.opacity = pl.opacity;
        }
    }
}

bool UIManager::SaveLayout(const std::string& presetName) {
    auto layouts = ExportPanelLayouts();
    bool ok = m_layoutManager.SaveLayout(presetName, layouts);
    if (ok) {
        m_activeLayoutName = presetName;
    }
    return ok;
}

bool UIManager::LoadLayout(const std::string& presetName) {
    std::unordered_map<std::string, PanelLayout> layouts;
    if (!m_layoutManager.LoadLayout(presetName, layouts)) {
        return false;
    }
    ImportPanelLayouts(layouts);
    m_activeLayoutName = presetName;
    return true;
}

std::vector<std::string> UIManager::GetAvailableLayouts() const {
    return m_layoutManager.GetAvailablePresets();
}

void UIManager::ResetToDefaultLayout() {
    InitPanelConfigs(m_windowW, m_windowH);
    m_activeLayoutName = "default";
}

} // namespace UI
