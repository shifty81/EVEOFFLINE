#include "ui/atlas/atlas_hud.h"

#include <algorithm>
#include <cstdio>

namespace atlas {

AtlasHUD::AtlasHUD() = default;
AtlasHUD::~AtlasHUD() = default;

// ── Overview tab → entity-type filter (PvE-focused) ──────────────────
//
// Travel:   Stations, Stargates, Planets, Moons, Wormholes, Celestials —
//           anything you can warp/travel to.
// Combat:   Frigates, Cruisers, Battleships, and other NPC ship types —
//           hostile or neutral ships on grid.
// Industry: Asteroids, Asteroid Belts, and mining-related objects.

bool AtlasHUD::matchesOverviewTab(const std::string& tab, const std::string& entityType) {
    if (tab == "Travel") {
        return entityType == "Station"
            || entityType == "Stargate"
            || entityType == "Planet"
            || entityType == "Moon"
            || entityType == "Wormhole"
            || entityType == "Celestial"
            || entityType == "Beacon"
            || entityType == "Citadel";
    }
    if (tab == "Combat") {
        return entityType == "Frigate"
            || entityType == "Cruiser"
            || entityType == "Battleship"
            || entityType == "Destroyer"
            || entityType == "Battlecruiser"
            || entityType == "Carrier"
            || entityType == "Dreadnought"
            || entityType == "npc"
            || entityType == "hostile"
            || entityType == "friendly"
            || entityType == "fleet";
    }
    if (tab == "Industry") {
        return entityType == "Asteroid"
            || entityType == "Asteroid Belt"
            || entityType == "Ice"
            || entityType == "Gas Cloud"
            || entityType == "Mining Barge"
            || entityType == "Wreck"
            || entityType == "Container";
    }
    // Unknown tab: show everything (fallback)
    return true;
}

void AtlasHUD::init(int windowW, int windowH) {
    float w = static_cast<float>(windowW);
    float h = static_cast<float>(windowH);

    // Overview panel: right side, ~300px wide, below selected-item panel
    m_overviewState.bounds = {w - 310.0f, 180.0f, 300.0f, h - 280.0f};
    m_overviewState.open = true;
    m_overviewState.minimized = false;

    // Selected item panel: top-right, ~300×120
    m_selectedItemState.bounds = {w - 310.0f, 10.0f, 300.0f, 120.0f};
    m_selectedItemState.open = true;
    m_selectedItemState.minimized = false;

    // Info panel: centre-left, ~280×260
    m_infoPanelState.bounds = {60.0f, 100.0f, 280.0f, 260.0f};
    m_infoPanelState.open = false;
    m_infoPanelState.minimized = false;

    // Dockable panels (opened via Neocom sidebar)
    m_inventoryState.bounds = {50.0f, 300.0f, 350.0f, 400.0f};
    m_inventoryState.open = false;

    m_fittingState.bounds = {420.0f, 300.0f, 400.0f, 450.0f};
    m_fittingState.open = false;

    m_marketState.bounds = {420.0f, 50.0f, 450.0f, 500.0f};
    m_marketState.open = false;

    m_missionState.bounds = {50.0f, 50.0f, 400.0f, 350.0f};
    m_missionState.open = false;

    m_dscanState.bounds = {w - 360.0f, 460.0f, 350.0f, 300.0f};
    m_dscanState.open = false;

    m_chatState.bounds = {60.0f, 420.0f, 380.0f, 300.0f};
    m_chatState.open = false;

    m_dronePanelState.bounds = {60.0f, 300.0f, 320.0f, 400.0f};
    m_dronePanelState.open = false;

    m_probeScannerState.bounds = {420.0f, 300.0f, 380.0f, 420.0f};
    m_probeScannerState.open = false;

    m_stationState.bounds = {420.0f, 100.0f, 340.0f, 420.0f};
    m_stationState.open = false;

    m_fleetState.bounds = {60.0f, 100.0f, 320.0f, 380.0f};
    m_fleetState.open = false;
}

void AtlasHUD::update(AtlasContext& ctx,
                       const ShipHUDData& ship,
                       const std::vector<TargetCardInfo>& targets,
                       const std::vector<OverviewEntry>& overview,
                       const SelectedItemInfo& selectedItem) {
    // Draw elements in back-to-front order

    // Inform the context about sidebar width so panels clamp to it
    ctx.setSidebarWidth(m_sidebarWidth);

    // 1. Sidebar (left edge) — pass active panel states for icon highlighting
    bool activeIcons[8] = {
        m_inventoryState.open,   // 0: Inventory
        m_fittingState.open,     // 1: Fitting
        m_marketState.open,      // 2: Market
        m_missionState.open,     // 3: Missions
        m_dscanState.open,       // 4: D-Scan
        m_overviewState.open,    // 5: Overview
        m_chatState.open,        // 6: Chat
        m_dronePanelState.open,  // 7: Drones
    };
    sidebarBar(ctx, 0.0f, m_sidebarWidth,
              static_cast<float>(ctx.input().windowH),
              m_sidebarIcons, m_sidebarCallback,
              activeIcons, m_skillQueuePct);

    // 2. Locked target cards (top-center row)
    drawTargetCards(ctx, targets);

    // 3. Selected item panel (top-right)
    if (m_selectedItemState.open && !selectedItem.name.empty()) {
        drawSelectedItemPanel(ctx, selectedItem);
    }

    // 4. Overview panel (right side)
    if (m_overviewState.open) {
        drawOverviewPanel(ctx, overview);
    }

    // 5. Fleet broadcasts (above target cards)
    drawFleetBroadcasts(ctx);

    // 6. Ship HUD (bottom-center)
    drawShipHUD(ctx, ship);

    // 7. Mode indicator (above HUD)
    drawModeIndicator(ctx);

    // 8. Combat log (bottom-left)
    drawCombatLog(ctx);

    // 9. Drone status (above module rack)
    if (m_showDroneStatus) {
        drawDroneStatus(ctx);
    }

    // 10. Info panel (if open)
    drawInfoPanel(ctx);

    // 11. Dockable panels (opened via Neocom sidebar)
    drawDockablePanel(ctx, "Inventory", m_inventoryState);
    drawDockablePanel(ctx, "Ship Fitting", m_fittingState);
    drawDockablePanel(ctx, "Market", m_marketState);
    drawDockablePanel(ctx, "Missions", m_missionState);
    drawDockablePanel(ctx, "D-Scan", m_dscanState);
    drawDockablePanel(ctx, "Chat", m_chatState);
    drawDockablePanel(ctx, "Drones", m_dronePanelState);
    drawDockablePanel(ctx, "Probe Scanner", m_probeScannerState);
    drawDockablePanel(ctx, "Station Services", m_stationState);
    drawDockablePanel(ctx, "Fleet", m_fleetState);

    // 12. Damage flashes (on top of everything)
    float winW = static_cast<float>(ctx.input().windowW);
    float winH = static_cast<float>(ctx.input().windowH);
    Vec2 hudCentre = {winW * 0.5f, winH - 110.0f};
    drawDamageFlashes(ctx, hudCentre, 80.0f);
}

// ── Ship HUD ────────────────────────────────────────────────────────

void AtlasHUD::drawShipHUD(AtlasContext& ctx, const ShipHUDData& ship) {
    float winW = static_cast<float>(ctx.input().windowW);
    float winH = static_cast<float>(ctx.input().windowH);

    // Advance animation time (approximate dt from input if available, else 1/60)
    float dt = 1.0f / 60.0f;  // default frame time
    m_time += dt;

    // Centre of HUD circle (bottom-center of screen)
    Vec2 hudCentre = {winW * 0.5f, winH - 110.0f};
    float hudRadius = 70.0f;

    // Status arcs (shield/armor/hull)
    shipStatusArcs(ctx, hudCentre, hudRadius,
                   ship.shieldPct, ship.armorPct, ship.hullPct);

    // Capacitor ring with smooth easing
    float capInner = hudRadius - 30.0f;
    float capOuter = hudRadius - 22.0f;
    capacitorRingAnimated(ctx, hudCentre, capInner, capOuter,
                          ship.capacitorPct, m_displayCapFrac,
                          dt, ship.capSegments);

    // Module rack (row of circles below the HUD circle)
    float moduleY = winH - 30.0f;
    float moduleR = 14.0f;
    float moduleGap = 4.0f;

    auto drawModuleRow = [&](const std::vector<ShipHUDData::ModuleInfo>& slots,
                             float startX, int slotOffset) {
        for (int i = 0; i < static_cast<int>(slots.size()); ++i) {
            const auto& mod = slots[i];
            if (!mod.fitted) continue;
            float cx = startX + i * (moduleR * 2 + moduleGap);
            bool clicked = moduleSlotEx(ctx, {cx, moduleY}, moduleR,
                                        mod.active, mod.cooldown, mod.color,
                                        mod.overheat, m_time);
            if (clicked && m_moduleCallback) {
                m_moduleCallback(slotOffset + i);
            }
        }
    };

    // Layout: high slots left of centre, mid centre, low right
    int highCount = static_cast<int>(ship.highSlots.size());
    int midCount  = static_cast<int>(ship.midSlots.size());
    int totalModules = highCount + midCount + static_cast<int>(ship.lowSlots.size());
    float totalWidth = totalModules * (moduleR * 2 + moduleGap) - moduleGap;
    float startX = hudCentre.x - totalWidth * 0.5f + moduleR;

    drawModuleRow(ship.highSlots, startX, 0);
    drawModuleRow(ship.midSlots,  startX + highCount * (moduleR * 2 + moduleGap), highCount);
    drawModuleRow(ship.lowSlots,  startX + (highCount + midCount) * (moduleR * 2 + moduleGap),
                  highCount + midCount);

    // Speed indicator (below module rack, moved up for better visibility)
    int speedDir = speedIndicator(ctx, {hudCentre.x, winH - 42.0f},
                   ship.currentSpeed, ship.maxSpeed);
    if (speedDir != 0 && m_speedChangeCallback) {
        m_speedChangeCallback(speedDir);
    }

    // Warp progress indicator (above the HUD circle when warping)
    if (ship.warpActive && ship.warpPhase > 0) {
        float warpY = hudCentre.y - hudRadius - 50.0f;
        warpProgressIndicator(ctx, {hudCentre.x, warpY},
                              ship.warpPhase, ship.warpProgress,
                              ship.warpSpeedAU);
    }

    // Keyboard shortcuts: F1–F8 activate high-slot modules
    if (m_moduleCallback) {
        const auto& input = ctx.input();
        for (int k = 0; k < 8; ++k) {
            if (input.keyPressed[Key::F1 + k]) {
                m_moduleCallback(k);
            }
        }
    }
}

// ── Target Cards ────────────────────────────────────────────────────

void AtlasHUD::drawTargetCards(AtlasContext& ctx,
                                const std::vector<TargetCardInfo>& targets) {
    if (targets.empty()) return;

    float winW = static_cast<float>(ctx.input().windowW);
    float cardW = 80.0f;
    float cardH = 80.0f;
    float gap = 4.0f;

    float totalW = targets.size() * (cardW + gap) - gap;
    float startX = (winW - totalW) * 0.5f;
    float startY = 8.0f;

    for (int i = 0; i < static_cast<int>(targets.size()); ++i) {
        Rect cardRect = {startX + i * (cardW + gap), startY, cardW, cardH};
        targetCard(ctx, cardRect, targets[i]);
    }
}

// ── Overview Panel ──────────────────────────────────────────────────

void AtlasHUD::drawOverviewPanel(AtlasContext& ctx,
                                  const std::vector<OverviewEntry>& entries) {
    PanelFlags flags;
    flags.showHeader = true;
    flags.showClose = true;
    flags.showMinimize = true;
    flags.drawBorder = true;

    if (!panelBeginStateful(ctx, "Overview", m_overviewState, flags)) {
        panelEnd(ctx);
        return;
    }

    const Theme& t = ctx.theme();
    float hh = t.headerHeight;
    Rect contentArea = {m_overviewState.bounds.x,
                        m_overviewState.bounds.y + hh,
                        m_overviewState.bounds.w,
                        m_overviewState.bounds.h - hh};

    // Tab header (interactive — multiple customizable tabs)
    Rect tabRect = {contentArea.x, contentArea.y, contentArea.w, 24.0f};
    int clickedTab = overviewHeaderInteractive(ctx, tabRect, m_overviewTabs, m_overviewActiveTab);
    if (clickedTab >= 0) {
        m_overviewActiveTab = clickedTab;
    }

    // ── Column header row (sortable) ───────────────────────────────
    float colY = contentArea.y + 28.0f;
    float colH = 18.0f;
    float colX = contentArea.x;
    float totalW = contentArea.w;

    // Column layout: Distance(25%), Name(35%), Type(20%), Velocity(20%)
    struct ColDef {
        const char* label;
        float widthPct;
        OverviewSortColumn col;
    };
    ColDef cols[] = {
        {"Dist",  0.25f, OverviewSortColumn::DISTANCE},
        {"Name",  0.35f, OverviewSortColumn::NAME},
        {"Type",  0.20f, OverviewSortColumn::TYPE},
        {"Vel",   0.20f, OverviewSortColumn::VELOCITY},
    };

    for (int c = 0; c < 4; ++c) {
        float cw = totalW * cols[c].widthPct;
        Rect colRect = {colX, colY, cw, colH};
        WidgetID colID = hashID("ovcol") ^ static_cast<uint32_t>(c);

        bool hovered = ctx.isHovered(colRect);
        if (hovered) {
            ctx.renderer().drawRect(colRect, t.hover);
            ctx.setHot(colID);
        }

        // Draw column label with sort indicator
        std::string colText = cols[c].label;
        if (m_overviewSortCol == cols[c].col) {
            colText += m_overviewSortAsc ? " ^" : " v";
        }
        ctx.renderer().drawText(colText,
            {colX + 4.0f, colY + 2.0f}, t.textSecondary);

        // Click to sort
        if (ctx.buttonBehavior(colRect, colID)) {
            if (m_overviewSortCol == cols[c].col) {
                m_overviewSortAsc = !m_overviewSortAsc;
            } else {
                m_overviewSortCol = cols[c].col;
                m_overviewSortAsc = true;
            }
        }
        colX += cw;
    }

    // Filter + sort entries by index (avoids copying the entire entries vector)
    // Apply per-tab entity-type filtering based on active tab label
    std::string activeTabName;
    if (m_overviewActiveTab >= 0 && m_overviewActiveTab < static_cast<int>(m_overviewTabs.size()))
        activeTabName = m_overviewTabs[m_overviewActiveTab];

    std::vector<int> sortedIdx;
    sortedIdx.reserve(entries.size());
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        if (matchesOverviewTab(activeTabName, entries[i].type))
            sortedIdx.push_back(i);
    }
    auto sortCol = m_overviewSortCol;
    bool sortAsc = m_overviewSortAsc;
    std::sort(sortedIdx.begin(), sortedIdx.end(),
        [&entries, sortCol, sortAsc](int ai, int bi) {
            const OverviewEntry& a = entries[ai];
            const OverviewEntry& b = entries[bi];
            int cmp = 0;
            switch (sortCol) {
                case OverviewSortColumn::DISTANCE:
                    cmp = (a.distance < b.distance) ? -1 : (a.distance > b.distance) ? 1 : 0;
                    break;
                case OverviewSortColumn::NAME:
                    cmp = a.name.compare(b.name);
                    break;
                case OverviewSortColumn::TYPE:
                    cmp = a.type.compare(b.type);
                    break;
                case OverviewSortColumn::VELOCITY:
                    cmp = (a.velocity < b.velocity) ? -1 : (a.velocity > b.velocity) ? 1 : 0;
                    break;
            }
            return sortAsc ? (cmp < 0) : (cmp > 0);
        });

    // Rows
    float rowH = 22.0f;
    float rowY = colY + colH + 2.0f;
    int maxRows = static_cast<int>((contentArea.bottom() - rowY) / rowH);
    int count = std::min(static_cast<int>(sortedIdx.size()), maxRows);

    for (int i = 0; i < count; ++i) {
        const OverviewEntry& entry = entries[sortedIdx[i]];
        Rect rowRect = {contentArea.x, rowY + i * rowH, contentArea.w, rowH};
        // overviewRow returns true when the row receives a left-click (press+release)
        bool clicked = overviewRow(ctx, rowRect, entry, (i % 2 == 1));

        // Left-click: select the entity
        if (clicked && !entry.entityId.empty()) {
            // Ctrl+Click = lock target (EVE standard)
            if (ctx.input().keyDown[Key::LeftControl] && m_overviewCtrlClickCb) {
                m_overviewCtrlClickCb(entry.entityId);
            } else if (m_overviewSelectCb) {
                m_overviewSelectCb(entry.entityId);
            }
        }

        // Right-click: show context menu for the entity
        if (!ctx.isMouseConsumed() && ctx.isHovered(rowRect) && ctx.isRightMouseClicked()) {
            if (!entry.entityId.empty() && m_overviewRightClickCb) {
                m_overviewRightClickCb(entry.entityId,
                                       ctx.input().mousePos.x,
                                       ctx.input().mousePos.y);
                ctx.consumeMouse();
            }
        }
    }

    // Scrollbar if needed
    if (static_cast<int>(sortedIdx.size()) > maxRows) {
        Rect scrollTrack = {contentArea.right() - 6.0f,
                           rowY,
                           6.0f,
                           contentArea.bottom() - rowY};
        scrollbar(ctx, scrollTrack, 0.0f,
                 sortedIdx.size() * rowH, contentArea.bottom() - rowY);
    }

    // Right-click on overview panel background (not on an entity row)
    // shows an empty-space context menu
    if (!ctx.isMouseConsumed() && ctx.isHovered(contentArea) && ctx.isRightMouseClicked()) {
        if (m_overviewBgRightClickCb) {
            m_overviewBgRightClickCb(ctx.input().mousePos.x,
                                     ctx.input().mousePos.y);
            ctx.consumeMouse();
        }
    }

    panelEnd(ctx);
}

// ── Selected Item Panel ─────────────────────────────────────────────

void AtlasHUD::drawSelectedItemPanel(AtlasContext& ctx,
                                      const SelectedItemInfo& info) {
    PanelFlags flags;
    flags.showHeader = true;
    flags.showClose = true;
    flags.showMinimize = true;
    flags.drawBorder = true;

    if (!panelBeginStateful(ctx, "Selected Item", m_selectedItemState, flags)) {
        panelEnd(ctx);
        return;
    }

    const Theme& t = ctx.theme();
    float hh = t.headerHeight;

    // Name
    float textY = m_selectedItemState.bounds.y + hh + 8.0f;
    ctx.renderer().drawText(info.name,
        {m_selectedItemState.bounds.x + t.padding, textY}, t.textPrimary);

    // Distance
    textY += 18.0f;
    char distBuf[64];
    std::snprintf(distBuf, sizeof(distBuf), "Distance: %.0f %s",
                  info.distance, info.distanceUnit.c_str());
    ctx.renderer().drawText(distBuf,
        {m_selectedItemState.bounds.x + t.padding, textY}, t.textSecondary);

    // Action buttons
    float btnY = textY + 24.0f;
    float btnSz = 24.0f;
    float btnGap = 6.0f;
    float btnX = m_selectedItemState.bounds.x + t.padding;

    // O = Orbit, >> = Approach, W = Warp, i = Info
    struct ActionBtn { const char* label; const std::function<void()>* cb; };
    ActionBtn actions[] = {
        {"O",  &m_selOrbitCb},
        {">>", &m_selApproachCb},
        {"W",  &m_selWarpCb},
        {"i",  &m_selInfoCb},
    };
    for (int i = 0; i < 4; ++i) {
        Rect btn = {btnX, btnY, btnSz, btnSz};
        if (button(ctx, actions[i].label, btn)) {
            if (actions[i].cb && *(actions[i].cb)) {
                (*(actions[i].cb))();
            }
        }
        btnX += btnSz + btnGap;
    }

    panelEnd(ctx);
}

// ── Mode Indicator ──────────────────────────────────────────────────

void AtlasHUD::drawModeIndicator(AtlasContext& ctx) {
    if (m_modeText.empty()) return;

    float winW = static_cast<float>(ctx.input().windowW);
    float winH = static_cast<float>(ctx.input().windowH);

    // Position above the ship HUD circle
    Vec2 pos = {winW * 0.5f, winH - 180.0f};
    modeIndicator(ctx, pos, m_modeText.c_str());
}

// ── Info Panel ──────────────────────────────────────────────────────

void AtlasHUD::showInfoPanel(const InfoPanelData& data) {
    m_infoPanelData = data;
    m_infoPanelState.open = true;
}

void AtlasHUD::drawInfoPanel(AtlasContext& ctx) {
    if (!m_infoPanelState.open || m_infoPanelData.isEmpty()) return;

    infoPanelDraw(ctx, m_infoPanelState, m_infoPanelData);
}


// ── Combat Log ──────────────────────────────────────────────────────

void AtlasHUD::addCombatLogMessage(const std::string& msg) {
    m_combatLog.push_back(msg);
    if (static_cast<int>(m_combatLog.size()) > MAX_COMBAT_LOG) {
        m_combatLog.erase(m_combatLog.begin());
    }
    // Auto-scroll to bottom
    float rowH = 16.0f;
    int visRows = 8;
    float maxScroll = std::max(0.0f,
        (static_cast<int>(m_combatLog.size()) - visRows) * rowH);
    m_combatLogScroll = maxScroll;
}

void AtlasHUD::drawCombatLog(AtlasContext& ctx) {
    if (m_combatLog.empty()) return;

    float winH = static_cast<float>(ctx.input().windowH);
    float logW = 280.0f;
    float logH = 160.0f;
    Rect logRect = {m_sidebarWidth + 8.0f, winH - logH - 8.0f, logW, logH};

    combatLogWidget(ctx, logRect, m_combatLog, m_combatLogScroll);
}

// ── Damage Flashes ──────────────────────────────────────────────────

void AtlasHUD::triggerDamageFlash(int layer, float duration) {
    // Limit concurrent flashes
    if (static_cast<int>(m_damageFlashes.size()) >= 3) {
        m_damageFlashes.erase(m_damageFlashes.begin());
    }
    DamageFlashState f;
    f.layer = layer;
    f.intensity = 1.0f;
    f.elapsed = 0.0f;
    f.duration = duration;
    m_damageFlashes.push_back(f);
}

bool AtlasHUD::hasDamageFlash() const {
    return !m_damageFlashes.empty();
}

void AtlasHUD::drawDamageFlashes(AtlasContext& ctx, Vec2 hudCentre,
                                  float hudRadius) {
    float dt = 1.0f / 60.0f;

    for (auto it = m_damageFlashes.begin(); it != m_damageFlashes.end(); ) {
        it->elapsed += dt;
        it->intensity = 1.0f - (it->elapsed / std::max(it->duration, 0.01f));
        if (it->intensity <= 0.0f) {
            it = m_damageFlashes.erase(it);
        } else {
            damageFlashOverlay(ctx, hudCentre, hudRadius,
                               it->layer, it->intensity);
            ++it;
        }
    }
}

// ── Drone Status ────────────────────────────────────────────────────

void AtlasHUD::drawDroneStatus(AtlasContext& ctx) {
    float winW = static_cast<float>(ctx.input().windowW);
    float winH = static_cast<float>(ctx.input().windowH);
    float barW = 260.0f;
    float barH = 22.0f;
    Rect barRect = {(winW - barW) * 0.5f, winH - 70.0f, barW, barH};

    droneStatusBar(ctx, barRect,
                   m_droneStatus.inSpace, m_droneStatus.inBay,
                   m_droneStatus.bandwidthUsed, m_droneStatus.bandwidthMax);
}

// ── Fleet Broadcasts ────────────────────────────────────────────────

void AtlasHUD::addFleetBroadcast(const std::string& sender,
                                  const std::string& message,
                                  const Color& color) {
    // Detect default (white) color and substitute a themed accent
    auto isDefaultColor = [](const Color& c) {
        return c.r == 1.0f && c.g == 1.0f && c.b == 1.0f;
    };

    FleetBroadcast bc;
    bc.sender = sender;
    bc.message = message;
    bc.color = isDefaultColor(color) ? atlas::defaultTheme().accentCombat : color;
    bc.age = 0.0f;
    bc.maxAge = 8.0f;
    m_broadcasts.push_back(bc);
    if (static_cast<int>(m_broadcasts.size()) > MAX_BROADCASTS) {
        m_broadcasts.erase(m_broadcasts.begin());
    }
}

void AtlasHUD::drawFleetBroadcasts(AtlasContext& ctx) {
    if (m_broadcasts.empty()) return;

    float dt = 1.0f / 60.0f;

    // Age broadcasts and remove expired
    for (auto it = m_broadcasts.begin(); it != m_broadcasts.end(); ) {
        it->age += dt;
        if (it->age >= it->maxAge) {
            it = m_broadcasts.erase(it);
        } else {
            ++it;
        }
    }

    if (m_broadcasts.empty()) return;

    float winW = static_cast<float>(ctx.input().windowW);
    float bannerW = 300.0f;
    float bannerH = static_cast<float>(m_broadcasts.size()) * 20.0f;
    Rect bannerRect = {(winW - bannerW) * 0.5f, 92.0f, bannerW, bannerH};

    fleetBroadcastBanner(ctx, bannerRect, m_broadcasts);
}

// ── Dockable Panel (Atlas panel with panel-type-aware content) ───────

void AtlasHUD::drawDockablePanel(AtlasContext& ctx, const char* title,
                                  PanelState& state) {
    if (!state.open) return;

    PanelFlags flags;
    flags.showHeader = true;
    flags.showClose = true;
    flags.showMinimize = true;
    flags.drawBorder = true;

    if (!panelBeginStateful(ctx, title, state, flags)) {
        panelEnd(ctx);
        return;
    }

    const Theme& t = ctx.theme();
    float hh = t.headerHeight;
    float x = state.bounds.x + t.padding;
    float y = state.bounds.y + hh + t.padding;
    float contentW = state.bounds.w - t.padding * 2.0f;
    float maxY = state.bounds.bottom() - t.padding;
    auto& r = ctx.renderer();

    std::string titleStr(title);

    if (titleStr == "Inventory") {
        // Tab header
        const char* invTabs[] = {"Cargo Hold", "Station Hangar"};
        for (int ti = 0; ti < 2; ++ti) {
            float tabW = contentW * 0.5f;
            Rect tabRect(x + ti * tabW, y, tabW, 20.0f);
            bool isActive = (m_inventoryData.activeTab == ti);
            r.drawRect(tabRect, isActive ? t.bgHeader : t.bgPanel);
            r.drawText(invTabs[ti], Vec2(x + ti * tabW + 4.0f, y + 3.0f),
                       isActive ? t.accentPrimary : t.textSecondary, 1.0f);
            WidgetID tabID = hashID("invtab") ^ static_cast<uint32_t>(ti);
            if (ctx.buttonBehavior(tabRect, tabID)) {
                m_inventoryData.activeTab = ti;
            }
        }
        y += 24.0f;

        // Cargo capacity bar
        float capFrac = m_inventoryData.maxCapacity > 0.0f
            ? m_inventoryData.usedCapacity / m_inventoryData.maxCapacity : 0.0f;
        Rect capBar(x, y, contentW, 14.0f);
        Color capColor = capFrac > 0.9f ? t.danger : (capFrac > 0.7f ? t.warning : t.accentPrimary);
        r.drawProgressBar(capBar, capFrac, capColor, t.bgHeader);
        char capBuf[64];
        std::snprintf(capBuf, sizeof(capBuf), "%.0f / %.0f m3",
                      m_inventoryData.usedCapacity, m_inventoryData.maxCapacity);
        r.drawText(capBuf, Vec2(x + 4, y + 1), t.textPrimary, 1.0f);
        y += 22.0f;

        separator(ctx, Vec2(x, y), contentW);
        y += 8.0f;

        // Column headers
        r.drawText("Item", Vec2(x, y), t.textSecondary, 1.0f);
        r.drawText("Qty", Vec2(x + contentW * 0.6f, y), t.textSecondary, 1.0f);
        r.drawText("Vol", Vec2(x + contentW * 0.8f, y), t.textSecondary, 1.0f);
        y += 16.0f;
        separator(ctx, Vec2(x, y), contentW);
        y += 4.0f;

        if (m_inventoryData.items.empty()) {
            label(ctx, Vec2(x, y), "Cargo hold is empty", t.textSecondary);
        } else {
            for (size_t i = 0; i < m_inventoryData.items.size() && y < maxY - 16.0f; ++i) {
                const auto& item = m_inventoryData.items[i];
                if (i % 2 == 1) {
                    r.drawRect(Rect(x, y, contentW, 16.0f), t.bgHeader.withAlpha(0.3f));
                }
                r.drawText(item.name, Vec2(x + 2, y + 1), t.textPrimary, 1.0f);
                char qtyBuf[16];
                std::snprintf(qtyBuf, sizeof(qtyBuf), "%d", item.quantity);
                r.drawText(qtyBuf, Vec2(x + contentW * 0.6f, y + 1), t.textSecondary, 1.0f);
                char volBuf[16];
                std::snprintf(volBuf, sizeof(volBuf), "%.1f", item.volume * item.quantity);
                r.drawText(volBuf, Vec2(x + contentW * 0.8f, y + 1), t.textSecondary, 1.0f);
                y += 16.0f;
            }
        }

    } else if (titleStr == "Ship Fitting") {
        // Ship name
        r.drawText(m_fittingData.shipName, Vec2(x, y), t.accentPrimary, 1.0f);
        y += 18.0f;
        separator(ctx, Vec2(x, y), contentW);
        y += 8.0f;

        // CPU bar
        float cpuFrac = m_fittingData.cpuMax > 0.0f ? m_fittingData.cpuUsed / m_fittingData.cpuMax : 0.0f;
        r.drawText("CPU", Vec2(x, y), t.textSecondary, 1.0f);
        Rect cpuBar(x + 40.0f, y, contentW - 40.0f, 12.0f);
        Color cpuColor = cpuFrac > 0.95f ? t.danger : t.accentPrimary;
        r.drawProgressBar(cpuBar, cpuFrac, cpuColor, t.bgHeader);
        char cpuBuf[32];
        std::snprintf(cpuBuf, sizeof(cpuBuf), "%.0f / %.0f tf", m_fittingData.cpuUsed, m_fittingData.cpuMax);
        r.drawText(cpuBuf, Vec2(x + 44.0f, y), t.textPrimary, 1.0f);
        y += 20.0f;

        // PG bar
        float pgFrac = m_fittingData.pgMax > 0.0f ? m_fittingData.pgUsed / m_fittingData.pgMax : 0.0f;
        r.drawText("PG", Vec2(x, y), t.textSecondary, 1.0f);
        Rect pgBar(x + 40.0f, y, contentW - 40.0f, 12.0f);
        Color pgColor = pgFrac > 0.95f ? t.danger : t.success;
        r.drawProgressBar(pgBar, pgFrac, pgColor, t.bgHeader);
        char pgBuf[32];
        std::snprintf(pgBuf, sizeof(pgBuf), "%.0f / %.0f MW", m_fittingData.pgUsed, m_fittingData.pgMax);
        r.drawText(pgBuf, Vec2(x + 44.0f, y), t.textPrimary, 1.0f);
        y += 20.0f;

        // Calibration bar
        float calFrac = m_fittingData.calibrationMax > 0.0f
            ? m_fittingData.calibrationUsed / m_fittingData.calibrationMax : 0.0f;
        r.drawText("Cal", Vec2(x, y), t.textSecondary, 1.0f);
        Rect calBar(x + 40.0f, y, contentW - 40.0f, 12.0f);
        Color calColor = {0.6f, 0.3f, 0.9f, 1.0f};  // purple
        r.drawProgressBar(calBar, calFrac, calColor, t.bgHeader);
        char calBuf[32];
        std::snprintf(calBuf, sizeof(calBuf), "%.0f / %.0f", m_fittingData.calibrationUsed, m_fittingData.calibrationMax);
        r.drawText(calBuf, Vec2(x + 44.0f, y), t.textPrimary, 1.0f);
        y += 24.0f;

        separator(ctx, Vec2(x, y), contentW);
        y += 8.0f;

        // Slot sections
        struct SlotSection {
            const char* label;
            const std::vector<FittingSlot>* slots;
        };
        SlotSection sections[] = {
            {"High Slots", &m_fittingData.highSlots},
            {"Mid Slots", &m_fittingData.midSlots},
            {"Low Slots", &m_fittingData.lowSlots},
        };
        for (int s = 0; s < 3 && y < maxY - 30.0f; ++s) {
            r.drawText(sections[s].label, Vec2(x, y), t.textSecondary, 1.0f);
            y += 16.0f;
            if (sections[s].slots->empty()) {
                r.drawText("[no slots]", Vec2(x + 10.0f, y), t.bgHeader, 1.0f);
                y += 14.0f;
            } else {
                for (size_t i = 0; i < sections[s].slots->size() && y < maxY - 14.0f; ++i) {
                    const auto& slot = (*sections[s].slots)[i];
                    if (slot.fitted) {
                        Color slotColor = slot.online ? t.textPrimary : t.textMuted;
                        r.drawText(slot.moduleName, Vec2(x + 10.0f, y), slotColor, 1.0f);
                    } else {
                        r.drawText("[empty]", Vec2(x + 10.0f, y), t.bgHeader, 1.0f);
                    }
                    y += 14.0f;
                }
            }
            y += 4.0f;
        }

        // Stats section
        if (y < maxY - 60.0f) {
            separator(ctx, Vec2(x, y), contentW);
            y += 8.0f;
            r.drawText("Stats", Vec2(x, y), t.textPrimary, 1.0f);
            y += 16.0f;

            char ehpBuf[32];
            std::snprintf(ehpBuf, sizeof(ehpBuf), "EHP: %.0f", m_fittingData.effectiveHP);
            r.drawText(ehpBuf, Vec2(x + 8.0f, y), t.textSecondary, 1.0f);
            y += 14.0f;

            char dpsBuf[32];
            std::snprintf(dpsBuf, sizeof(dpsBuf), "DPS: %.1f", m_fittingData.dps);
            r.drawText(dpsBuf, Vec2(x + 8.0f, y), t.textSecondary, 1.0f);
            y += 14.0f;

            char velBuf[32];
            std::snprintf(velBuf, sizeof(velBuf), "Max Vel: %.0f m/s", m_fittingData.maxVelocity);
            r.drawText(velBuf, Vec2(x + 8.0f, y), t.textSecondary, 1.0f);
            y += 14.0f;

            if (m_fittingData.capStable) {
                r.drawText("Cap: Stable", Vec2(x + 8.0f, y), t.success, 1.0f);
            } else {
                char capBuf2[48];
                std::snprintf(capBuf2, sizeof(capBuf2), "Cap: %.0fs", m_fittingData.capTime);
                r.drawText(capBuf2, Vec2(x + 8.0f, y), t.warning, 1.0f);
            }
        }

    } else if (titleStr == "Market") {
        // Tab bar
        const char* mktTabs[] = {"Browse", "My Orders", "History"};
        for (int ti = 0; ti < 3; ++ti) {
            float tabW = contentW / 3.0f;
            Rect tabRect(x + ti * tabW, y, tabW, 20.0f);
            bool isActive = (m_marketData.activeTab == ti);
            r.drawRect(tabRect, isActive ? t.bgHeader : t.bgPanel);
            r.drawText(mktTabs[ti], Vec2(x + ti * tabW + 4.0f, y + 3.0f),
                       isActive ? t.accentPrimary : t.textSecondary, 1.0f);
            WidgetID tabID = hashID("mkttab") ^ static_cast<uint32_t>(ti);
            if (ctx.buttonBehavior(tabRect, tabID)) {
                m_marketData.activeTab = ti;
            }
        }
        y += 24.0f;

        separator(ctx, Vec2(x, y), contentW);
        y += 8.0f;

        // Sell orders section
        r.drawText("Sell Orders", Vec2(x, y), t.danger, 1.0f);
        y += 18.0f;
        r.drawText("Item", Vec2(x, y), t.textSecondary, 1.0f);
        r.drawText("Price", Vec2(x + contentW * 0.5f, y), t.textSecondary, 1.0f);
        r.drawText("Qty", Vec2(x + contentW * 0.8f, y), t.textSecondary, 1.0f);
        y += 16.0f;
        separator(ctx, Vec2(x, y), contentW);
        y += 4.0f;

        if (m_marketData.sellOrders.empty()) {
            label(ctx, Vec2(x + 8, y), "No sell orders", t.textSecondary);
            y += 18.0f;
        } else {
            for (size_t i = 0; i < m_marketData.sellOrders.size() && y < maxY * 0.5f; ++i) {
                const auto& order = m_marketData.sellOrders[i];
                if (i % 2 == 1) {
                    r.drawRect(Rect(x, y, contentW, 16.0f), t.bgHeader.withAlpha(0.3f));
                }
                r.drawText(order.itemName, Vec2(x + 2, y + 1), t.textPrimary, 1.0f);
                char priceBuf[32];
                std::snprintf(priceBuf, sizeof(priceBuf), "%.0f", order.price);
                r.drawText(priceBuf, Vec2(x + contentW * 0.5f, y + 1), t.textSecondary, 1.0f);
                char qtyBuf[16];
                std::snprintf(qtyBuf, sizeof(qtyBuf), "%d", order.quantity);
                r.drawText(qtyBuf, Vec2(x + contentW * 0.8f, y + 1), t.textSecondary, 1.0f);
                y += 16.0f;
            }
        }

        y += 4.0f;
        separator(ctx, Vec2(x, y), contentW);
        y += 8.0f;

        // Buy orders section
        r.drawText("Buy Orders", Vec2(x, y), t.success, 1.0f);
        y += 18.0f;
        r.drawText("Item", Vec2(x, y), t.textSecondary, 1.0f);
        r.drawText("Price", Vec2(x + contentW * 0.5f, y), t.textSecondary, 1.0f);
        r.drawText("Qty", Vec2(x + contentW * 0.8f, y), t.textSecondary, 1.0f);
        y += 16.0f;
        separator(ctx, Vec2(x, y), contentW);
        y += 4.0f;

        if (m_marketData.buyOrders.empty()) {
            label(ctx, Vec2(x + 8, y), "No buy orders", t.textSecondary);
        } else {
            for (size_t i = 0; i < m_marketData.buyOrders.size() && y < maxY - 16.0f; ++i) {
                const auto& order = m_marketData.buyOrders[i];
                if (i % 2 == 1) {
                    r.drawRect(Rect(x, y, contentW, 16.0f), t.bgHeader.withAlpha(0.3f));
                }
                r.drawText(order.itemName, Vec2(x + 2, y + 1), t.textPrimary, 1.0f);
                char priceBuf[32];
                std::snprintf(priceBuf, sizeof(priceBuf), "%.0f", order.price);
                r.drawText(priceBuf, Vec2(x + contentW * 0.5f, y + 1), t.textSecondary, 1.0f);
                char qtyBuf[16];
                std::snprintf(qtyBuf, sizeof(qtyBuf), "%d", order.quantity);
                r.drawText(qtyBuf, Vec2(x + contentW * 0.8f, y + 1), t.textSecondary, 1.0f);
                y += 16.0f;
            }
        }

    } else if (titleStr == "Missions") {
        if (!m_missionInfo.active) {
            label(ctx, Vec2(x, y), "No Active Mission", t.textSecondary);
            y += 20.0f;
            label(ctx, Vec2(x, y), "Visit an agent to accept a mission", t.textSecondary);
        } else {
            // Mission name
            r.drawText(m_missionInfo.name, Vec2(x, y), t.accentPrimary, 1.0f);
            y += 18.0f;

            // Mission type and level
            char infoBuf[128];
            std::snprintf(infoBuf, sizeof(infoBuf), "Level %d %s",
                          m_missionInfo.level, m_missionInfo.type.c_str());
            r.drawText(infoBuf, Vec2(x, y), t.textSecondary, 1.0f);
            y += 16.0f;

            if (!m_missionInfo.agentName.empty()) {
                char agentBuf[128];
                std::snprintf(agentBuf, sizeof(agentBuf), "Agent: %s",
                              m_missionInfo.agentName.c_str());
                r.drawText(agentBuf, Vec2(x, y), t.textSecondary, 1.0f);
                y += 16.0f;
            }

            // Time limit
            if (m_missionInfo.timeLimitHours > 0.0f && y < maxY - 60.0f) {
                char timeBuf[64];
                float remaining = m_missionInfo.timeLimitHours - m_missionInfo.timeElapsedHours;
                if (remaining < 0.0f) remaining = 0.0f;
                std::snprintf(timeBuf, sizeof(timeBuf), "Time: %.1f / %.1f hrs",
                              m_missionInfo.timeElapsedHours, m_missionInfo.timeLimitHours);
                r.drawText(timeBuf, Vec2(x, y),
                           remaining < 1.0f ? t.danger : t.textSecondary, 1.0f);
                y += 16.0f;
                float timePct = m_missionInfo.timeElapsedHours / m_missionInfo.timeLimitHours;
                Rect timeBar(x, y, contentW, 10.0f);
                Color barColor = timePct > 0.8f ? t.danger : t.accentPrimary;
                r.drawProgressBar(timeBar, timePct, barColor, t.bgHeader);
                y += 16.0f;
            }

            y += 4.0f;
            separator(ctx, Vec2(x, y), contentW);
            y += 8.0f;

            // Objectives
            r.drawText("Objectives:", Vec2(x, y), t.textPrimary, 1.0f);
            y += 16.0f;
            for (const auto& obj : m_missionInfo.objectives) {
                if (y > maxY - 40.0f) break;
                Color objColor = obj.completed ? t.success : t.textSecondary;
                const char* marker = obj.completed ? "[x] " : "[ ] ";
                char objBuf[256];
                std::snprintf(objBuf, sizeof(objBuf), "%s%s",
                              marker, obj.description.c_str());
                r.drawText(objBuf, Vec2(x + 8.0f, y), objColor, 1.0f);
                y += 14.0f;
            }

            y += 8.0f;
            separator(ctx, Vec2(x, y), contentW);
            y += 8.0f;

            // Rewards
            if (y < maxY - 40.0f) {
                r.drawText("Rewards:", Vec2(x, y), t.textPrimary, 1.0f);
                y += 16.0f;
                if (m_missionInfo.iskReward > 0) {
                    char iskBuf[64];
                    std::snprintf(iskBuf, sizeof(iskBuf), "ISK: %.0f",
                                  m_missionInfo.iskReward);
                    r.drawText(iskBuf, Vec2(x + 8.0f, y), t.warning, 1.0f);
                    y += 14.0f;
                }
                if (m_missionInfo.lpReward > 0) {
                    char lpBuf[64];
                    std::snprintf(lpBuf, sizeof(lpBuf), "LP: %.0f",
                                  m_missionInfo.lpReward);
                    r.drawText(lpBuf, Vec2(x + 8.0f, y), t.accentSecondary, 1.0f);
                    y += 14.0f;
                }
            }
        }

    } else if (titleStr == "D-Scan") {
        // Scan controls
        char angleBuf[32];
        std::snprintf(angleBuf, sizeof(angleBuf), "Angle: %.0f deg", m_dscanAngle);
        r.drawText(angleBuf, Vec2(x, y), t.textSecondary, 1.0f);
        y += 16.0f;
        char rangeBuf[32];
        std::snprintf(rangeBuf, sizeof(rangeBuf), "Range: %.1f AU", m_dscanRange);
        r.drawText(rangeBuf, Vec2(x, y), t.textSecondary, 1.0f);
        y += 20.0f;
        // Scan button (also triggered by V key)
        Rect scanBtn(x, y, 80.0f, 22.0f);
        bool scanPressed = button(ctx, "SCAN", scanBtn);
        if (ctx.input().keyPressed[Key::V]) scanPressed = true;
        if (scanPressed && m_dscanCallback) {
            m_dscanCallback();
        }
        y += 30.0f;
        separator(ctx, Vec2(x, y), contentW);
        y += 8.0f;

        // Results header
        char countBuf[32];
        std::snprintf(countBuf, sizeof(countBuf), "Results: %d",
                      static_cast<int>(m_dscanResults.size()));
        r.drawText(countBuf, Vec2(x, y), t.textPrimary, 1.0f);
        y += 18.0f;

        if (m_dscanResults.empty()) {
            label(ctx, Vec2(x, y), "No scan results", t.textSecondary);
        } else {
            // Column headers
            r.drawText("Name", Vec2(x, y), t.textSecondary, 1.0f);
            r.drawText("Type", Vec2(x + contentW * 0.5f, y), t.textSecondary, 1.0f);
            r.drawText("Dist", Vec2(x + contentW * 0.8f, y), t.textSecondary, 1.0f);
            y += 16.0f;
            separator(ctx, Vec2(x, y), contentW);
            y += 4.0f;

            for (size_t i = 0; i < m_dscanResults.size() && y < maxY - 16.0f; ++i) {
                if (i % 2 == 1) {
                    r.drawRect(Rect(x, y, contentW, 16.0f),
                               t.bgHeader.withAlpha(0.3f));
                }
                r.drawText(m_dscanResults[i].name,
                           Vec2(x + 2, y + 1), t.textPrimary, 1.0f);
                r.drawText(m_dscanResults[i].type,
                           Vec2(x + contentW * 0.5f, y + 1), t.textSecondary, 1.0f);
                char distBuf[32];
                std::snprintf(distBuf, sizeof(distBuf), "%.1f AU",
                              m_dscanResults[i].distance);
                r.drawText(distBuf,
                           Vec2(x + contentW * 0.8f, y + 1), t.textSecondary, 1.0f);
                y += 16.0f;
            }
        }

    } else if (titleStr == "Chat") {
        // Channel tab
        Rect tabRect(x, y, 60.0f, 18.0f);
        r.drawRect(tabRect, t.bgHeader);
        r.drawText("Local", Vec2(x + 4, y + 2), t.accentPrimary, 1.0f);
        y += 22.0f;
        separator(ctx, Vec2(x, y), contentW);
        y += 4.0f;
        label(ctx, Vec2(x, y), "No messages", t.textSecondary);

    } else if (titleStr == "Drones") {
        // Bandwidth
        r.drawText("Bandwidth", Vec2(x, y), t.textSecondary, 1.0f);
        y += 16.0f;
        Rect bwBar(x, y, contentW, 12.0f);
        r.drawProgressBar(bwBar, 0.0f, t.accentPrimary, t.bgHeader);
        r.drawText("0 / 0 Mbit/s", Vec2(x + 4, y), t.textPrimary, 1.0f);
        y += 20.0f;

        // Bay capacity
        r.drawText("Drone Bay", Vec2(x, y), t.textSecondary, 1.0f);
        y += 16.0f;
        Rect bayBar(x, y, contentW, 12.0f);
        r.drawProgressBar(bayBar, 0.0f, t.accentSecondary, t.bgHeader);
        r.drawText("0 / 0 m3", Vec2(x + 4, y), t.textPrimary, 1.0f);
        y += 24.0f;

        separator(ctx, Vec2(x, y), contentW);
        y += 8.0f;

        r.drawText("In Space (0)", Vec2(x, y), t.textPrimary, 1.0f);
        y += 18.0f;
        r.drawText("In Bay (0)", Vec2(x, y), t.textPrimary, 1.0f);

    } else if (titleStr == "Probe Scanner") {
        // Probe deployment info
        char probeBuf[64];
        std::snprintf(probeBuf, sizeof(probeBuf), "Probes: %d deployed", m_probeCount);
        r.drawText(probeBuf, Vec2(x, y), t.textPrimary, 1.0f);
        y += 16.0f;
        char rangeBuf[64];
        std::snprintf(rangeBuf, sizeof(rangeBuf), "Scan Range: %.1f AU", m_probeRange);
        r.drawText(rangeBuf, Vec2(x, y), t.textSecondary, 1.0f);
        y += 20.0f;

        // Analyze button
        Rect analyzeBtn(x, y, 100.0f, 22.0f);
        if (button(ctx, "Analyze", analyzeBtn)) {
            if (m_probeScanCallback) m_probeScanCallback();
        }
        y += 30.0f;

        separator(ctx, Vec2(x, y), contentW);
        y += 8.0f;

        // Filter checkboxes row
        r.drawText("Filters:", Vec2(x, y), t.textSecondary, 1.0f);
        y += 16.0f;
        r.drawText("Anomalies | Signatures | Ships", Vec2(x + 4.0f, y), t.textMuted, 1.0f);
        y += 18.0f;

        separator(ctx, Vec2(x, y), contentW);
        y += 8.0f;

        // Results
        char countBuf[32];
        std::snprintf(countBuf, sizeof(countBuf), "Results: %d",
                      static_cast<int>(m_probeScanResults.size()));
        r.drawText(countBuf, Vec2(x, y), t.textPrimary, 1.0f);
        y += 18.0f;

        if (m_probeScanResults.empty()) {
            label(ctx, Vec2(x, y), "Deploy probes and scan to find signatures", t.textSecondary);
        } else {
            // Column headers
            r.drawText("ID", Vec2(x, y), t.textSecondary, 1.0f);
            r.drawText("Group", Vec2(x + contentW * 0.2f, y), t.textSecondary, 1.0f);
            r.drawText("Type", Vec2(x + contentW * 0.5f, y), t.textSecondary, 1.0f);
            r.drawText("Signal", Vec2(x + contentW * 0.78f, y), t.textSecondary, 1.0f);
            y += 16.0f;
            separator(ctx, Vec2(x, y), contentW);
            y += 4.0f;

            for (size_t i = 0; i < m_probeScanResults.size() && y < maxY - 16.0f; ++i) {
                const auto& res = m_probeScanResults[i];
                if (i % 2 == 1) {
                    r.drawRect(Rect(x, y, contentW, 16.0f),
                               t.bgHeader.withAlpha(0.3f));
                }

                // Signal strength color: red < 25%, yellow < 75%, green >= 75%
                Color sigColor = t.danger;
                if (res.signalStrength >= 75.0f) sigColor = t.success;
                else if (res.signalStrength >= 25.0f) sigColor = t.warning;

                r.drawText(res.id.substr(0, 7),
                           Vec2(x + 2, y + 1), t.textMuted, 1.0f);
                r.drawText(res.group,
                           Vec2(x + contentW * 0.2f, y + 1), t.textPrimary, 1.0f);

                std::string typeStr = res.signalStrength >= 25.0f ? res.type : "???";
                r.drawText(typeStr,
                           Vec2(x + contentW * 0.5f, y + 1), t.textSecondary, 1.0f);

                char sigBuf[16];
                std::snprintf(sigBuf, sizeof(sigBuf), "%.0f%%", res.signalStrength);
                r.drawText(sigBuf,
                           Vec2(x + contentW * 0.78f, y + 1), sigColor, 1.0f);

                // Signal strength bar
                float barX = x + contentW * 0.88f;
                float barW = contentW * 0.12f - 2.0f;
                Rect sigBar(barX, y + 3.0f, barW, 10.0f);
                r.drawProgressBar(sigBar, res.signalStrength / 100.0f, sigColor, t.bgHeader);

                y += 16.0f;
            }
        }

    } else if (titleStr == "Station Services") {
        // Station name
        r.drawText(m_stationData.stationName.empty() ? "Unknown Station" : m_stationData.stationName,
                   Vec2(x, y), t.accentPrimary, 1.0f);
        y += 18.0f;

        // Distance
        char distBuf[64];
        std::snprintf(distBuf, sizeof(distBuf), "Distance: %.0f m", m_stationData.distance);
        r.drawText(distBuf, Vec2(x, y), t.textSecondary, 1.0f);
        y += 16.0f;

        // Docking range
        char rangeBuf[64];
        std::snprintf(rangeBuf, sizeof(rangeBuf), "Docking Range: %.0f m", m_stationData.dockingRange);
        r.drawText(rangeBuf, Vec2(x, y), t.textSecondary, 1.0f);
        y += 20.0f;

        separator(ctx, Vec2(x, y), contentW);
        y += 8.0f;

        // Docking status
        if (m_stationData.isDocked) {
            r.drawText("Status: DOCKED", Vec2(x, y), t.success, 1.0f);
        } else {
            r.drawText("Status: In Space", Vec2(x, y), t.textSecondary, 1.0f);
        }
        y += 20.0f;

        separator(ctx, Vec2(x, y), contentW);
        y += 8.0f;

        // Ship HP bars (shown when docked for repair assessment)
        r.drawText("Ship Status", Vec2(x, y), t.textPrimary, 1.0f);
        y += 18.0f;

        // Shield bar
        r.drawText("Shield", Vec2(x, y + 1), t.textSecondary, 1.0f);
        Rect shieldBar(x + 60.0f, y, contentW - 60.0f, 14.0f);
        Color shieldColor = {0.2f, 0.6f, 1.0f, 1.0f};
        r.drawProgressBar(shieldBar, m_stationData.shieldPct, shieldColor, t.bgHeader);
        char shieldBuf[32];
        std::snprintf(shieldBuf, sizeof(shieldBuf), "%.0f%%", m_stationData.shieldPct * 100.0f);
        r.drawText(shieldBuf, Vec2(x + 64.0f, y + 1), t.textPrimary, 1.0f);
        y += 20.0f;

        // Armor bar
        r.drawText("Armor", Vec2(x, y + 1), t.textSecondary, 1.0f);
        Rect armorBar(x + 60.0f, y, contentW - 60.0f, 14.0f);
        Color armorColor = {0.9f, 0.7f, 0.1f, 1.0f};
        r.drawProgressBar(armorBar, m_stationData.armorPct, armorColor, t.bgHeader);
        char armorBuf[32];
        std::snprintf(armorBuf, sizeof(armorBuf), "%.0f%%", m_stationData.armorPct * 100.0f);
        r.drawText(armorBuf, Vec2(x + 64.0f, y + 1), t.textPrimary, 1.0f);
        y += 20.0f;

        // Hull bar
        r.drawText("Hull", Vec2(x, y + 1), t.textSecondary, 1.0f);
        Rect hullBar(x + 60.0f, y, contentW - 60.0f, 14.0f);
        Color hullColor = {0.9f, 0.2f, 0.2f, 1.0f};
        r.drawProgressBar(hullBar, m_stationData.hullPct, hullColor, t.bgHeader);
        char hullBuf[32];
        std::snprintf(hullBuf, sizeof(hullBuf), "%.0f%%", m_stationData.hullPct * 100.0f);
        r.drawText(hullBuf, Vec2(x + 64.0f, y + 1), t.textPrimary, 1.0f);
        y += 24.0f;

        separator(ctx, Vec2(x, y), contentW);
        y += 8.0f;

        // Repair cost
        if (m_stationData.repairCostIsk > 0.0f) {
            char costBuf[64];
            std::snprintf(costBuf, sizeof(costBuf), "Repair Cost: %.0f ISK", m_stationData.repairCostIsk);
            r.drawText(costBuf, Vec2(x, y), t.warning, 1.0f);
            y += 20.0f;
        }

        // Action buttons
        if (y < maxY - 30.0f) {
            if (m_stationData.isDocked) {
                // Undock button
                Rect undockBtn(x, y, 100.0f, 24.0f);
                if (button(ctx, "UNDOCK", undockBtn)) {
                    if (m_stationUndockCb) m_stationUndockCb();
                }
                // Repair button (only if damage exists)
                if (m_stationData.shieldPct < 1.0f || m_stationData.armorPct < 1.0f || m_stationData.hullPct < 1.0f) {
                    Rect repairBtn(x + 110.0f, y, 100.0f, 24.0f);
                    if (button(ctx, "REPAIR", repairBtn)) {
                        if (m_stationRepairCb) m_stationRepairCb();
                    }
                }
            } else {
                // Dock button (only if in range)
                bool inRange = m_stationData.distance <= m_stationData.dockingRange;
                Rect dockBtn(x, y, 100.0f, 24.0f);
                if (inRange) {
                    if (button(ctx, "DOCK", dockBtn)) {
                        if (m_stationDockCb) m_stationDockCb();
                    }
                } else {
                    r.drawRect(dockBtn, t.bgHeader);
                    r.drawText("DOCK", Vec2(x + 28.0f, y + 4.0f), t.textMuted, 1.0f);
                    r.drawText("(out of range)", Vec2(x + 110.0f, y + 4.0f), t.textMuted, 1.0f);
                }
            }
        }

    } else if (titleStr == "Fleet") {
        if (!m_fleetData.inFleet) {
            label(ctx, Vec2(x, y), "Not in a fleet", t.textSecondary);
            y += 20.0f;
            label(ctx, Vec2(x, y), "Form or join a fleet to see members", t.textSecondary);
        } else {
            // Fleet name
            r.drawText(m_fleetData.fleetName.empty() ? "My Fleet" : m_fleetData.fleetName,
                       Vec2(x, y), t.accentPrimary, 1.0f);
            y += 18.0f;

            // Member count
            char memberBuf[32];
            std::snprintf(memberBuf, sizeof(memberBuf), "Members: %d", m_fleetData.memberCount);
            r.drawText(memberBuf, Vec2(x, y), t.textSecondary, 1.0f);
            y += 20.0f;

            separator(ctx, Vec2(x, y), contentW);
            y += 8.0f;

            // Column headers
            r.drawText("Name", Vec2(x, y), t.textSecondary, 1.0f);
            r.drawText("Ship", Vec2(x + contentW * 0.4f, y), t.textSecondary, 1.0f);
            r.drawText("HP", Vec2(x + contentW * 0.75f, y), t.textSecondary, 1.0f);
            y += 16.0f;
            separator(ctx, Vec2(x, y), contentW);
            y += 4.0f;

            for (size_t i = 0; i < m_fleetData.members.size() && y < maxY - 16.0f; ++i) {
                const auto& member = m_fleetData.members[i];
                if (i % 2 == 1) {
                    r.drawRect(Rect(x, y, contentW, 16.0f), t.bgHeader.withAlpha(0.3f));
                }

                // Name (commander gets star prefix)
                std::string displayName = member.isCommander ? "* " + member.name : member.name;
                Color nameColor = member.isCommander ? t.accentPrimary : t.textPrimary;
                r.drawText(displayName, Vec2(x + 2, y + 1), nameColor, 1.0f);

                // Ship type
                r.drawText(member.shipType, Vec2(x + contentW * 0.4f, y + 1), t.textSecondary, 1.0f);

                // Mini HP bar (shield + armor as stacked thin bar)
                float barX = x + contentW * 0.75f;
                float barW = contentW * 0.25f - 4.0f;
                float barH = 4.0f;
                float barY = y + 6.0f;

                // Shield
                Rect sBar(barX, barY, barW, barH);
                Color shieldC = {0.2f, 0.6f, 1.0f, 1.0f};
                r.drawProgressBar(sBar, member.shieldPct, shieldC, t.bgHeader);

                // Armor (below shield)
                Rect aBar(barX, barY + barH + 1, barW, barH);
                Color armorC = {0.9f, 0.7f, 0.1f, 1.0f};
                r.drawProgressBar(aBar, member.armorPct, armorC, t.bgHeader);

                y += 16.0f;
            }
        }

    } else {
        // Generic fallback
        label(ctx, Vec2(x, y), title, t.textPrimary);
        y += 20.0f;
        separator(ctx, Vec2(x, y), contentW);
    }

    panelEnd(ctx);
}

} // namespace atlas
