#pragma once
#include "DockNode.h"
#include <vector>
#include <memory>

namespace atlas::editor {

class EditorLayout {
public:
    void RegisterPanel(EditorPanel* panel);
    void Draw();

    DockNode& Root() { return m_root; }
    const std::vector<EditorPanel*>& Panels() const { return m_panels; }

private:
    DockNode m_root;
    std::vector<EditorPanel*> m_panels;

    static void DrawNode(DockNode& node);
};

}
