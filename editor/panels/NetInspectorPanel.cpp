#include "NetInspectorPanel.h"

namespace atlas::editor {

void NetInspectorPanel::Draw() {
    // TODO: Render network mode, peer list, RTT, bandwidth via Atlas UI
    auto mode = m_net.Mode();
    auto& peers = m_net.Peers();
    (void)mode;
    (void)peers;
}

}
