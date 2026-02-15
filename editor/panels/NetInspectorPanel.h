#pragma once
#include "../ui/EditorPanel.h"
#include "../../engine/net/NetContext.h"

namespace atlas::editor {

class NetInspectorPanel : public EditorPanel {
public:
    explicit NetInspectorPanel(net::NetContext& net) : m_net(net) {}

    const char* Name() const override { return "Network"; }
    void Draw() override;

private:
    net::NetContext& m_net;
};

}
