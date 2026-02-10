#include "ui/photon/photon_hud.h"

#include <algorithm>
#include <cstdio>

namespace photon {

PhotonHUD::PhotonHUD() = default;
PhotonHUD::~PhotonHUD() = default;

void PhotonHUD::init(int windowW, int windowH) {
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
}

void PhotonHUD::update(PhotonContext& ctx,
                       const ShipHUDData& ship,
                       const std::vector<TargetCardInfo>& targets,
                       const std::vector<OverviewEntry>& overview,
                       const SelectedItemInfo& selectedItem) {
    // Draw elements in back-to-front order

    // 1. Neocom sidebar (left edge)
    neocomBar(ctx, 0.0f, m_neocomWidth,
              static_cast<float>(ctx.input().windowH),
              m_neocomIcons, m_neocomCallback);

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

    // 5. Ship HUD (bottom-center)
    drawShipHUD(ctx, ship);
}

// ── Ship HUD ────────────────────────────────────────────────────────

void PhotonHUD::drawShipHUD(PhotonContext& ctx, const ShipHUDData& ship) {
    float winW = static_cast<float>(ctx.input().windowW);
    float winH = static_cast<float>(ctx.input().windowH);

    // Centre of HUD circle (bottom-center of screen)
    Vec2 hudCentre = {winW * 0.5f, winH - 110.0f};
    float hudRadius = 70.0f;

    // Status arcs (shield/armor/hull)
    shipStatusArcs(ctx, hudCentre, hudRadius,
                   ship.shieldPct, ship.armorPct, ship.hullPct);

    // Capacitor ring (inside the status arcs)
    float capInner = hudRadius - 30.0f;
    float capOuter = hudRadius - 22.0f;
    capacitorRing(ctx, hudCentre, capInner, capOuter,
                  ship.capacitorPct, ship.capSegments);

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
            bool clicked = moduleSlot(ctx, {cx, moduleY}, moduleR,
                                       mod.active, mod.cooldown, mod.color);
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

    // Speed indicator (below module rack)
    speedIndicator(ctx, {hudCentre.x, winH - 12.0f},
                   ship.currentSpeed, ship.maxSpeed);
}

// ── Target Cards ────────────────────────────────────────────────────

void PhotonHUD::drawTargetCards(PhotonContext& ctx,
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

void PhotonHUD::drawOverviewPanel(PhotonContext& ctx,
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

    // Tab header
    std::vector<std::string> tabs = {"All", "Combat", "Mining", "Custom"};
    Rect tabRect = {contentArea.x, contentArea.y, contentArea.w, 24.0f};
    overviewHeader(ctx, tabRect, tabs, 0);

    // Rows
    float rowH = 22.0f;
    float rowY = contentArea.y + 28.0f;
    int maxRows = static_cast<int>((contentArea.h - 28.0f) / rowH);
    int count = std::min(static_cast<int>(entries.size()), maxRows);

    for (int i = 0; i < count; ++i) {
        Rect rowRect = {contentArea.x, rowY + i * rowH, contentArea.w, rowH};
        overviewRow(ctx, rowRect, entries[i], (i % 2 == 1));
    }

    // Scrollbar if needed
    if (static_cast<int>(entries.size()) > maxRows) {
        Rect scrollTrack = {contentArea.right() - 6.0f,
                           contentArea.y + 28.0f,
                           6.0f,
                           contentArea.h - 28.0f};
        scrollbar(ctx, scrollTrack, 0.0f,
                 entries.size() * rowH, contentArea.h - 28.0f);
    }

    panelEnd(ctx);
}

// ── Selected Item Panel ─────────────────────────────────────────────

void PhotonHUD::drawSelectedItemPanel(PhotonContext& ctx,
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
    const char* actions[] = {"O", ">>", "W", "i"};
    for (int i = 0; i < 4; ++i) {
        Rect btn = {btnX, btnY, btnSz, btnSz};
        button(ctx, actions[i], btn);
        btnX += btnSz + btnGap;
    }

    panelEnd(ctx);
}

} // namespace photon
