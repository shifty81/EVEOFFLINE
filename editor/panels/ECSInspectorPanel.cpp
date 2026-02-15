#include "ECSInspectorPanel.h"

namespace atlas::editor {

void ECSInspectorPanel::Draw() {
    // TODO: Render entity list with component details via Atlas UI
    auto entities = m_world.GetEntities();
    (void)entities;
}

}
