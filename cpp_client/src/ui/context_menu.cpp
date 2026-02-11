#include "ui/context_menu.h"
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
}

void ContextMenu::ShowEmptySpaceMenu(float world_x, float world_y, float world_z) {
    m_menuType = ContextMenuType::EMPTY_SPACE;
    m_worldX = world_x;
    m_worldY = world_y;
    m_worldZ = world_z;
}

void ContextMenu::Close() {
    m_menuType = ContextMenuType::NONE;
}

void ContextMenu::Render() {
    // Rendering is handled by the Atlas/RmlUi system
}

void ContextMenu::RenderOrbitSubmenu() {
}

void ContextMenu::RenderKeepAtRangeSubmenu() {
}

void ContextMenu::RenderWarpToSubmenu() {
}

} // namespace UI
