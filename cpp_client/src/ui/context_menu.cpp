#include "ui/context_menu.h"
#include "ui/atlas/atlas_context.h"
#include <iostream>

namespace UI {

ContextMenu::ContextMenu()
    : m_menuType(ContextMenuType::NONE)
    , m_targetIsLocked(false)
    , m_worldX(0.0f)
    , m_worldY(0.0f)
    , m_worldZ(0.0f)
{
}

void ContextMenu::ShowEntityMenu(const std::string& entity_id, bool is_locked) {
    m_menuType = ContextMenuType::ENTITY;
    m_targetEntityId = entity_id;
    m_targetIsLocked = is_locked;
    m_activeSubmenu = -1;
}

void ContextMenu::ShowEmptySpaceMenu(float world_x, float world_y, float world_z) {
    m_menuType = ContextMenuType::EMPTY_SPACE;
    m_worldX = world_x;
    m_worldY = world_y;
    m_worldZ = world_z;
    m_activeSubmenu = -1;
}

void ContextMenu::Close() {
    m_menuType = ContextMenuType::NONE;
    m_activeSubmenu = -1;
}

void ContextMenu::Render() {
    // Legacy stub — rendering now goes through RenderAtlas()
}

void ContextMenu::RenderAtlas(atlas::AtlasContext& ctx) {
    if (m_menuType == ContextMenuType::NONE) return;

    const atlas::Theme& t = ctx.theme();
    auto& r = ctx.renderer();

    // ── EVE-style dark translucent context menu ─────────────────────
    // Menu items for entity context menu
    struct MenuItem {
        const char* label;
        ContextMenuAction action;
        int submenuIdx;  // -1 = no submenu
    };

    std::vector<MenuItem> items;
    if (m_menuType == ContextMenuType::ENTITY) {
        items.push_back({"Approach",       ContextMenuAction::APPROACH,       -1});
        items.push_back({"Orbit",          ContextMenuAction::ORBIT,           0});
        items.push_back({"Keep at Range",  ContextMenuAction::KEEP_AT_RANGE,   1});
        items.push_back({"Warp To",        ContextMenuAction::WARP_TO,         2});
        items.push_back({"Align To",       ContextMenuAction::NAVIGATE_TO,    -1});
        if (m_targetIsLocked) {
            items.push_back({"Unlock Target", ContextMenuAction::UNLOCK_TARGET, -1});
        } else {
            items.push_back({"Lock Target",   ContextMenuAction::LOCK_TARGET,   -1});
        }
        items.push_back({"Look At",        ContextMenuAction::LOOK_AT,        -1});
        items.push_back({"Show Info",      ContextMenuAction::SHOW_INFO,      -1});
    } else {
        items.push_back({"Align To",          ContextMenuAction::NAVIGATE_TO, -1});
        items.push_back({"Bookmark Location", ContextMenuAction::BOOKMARK,    -1});
    }

    // Layout constants
    float itemH   = 24.0f;
    float menuW   = 180.0f;
    float menuH   = items.size() * itemH + 4.0f;
    float padX    = 8.0f;

    // Clamp position so menu stays on screen
    float winW = static_cast<float>(ctx.input().windowW);
    float winH = static_cast<float>(ctx.input().windowH);
    float menuX = std::min(m_screenX, winW - menuW - 4.0f);
    float menuY = std::min(m_screenY, winH - menuH - 4.0f);

    // Background
    atlas::Rect bg(menuX, menuY, menuW, menuH);
    r.drawRect(bg, t.bgPanel);
    r.drawRectOutline(bg, t.borderNormal);

    // Header line (entity name)
    if (m_menuType == ContextMenuType::ENTITY && !m_targetEntityId.empty()) {
        r.drawText(m_targetEntityId,
                   {menuX + padX, menuY + 2.0f},
                   t.accentPrimary);
        menuY += itemH;
    }

    // Draw items
    float curY = menuY + 2.0f;
    for (int i = 0; i < static_cast<int>(items.size()); ++i) {
        atlas::Rect itemRect(menuX, curY, menuW, itemH);
        atlas::WidgetID id = ctx.currentID(items[i].label);

        bool hovered = ctx.isHovered(itemRect);
        if (hovered) {
            r.drawRect(itemRect, t.hover);
            ctx.setHot(id);
        }

        r.drawText(items[i].label, {menuX + padX, curY + 5.0f},
                   hovered ? t.accentSecondary : t.textPrimary);

        // Submenu arrow indicator
        if (items[i].submenuIdx >= 0) {
            r.drawText(">", {menuX + menuW - 16.0f, curY + 5.0f}, t.textSecondary);
        }

        // Click handling
        if (ctx.buttonBehavior(itemRect, id)) {
            if (items[i].submenuIdx >= 0) {
                m_activeSubmenu = items[i].submenuIdx;
            } else {
                switch (items[i].action) {
                    case ContextMenuAction::APPROACH:
                        if (m_onApproach) m_onApproach(m_targetEntityId);
                        break;
                    case ContextMenuAction::NAVIGATE_TO:
                        if (m_menuType == ContextMenuType::ENTITY && m_onApproach) {
                            m_onApproach(m_targetEntityId);
                        } else if (m_onNavigateTo) {
                            m_onNavigateTo(m_worldX, m_worldY, m_worldZ);
                        }
                        break;
                    case ContextMenuAction::LOCK_TARGET:
                        if (m_onLockTarget) m_onLockTarget(m_targetEntityId);
                        break;
                    case ContextMenuAction::UNLOCK_TARGET:
                        if (m_onUnlockTarget) m_onUnlockTarget(m_targetEntityId);
                        break;
                    case ContextMenuAction::LOOK_AT:
                        if (m_onLookAt) m_onLookAt(m_targetEntityId);
                        break;
                    case ContextMenuAction::SHOW_INFO:
                        if (m_onShowInfo) m_onShowInfo(m_targetEntityId);
                        break;
                    case ContextMenuAction::BOOKMARK:
                        if (m_onBookmark) m_onBookmark(m_worldX, m_worldY, m_worldZ);
                        break;
                    default:
                        break;
                }
                Close();
            }
        }
        curY += itemH;
    }

    // ── Submenu rendering ───────────────────────────────────────────
    if (m_activeSubmenu >= 0) {
        struct SubItem { const char* label; int distance_m; };
        std::vector<SubItem> subItems;

        if (m_activeSubmenu == 0) {  // Orbit
            subItems = {{"500 m", 500}, {"1,000 m", 1000}, {"5,000 m", 5000},
                        {"10 km", 10000}, {"20 km", 20000}, {"50 km", 50000}};
        } else if (m_activeSubmenu == 1) {  // Keep at Range
            subItems = {{"1,000 m", 1000}, {"5,000 m", 5000}, {"10 km", 10000},
                        {"20 km", 20000}, {"50 km", 50000}};
        } else {  // Warp To
            subItems = {{"Within 0 m", 0}, {"Within 10 km", 10000},
                        {"Within 50 km", 50000}, {"Within 100 km", 100000}};
        }

        float subW = 130.0f;
        float subH = subItems.size() * itemH + 4.0f;
        float subX = menuX + menuW + 2.0f;
        // Position submenu next to the parent item
        float subY = menuY + 2.0f + (m_activeSubmenu + 1) * itemH;

        // Clamp to screen
        if (subX + subW > winW) subX = menuX - subW - 2.0f;
        if (subY + subH > winH) subY = winH - subH - 4.0f;

        atlas::Rect subBg(subX, subY, subW, subH);
        r.drawRect(subBg, t.bgPanel);
        r.drawRectOutline(subBg, t.borderNormal);

        float sy = subY + 2.0f;
        for (int j = 0; j < static_cast<int>(subItems.size()); ++j) {
            atlas::Rect sr(subX, sy, subW, itemH);
            atlas::WidgetID sid = ctx.currentID(subItems[j].label);
            bool sh = ctx.isHovered(sr);
            if (sh) {
                r.drawRect(sr, t.hover);
                ctx.setHot(sid);
            }
            r.drawText(subItems[j].label, {subX + padX, sy + 5.0f},
                       sh ? t.accentSecondary : t.textPrimary);

            if (ctx.buttonBehavior(sr, sid)) {
                if (m_activeSubmenu == 0 && m_onOrbit) {
                    m_onOrbit(m_targetEntityId, subItems[j].distance_m);
                } else if (m_activeSubmenu == 1 && m_onKeepAtRange) {
                    m_onKeepAtRange(m_targetEntityId, subItems[j].distance_m);
                } else if (m_activeSubmenu == 2 && m_onWarpTo) {
                    m_onWarpTo(m_targetEntityId, subItems[j].distance_m);
                }
                Close();
            }
            sy += itemH;
        }
    }
}

void ContextMenu::RenderOrbitSubmenu() {
}

void ContextMenu::RenderKeepAtRangeSubmenu() {
}

void ContextMenu::RenderWarpToSubmenu() {
}

} // namespace UI
