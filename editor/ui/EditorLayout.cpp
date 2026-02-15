#include "EditorLayout.h"

namespace atlas::editor {

void EditorLayout::RegisterPanel(EditorPanel* panel) {
    m_panels.push_back(panel);
}

void EditorLayout::Draw() {
    DrawNode(m_root);
}

void EditorLayout::DrawNode(DockNode& node) {
    if (node.split == DockSplit::None) {
        if (node.panel && node.panel->IsVisible()) {
            node.panel->Draw();
        }
        return;
    }

    if (node.a) DrawNode(*node.a);
    if (node.b) DrawNode(*node.b);
}

}
