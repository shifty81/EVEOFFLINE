#pragma once
#include <string>

namespace atlas::editor {

class EditorPanel {
public:
    virtual ~EditorPanel() = default;
    virtual const char* Name() const = 0;
    virtual void Draw() = 0;
    virtual bool IsVisible() const { return m_visible; }
    virtual void SetVisible(bool visible) { m_visible = visible; }

private:
    bool m_visible = true;
};

}
