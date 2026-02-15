#include "ECSInspectorPanel.h"

namespace atlas::editor {

void ECSInspectorPanel::Draw() {
    // Stub: In a real implementation, this would render via Atlas UI
    // For now, it demonstrates the panel structure
    auto entities = m_world.GetEntities();
    // Draw entity list, component details, etc.
    (void)entities;
}

}
