#include "NetInspectorPanel.h"

namespace atlas::editor {

void NetInspectorPanel::Draw() {
    // Stub: In a real implementation, this would render via Atlas UI
    // Display mode, peers, RTT, bandwidth, etc.
    auto mode = m_net.Mode();
    auto& peers = m_net.Peers();
    (void)mode;
    (void)peers;
}

}
