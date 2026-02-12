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

        // Dispatch to panel-specific rendering based on panel ID
        if (id == "inventory") {
            RenderInventoryContent(x, y, contentW, b.bottom() - theme.padding);
        } else if (id == "fitting") {
            RenderFittingContent(x, y, contentW, b.bottom() - theme.padding);
        } else if (id == "mission") {
            RenderMissionContent(x, y, contentW, b.bottom() - theme.padding);
        } else if (id == "dscan") {
            RenderDScanContent(x, y, contentW, b.bottom() - theme.padding);
        } else if (id == "chat") {
            RenderChatContent(x, y, contentW, b.bottom() - theme.padding);
        } else if (id == "drones") {
            RenderDroneContent(x, y, contentW, b.bottom() - theme.padding);
        } else if (id == "market") {
            RenderMarketContent(x, y, contentW, b.bottom() - theme.padding);
        } else if (id == "probe_scanner") {
            RenderProbeScannerContent(x, y, contentW, b.bottom() - theme.padding);
        } else {
            atlas::label(m_ctx, atlas::Vec2(x, y), cfg.title, theme.textPrimary);
            y += 20.0f;
            atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
        }
    }
    atlas::panelEnd(m_ctx);
}

// ============================================================================
// Panel-Specific Content Rendering (Atlas native)
// ============================================================================

void UIManager::RenderInventoryContent(float x, float y, float contentW, float maxY) {
    const auto& theme = m_ctx.theme();
    auto& r = m_ctx.renderer();

    if (!m_inventoryPanel) return;
    const auto& data = m_inventoryPanel->GetData();

    // Cargo capacity bar
    atlas::label(m_ctx, atlas::Vec2(x, y), "Cargo Hold", theme.textPrimary);
    y += 18.0f;
    float capFrac = data.cargo_capacity > 0 ? data.cargo_used / data.cargo_capacity : 0.0f;
    atlas::Rect capBar(x, y, contentW, 14.0f);
    r.drawProgressBar(capBar, capFrac, theme.accentPrimary, theme.bgHeader);
    char capBuf[64];
    std::snprintf(capBuf, sizeof(capBuf), "%.0f / %.0f m3", data.cargo_used, data.cargo_capacity);
    r.drawText(capBuf, atlas::Vec2(x + 4, y + 1), theme.textPrimary, 1.0f);
    y += 22.0f;

    atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
    y += 8.0f;

    // Item list
    const auto& items = data.cargo_items;
    if (items.empty()) {
        atlas::label(m_ctx, atlas::Vec2(x, y), "Cargo hold is empty", theme.textSecondary);
    } else {
        // Column headers
        r.drawText("Item", atlas::Vec2(x, y), theme.textSecondary, 1.0f);
        r.drawText("Qty", atlas::Vec2(x + contentW * 0.6f, y), theme.textSecondary, 1.0f);
        r.drawText("Vol", atlas::Vec2(x + contentW * 0.8f, y), theme.textSecondary, 1.0f);
        y += 16.0f;
        atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
        y += 4.0f;

        for (size_t i = 0; i < items.size() && y < maxY - 16.0f; ++i) {
            const auto& item = items[i];
            if (i % 2 == 1) {
                r.drawRect(atlas::Rect(x, y, contentW, 16.0f), theme.bgHeader.withAlpha(0.3f));
            }
            r.drawText(item.name, atlas::Vec2(x + 2, y + 1), theme.textPrimary, 1.0f);
            char qtyBuf[16];
            std::snprintf(qtyBuf, sizeof(qtyBuf), "%d", item.quantity);
            r.drawText(qtyBuf, atlas::Vec2(x + contentW * 0.6f, y + 1), theme.textPrimary, 1.0f);
            char volBuf[16];
            std::snprintf(volBuf, sizeof(volBuf), "%.1f", item.volume * item.quantity);
            r.drawText(volBuf, atlas::Vec2(x + contentW * 0.8f, y + 1), theme.textSecondary, 1.0f);
            y += 16.0f;
        }
    }
}

void UIManager::RenderFittingContent(float x, float y, float contentW, float maxY) {
    const auto& theme = m_ctx.theme();
    auto& r = m_ctx.renderer();

    if (!m_fittingPanel) return;
    const auto& data = m_fittingPanel->GetData();

    // Ship name and type
    r.drawText(data.ship_name, atlas::Vec2(x, y), theme.accentPrimary, 1.0f);
    y += 16.0f;
    r.drawText(data.ship_type, atlas::Vec2(x, y), theme.textSecondary, 1.0f);
    y += 22.0f;
    atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
    y += 8.0f;

    // CPU bar
    float cpuFrac = data.cpu_max > 0 ? data.cpu_used / data.cpu_max : 0.0f;
    r.drawText("CPU", atlas::Vec2(x, y), theme.textSecondary, 1.0f);
    atlas::Rect cpuBar(x + 40.0f, y, contentW - 40.0f, 12.0f);
    atlas::Color cpuColor = cpuFrac > 0.9f ? theme.danger : theme.accentPrimary;
    r.drawProgressBar(cpuBar, cpuFrac, cpuColor, theme.bgHeader);
    char cpuBuf[64];
    std::snprintf(cpuBuf, sizeof(cpuBuf), "%.0f / %.0f tf", data.cpu_used, data.cpu_max);
    r.drawText(cpuBuf, atlas::Vec2(x + 44.0f, y), theme.textPrimary, 1.0f);
    y += 20.0f;

    // Powergrid bar
    float pgFrac = data.powergrid_max > 0 ? data.powergrid_used / data.powergrid_max : 0.0f;
    r.drawText("PG", atlas::Vec2(x, y), theme.textSecondary, 1.0f);
    atlas::Rect pgBar(x + 40.0f, y, contentW - 40.0f, 12.0f);
    atlas::Color pgColor = pgFrac > 0.9f ? theme.danger : theme.success;
    r.drawProgressBar(pgBar, pgFrac, pgColor, theme.bgHeader);
    char pgBuf[64];
    std::snprintf(pgBuf, sizeof(pgBuf), "%.0f / %.0f MW", data.powergrid_used, data.powergrid_max);
    r.drawText(pgBuf, atlas::Vec2(x + 44.0f, y), theme.textPrimary, 1.0f);
    y += 24.0f;

    atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
    y += 8.0f;

    // Slot sections
    auto renderSlots = [&](const char* title, const auto& slots, int maxSlots) {
        if (y > maxY - 20.0f) return;
        r.drawText(title, atlas::Vec2(x, y), theme.textSecondary, 1.0f);
        y += 16.0f;
        for (int i = 0; i < maxSlots && y < maxY - 16.0f; ++i) {
            if (slots[i].has_value()) {
                const auto& mod = slots[i].value();
                atlas::Color modColor = mod.is_online ? theme.accentPrimary : theme.textSecondary;
                r.drawText(mod.name, atlas::Vec2(x + 10.0f, y), modColor, 1.0f);
            } else {
                r.drawText("[empty]", atlas::Vec2(x + 10.0f, y), theme.bgHeader, 1.0f);
            }
            y += 14.0f;
        }
        y += 4.0f;
    };

    renderSlots("High Slots", data.high_slots, 8);
    renderSlots("Mid Slots", data.mid_slots, 8);
    renderSlots("Low Slots", data.low_slots, 8);
}

void UIManager::RenderMissionContent(float x, float y, float contentW, float maxY) {
    const auto& theme = m_ctx.theme();
    auto& r = m_ctx.renderer();

    if (!m_missionPanel) return;
    const auto& data = m_missionPanel->GetData();

    if (!data.is_active) {
        atlas::label(m_ctx, atlas::Vec2(x, y), "No Active Mission", theme.textSecondary);
        y += 20.0f;
        atlas::label(m_ctx, atlas::Vec2(x, y), "Visit an agent to accept a mission", theme.textSecondary);
        return;
    }

    // Mission title
    r.drawText(data.mission_name, atlas::Vec2(x, y), theme.accentPrimary, 1.0f);
    y += 18.0f;

    // Mission type and level
    char infoBuf[128];
    std::snprintf(infoBuf, sizeof(infoBuf), "Level %d %s", data.level, data.mission_type.c_str());
    r.drawText(infoBuf, atlas::Vec2(x, y), theme.textSecondary, 1.0f);
    y += 16.0f;

    if (!data.agent_name.empty()) {
        char agentBuf[128];
        std::snprintf(agentBuf, sizeof(agentBuf), "Agent: %s", data.agent_name.c_str());
        r.drawText(agentBuf, atlas::Vec2(x, y), theme.textSecondary, 1.0f);
        y += 16.0f;
    }

    y += 4.0f;
    atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
    y += 8.0f;

    // Objectives
    r.drawText("Objectives:", atlas::Vec2(x, y), theme.textPrimary, 1.0f);
    y += 16.0f;
    for (const auto& obj : data.objectives) {
        if (y > maxY - 20.0f) break;
        atlas::Color objColor = obj.completed ? theme.success : theme.textSecondary;
        const char* marker = obj.completed ? "[x] " : "[ ] ";
        char objBuf[256];
        std::snprintf(objBuf, sizeof(objBuf), "%s%s", marker, obj.description.c_str());
        r.drawText(objBuf, atlas::Vec2(x + 8.0f, y), objColor, 1.0f);
        y += 14.0f;
    }

    y += 8.0f;
    atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
    y += 8.0f;

    // Rewards
    if (y < maxY - 40.0f) {
        r.drawText("Rewards:", atlas::Vec2(x, y), theme.textPrimary, 1.0f);
        y += 16.0f;
        if (data.isk_reward > 0) {
            char iskBuf[64];
            std::snprintf(iskBuf, sizeof(iskBuf), "ISK: %.0f", data.isk_reward);
            r.drawText(iskBuf, atlas::Vec2(x + 8.0f, y), theme.warning, 1.0f);
            y += 14.0f;
        }
        if (data.lp_reward > 0) {
            char lpBuf[64];
            std::snprintf(lpBuf, sizeof(lpBuf), "LP: %.0f", data.lp_reward);
            r.drawText(lpBuf, atlas::Vec2(x + 8.0f, y), theme.accentSecondary, 1.0f);
            y += 14.0f;
        }
    }
}

void UIManager::RenderDScanContent(float x, float y, float contentW, float maxY) {
    const auto& theme = m_ctx.theme();
    auto& r = m_ctx.renderer();

    if (!m_dscanPanel) return;
    const auto& results = m_dscanPanel->GetResults();

    // Scan controls
    char angleBuf[32];
    std::snprintf(angleBuf, sizeof(angleBuf), "Angle: %.0f deg", m_dscanPanel->GetScanAngle());
    r.drawText(angleBuf, atlas::Vec2(x, y), theme.textSecondary, 1.0f);
    y += 16.0f;

    char rangeBuf[32];
    std::snprintf(rangeBuf, sizeof(rangeBuf), "Range: %.1f AU", m_dscanPanel->GetScanRange());
    r.drawText(rangeBuf, atlas::Vec2(x, y), theme.textSecondary, 1.0f);
    y += 20.0f;

    // Scan button
    atlas::Rect scanBtn(x, y, 80.0f, 22.0f);
    if (atlas::button(m_ctx, "SCAN", scanBtn)) {
        if (m_dscanPanel) m_dscanPanel->ConsumesScanRequest();
    }
    y += 30.0f;

    atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
    y += 8.0f;

    // Results header
    char countBuf[32];
    std::snprintf(countBuf, sizeof(countBuf), "Results: %d", static_cast<int>(results.size()));
    r.drawText(countBuf, atlas::Vec2(x, y), theme.textPrimary, 1.0f);
    y += 18.0f;

    if (results.empty()) {
        atlas::label(m_ctx, atlas::Vec2(x, y), "No scan results", theme.textSecondary);
    } else {
        // Column headers
        r.drawText("Name", atlas::Vec2(x, y), theme.textSecondary, 1.0f);
        r.drawText("Type", atlas::Vec2(x + contentW * 0.5f, y), theme.textSecondary, 1.0f);
        r.drawText("Dist", atlas::Vec2(x + contentW * 0.8f, y), theme.textSecondary, 1.0f);
        y += 16.0f;
        atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
        y += 4.0f;

        for (size_t i = 0; i < results.size() && y < maxY - 16.0f; ++i) {
            const auto& res = results[i];
            if (i % 2 == 1) {
                r.drawRect(atlas::Rect(x, y, contentW, 16.0f), theme.bgHeader.withAlpha(0.3f));
            }
            r.drawText(res.name, atlas::Vec2(x + 2, y + 1), theme.textPrimary, 1.0f);
            r.drawText(res.type, atlas::Vec2(x + contentW * 0.5f, y + 1), theme.textSecondary, 1.0f);
            char distBuf[32];
            std::snprintf(distBuf, sizeof(distBuf), "%.1f AU", res.distance);
            r.drawText(distBuf, atlas::Vec2(x + contentW * 0.8f, y + 1), theme.textSecondary, 1.0f);
            y += 16.0f;
        }
    }
}

void UIManager::RenderChatContent(float x, float y, float contentW, float maxY) {
    const auto& theme = m_ctx.theme();
    auto& r = m_ctx.renderer();

    if (!m_chatPanel) return;
    const auto& channels = m_chatPanel->GetChannels();
    const auto& activeId = m_chatPanel->GetActiveChannel();
    const auto& allMsgs = m_chatPanel->GetAllMessages();

    // Channel tabs
    if (!channels.empty()) {
        float tabX = x;
        for (const auto& ch : channels) {
            bool isActive = (ch.channel_id == activeId);
            atlas::Color tabColor = isActive ? theme.accentPrimary : theme.textSecondary;
            float textW = r.measureText(ch.channel_name);
            if (isActive) {
                r.drawRect(atlas::Rect(tabX, y, textW + 8, 18.0f), theme.bgHeader);
            }
            r.drawText(ch.channel_name, atlas::Vec2(tabX + 4, y + 2), tabColor, 1.0f);
            tabX += textW + 16.0f;
        }
        y += 22.0f;
        atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
        y += 4.0f;
    }

    // Messages for active channel
    auto msgIt = allMsgs.find(activeId);
    if (msgIt != allMsgs.end() && !msgIt->second.empty()) {
        const auto& messages = msgIt->second;
        // Show last N messages that fit
        int startIdx = static_cast<int>(messages.size()) - 1;
        float msgY = maxY - 30.0f;  // reserve space for input area
        std::vector<int> visibleIdxs;
        for (int i = startIdx; i >= 0 && msgY > y; --i) {
            visibleIdxs.push_back(i);
            msgY -= 14.0f;
        }

        // Render in forward order
        float drawY = y;
        for (auto rit = visibleIdxs.rbegin(); rit != visibleIdxs.rend(); ++rit) {
            const auto& msg = messages[*rit];
            atlas::Color nameColor;
            switch (msg.sender_type) {
                case ChatMessage::SenderType::Self:     nameColor = theme.accentPrimary; break;
                case ChatMessage::SenderType::Hostile:  nameColor = theme.danger; break;
                case ChatMessage::SenderType::Friendly: nameColor = theme.success; break;
                case ChatMessage::SenderType::System:   nameColor = theme.warning; break;
                default:                                nameColor = theme.textSecondary; break;
            }
            char lineBuf[256];
            std::snprintf(lineBuf, sizeof(lineBuf), "[%s] %s: %s",
                          msg.timestamp.c_str(), msg.sender_name.c_str(), msg.content.c_str());
            r.drawText(lineBuf, atlas::Vec2(x + 2, drawY), nameColor, 1.0f);
            drawY += 14.0f;
        }
    } else {
        atlas::label(m_ctx, atlas::Vec2(x, y), "No messages", theme.textSecondary);
    }
}

void UIManager::RenderDroneContent(float x, float y, float contentW, float maxY) {
    const auto& theme = m_ctx.theme();
    auto& r = m_ctx.renderer();

    if (!m_droneControlPanel) return;
    const auto& data = m_droneControlPanel->GetData();

    // Bandwidth
    float bwFrac = data.max_bandwidth > 0 ? static_cast<float>(data.used_bandwidth) / data.max_bandwidth : 0.0f;
    r.drawText("Bandwidth", atlas::Vec2(x, y), theme.textSecondary, 1.0f);
    y += 16.0f;
    atlas::Rect bwBar(x, y, contentW, 12.0f);
    r.drawProgressBar(bwBar, bwFrac, theme.accentPrimary, theme.bgHeader);
    char bwBuf[32];
    std::snprintf(bwBuf, sizeof(bwBuf), "%d / %d Mbit/s", data.used_bandwidth, data.max_bandwidth);
    r.drawText(bwBuf, atlas::Vec2(x + 4, y), theme.textPrimary, 1.0f);
    y += 20.0f;

    // Bay capacity
    float bayFrac = data.bay_capacity > 0 ? data.bay_used / data.bay_capacity : 0.0f;
    r.drawText("Drone Bay", atlas::Vec2(x, y), theme.textSecondary, 1.0f);
    y += 16.0f;
    atlas::Rect bayBar(x, y, contentW, 12.0f);
    r.drawProgressBar(bayBar, bayFrac, theme.accentSecondary, theme.bgHeader);
    char bayBuf[32];
    std::snprintf(bayBuf, sizeof(bayBuf), "%.0f / %.0f m3", data.bay_used, data.bay_capacity);
    r.drawText(bayBuf, atlas::Vec2(x + 4, y), theme.textPrimary, 1.0f);
    y += 24.0f;

    atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
    y += 8.0f;

    // Drones in space
    char spaceBuf[32];
    std::snprintf(spaceBuf, sizeof(spaceBuf), "In Space (%d)", static_cast<int>(data.space_drones.size()));
    r.drawText(spaceBuf, atlas::Vec2(x, y), theme.textPrimary, 1.0f);
    y += 16.0f;
    for (const auto& drone : data.space_drones) {
        if (y > maxY - 16.0f) break;
        float hpFrac = drone.max_hitpoints > 0 ? drone.hitpoints / drone.max_hitpoints : 0.0f;
        atlas::Color droneColor = drone.is_engaging ? theme.danger : theme.accentPrimary;
        r.drawText(drone.name, atlas::Vec2(x + 8.0f, y), droneColor, 1.0f);
        // Mini HP bar
        atlas::Rect hpBar(x + contentW - 60.0f, y + 2, 50.0f, 8.0f);
        r.drawProgressBar(hpBar, hpFrac, theme.success, theme.bgHeader);
        y += 14.0f;
    }

    y += 8.0f;

    // Drones in bay
    if (y < maxY - 30.0f) {
        char bayLabel[32];
        std::snprintf(bayLabel, sizeof(bayLabel), "In Bay (%d)", static_cast<int>(data.bay_drones.size()));
        r.drawText(bayLabel, atlas::Vec2(x, y), theme.textPrimary, 1.0f);
        y += 16.0f;
        for (const auto& drone : data.bay_drones) {
            if (y > maxY - 16.0f) break;
            r.drawText(drone.name, atlas::Vec2(x + 8.0f, y), theme.textSecondary, 1.0f);
            y += 14.0f;
        }
    }
}

void UIManager::RenderMarketContent(float x, float y, float contentW, float maxY) {
    const auto& theme = m_ctx.theme();
    auto& r = m_ctx.renderer();

    if (!m_marketPanel) return;
    const auto& buyOrders = m_marketPanel->GetBuyOrders();
    const auto& sellOrders = m_marketPanel->GetSellOrders();

    // Sell orders section
    r.drawText("Sell Orders", atlas::Vec2(x, y), theme.danger, 1.0f);
    y += 18.0f;

    if (sellOrders.empty()) {
        atlas::label(m_ctx, atlas::Vec2(x + 8, y), "No sell orders", theme.textSecondary);
        y += 16.0f;
    } else {
        // Headers
        r.drawText("Item", atlas::Vec2(x, y), theme.textSecondary, 1.0f);
        r.drawText("Price", atlas::Vec2(x + contentW * 0.5f, y), theme.textSecondary, 1.0f);
        r.drawText("Qty", atlas::Vec2(x + contentW * 0.8f, y), theme.textSecondary, 1.0f);
        y += 16.0f;
        atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
        y += 4.0f;

        for (size_t i = 0; i < sellOrders.size() && y < maxY * 0.5f; ++i) {
            const auto& order = sellOrders[i];
            r.drawText(order.item_name, atlas::Vec2(x + 2, y), theme.textPrimary, 1.0f);
            char priceBuf[32];
            std::snprintf(priceBuf, sizeof(priceBuf), "%.2f", order.price);
            r.drawText(priceBuf, atlas::Vec2(x + contentW * 0.5f, y), theme.danger, 1.0f);
            char qtyBuf[16];
            std::snprintf(qtyBuf, sizeof(qtyBuf), "%d", order.quantity);
            r.drawText(qtyBuf, atlas::Vec2(x + contentW * 0.8f, y), theme.textSecondary, 1.0f);
            y += 14.0f;
        }
    }

    y += 8.0f;
    atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
    y += 8.0f;

    // Buy orders section
    r.drawText("Buy Orders", atlas::Vec2(x, y), theme.success, 1.0f);
    y += 18.0f;

    if (buyOrders.empty()) {
        atlas::label(m_ctx, atlas::Vec2(x + 8, y), "No buy orders", theme.textSecondary);
    } else {
        r.drawText("Item", atlas::Vec2(x, y), theme.textSecondary, 1.0f);
        r.drawText("Price", atlas::Vec2(x + contentW * 0.5f, y), theme.textSecondary, 1.0f);
        r.drawText("Qty", atlas::Vec2(x + contentW * 0.8f, y), theme.textSecondary, 1.0f);
        y += 16.0f;
        atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
        y += 4.0f;

        for (size_t i = 0; i < buyOrders.size() && y < maxY - 16.0f; ++i) {
            const auto& order = buyOrders[i];
            r.drawText(order.item_name, atlas::Vec2(x + 2, y), theme.textPrimary, 1.0f);
            char priceBuf[32];
            std::snprintf(priceBuf, sizeof(priceBuf), "%.2f", order.price);
            r.drawText(priceBuf, atlas::Vec2(x + contentW * 0.5f, y), theme.success, 1.0f);
            char qtyBuf[16];
            std::snprintf(qtyBuf, sizeof(qtyBuf), "%d", order.quantity);
            r.drawText(qtyBuf, atlas::Vec2(x + contentW * 0.8f, y), theme.textSecondary, 1.0f);
            y += 14.0f;
        }
    }
}

void UIManager::RenderProbeScannerContent(float x, float y, float contentW, float maxY) {
    const auto& theme = m_ctx.theme();
    auto& r = m_ctx.renderer();

    if (!m_probeScannerPanel) return;

    // Probe deployment info
    char probeBuf[64];
    std::snprintf(probeBuf, sizeof(probeBuf), "Probes: %d deployed",
                  m_probeScannerPanel->GetProbeCount());
    r.drawText(probeBuf, atlas::Vec2(x, y), theme.textPrimary, 1.0f);
    y += 16.0f;
    char rangeBuf[64];
    std::snprintf(rangeBuf, sizeof(rangeBuf), "Scan Range: %.1f AU",
                  m_probeScannerPanel->GetProbeRange());
    r.drawText(rangeBuf, atlas::Vec2(x, y), theme.textSecondary, 1.0f);
    y += 20.0f;

    // Scan button
    atlas::Rect scanBtn(x, y, 100.0f, 22.0f);
    if (atlas::button(m_ctx, "Analyze", scanBtn)) {
        if (m_probeScannerPanel->ConsumesScanRequest()) {
            // Callback handled internally
        }
    }
    y += 30.0f;

    atlas::separator(m_ctx, atlas::Vec2(x, y), contentW);
    y += 8.0f;

    atlas::label(m_ctx, atlas::Vec2(x, y), "Deploy probes and scan to find signatures", theme.textSecondary);
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
    const std::unordered_map<std::string, std::shared_ptr<::atlas::Entity>>& entities)
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

// ============================================================================
// UI Scale (Phase 4.10)
// ============================================================================

void UIManager::SetUIScale(float scale) {
    m_uiScale = std::max(0.5f, std::min(2.0f, scale));
    std::cout << "[UIManager] UI scale set to " << m_uiScale << std::endl;
}

// ============================================================================
// Color Scheme (Phase 4.10)
// ============================================================================

void UIManager::SetColorScheme(ColorScheme scheme) {
    m_colorScheme = scheme;

    atlas::Theme theme = m_ctx.theme();
    switch (scheme) {
        case ColorScheme::CLASSIC:
            // Warmer, amber-tinted classic theme
            theme.accentPrimary   = atlas::Color(0.90f, 0.75f, 0.30f, 1.0f);
            theme.accentSecondary = atlas::Color(0.95f, 0.85f, 0.45f, 1.0f);
            theme.accentDim       = atlas::Color(0.40f, 0.35f, 0.15f, 1.0f);
            theme.bgPanel         = atlas::Color(0.06f, 0.05f, 0.04f, 0.95f);
            theme.bgHeader        = atlas::Color(0.10f, 0.08f, 0.06f, 1.0f);
            theme.textPrimary     = atlas::Color(0.93f, 0.90f, 0.82f, 1.0f);
            theme.textSecondary   = atlas::Color(0.65f, 0.60f, 0.50f, 1.0f);
            std::cout << "[UIManager] Color scheme set to Classic" << std::endl;
            break;

        case ColorScheme::COLORBLIND:
            // Deuteranopia-safe: blue/orange instead of green/red
            theme.accentPrimary   = atlas::Color(0.20f, 0.60f, 1.00f, 1.0f);
            theme.accentSecondary = atlas::Color(0.40f, 0.75f, 1.00f, 1.0f);
            theme.accentDim       = atlas::Color(0.10f, 0.30f, 0.50f, 1.0f);
            theme.success         = atlas::Color(0.20f, 0.60f, 1.00f, 1.0f);
            theme.warning         = atlas::Color(1.00f, 0.70f, 0.20f, 1.0f);
            theme.danger          = atlas::Color(1.00f, 0.50f, 0.00f, 1.0f);
            std::cout << "[UIManager] Color scheme set to Colorblind" << std::endl;
            break;

        case ColorScheme::DEFAULT:
        default:
            // Reset to default Atlas theme (teal/cyan)
            theme = atlas::Theme{};
            std::cout << "[UIManager] Color scheme set to Default" << std::endl;
            break;
    }
    m_ctx.setTheme(theme);
}

} // namespace UI
