#include "NetContext.h"
#include <algorithm>

namespace atlas::net {

void NetContext::Init(NetMode mode) {
    m_mode = mode;
    m_peers.clear();
    m_snapshots.clear();
    m_nextPeerID = 1;
    while (!m_outgoing.empty()) m_outgoing.pop();
    while (!m_incoming.empty()) m_incoming.pop();
}

void NetContext::Shutdown() {
    m_peers.clear();
    m_snapshots.clear();
    while (!m_outgoing.empty()) m_outgoing.pop();
    while (!m_incoming.empty()) m_incoming.pop();
    m_mode = NetMode::Standalone;
}

void NetContext::Poll() {
    // In local/loopback mode, move outgoing packets to incoming
    // This enables testing without real sockets
    while (!m_outgoing.empty()) {
        m_incoming.push(m_outgoing.front().packet);
        m_outgoing.pop();
    }
}

void NetContext::Send(uint32_t peerID, const Packet& pkt) {
    QueuedPacket qp;
    qp.destPeerID = peerID;
    qp.packet = pkt;
    m_outgoing.push(qp);
}

void NetContext::Broadcast(const Packet& pkt) {
    QueuedPacket qp;
    qp.destPeerID = 0; // 0 = broadcast
    qp.packet = pkt;
    m_outgoing.push(qp);
}

void NetContext::Flush() {
    // In a real implementation, this would push buffered data to sockets
    // For local mode, Poll() already handles the transfer
}

NetMode NetContext::Mode() const {
    return m_mode;
}

const std::vector<NetPeer>& NetContext::Peers() const {
    return m_peers;
}

bool NetContext::IsAuthority() const {
    return m_mode == NetMode::Server || m_mode == NetMode::P2P_Host;
}

uint32_t NetContext::AddPeer() {
    NetPeer peer;
    peer.id = m_nextPeerID++;
    peer.rtt = 0.0f;
    peer.connected = true;
    m_peers.push_back(peer);
    return peer.id;
}

void NetContext::RemovePeer(uint32_t peerID) {
    m_peers.erase(
        std::remove_if(m_peers.begin(), m_peers.end(),
            [peerID](const NetPeer& p) { return p.id == peerID; }),
        m_peers.end()
    );
}

bool NetContext::Receive(Packet& outPkt) {
    if (m_incoming.empty()) return false;
    outPkt = m_incoming.front();
    m_incoming.pop();
    return true;
}

void NetContext::SaveSnapshot(uint32_t tick) {
    WorldSnapshot snap;
    snap.tick = tick;
    // Stub: serialize ECS state
    m_snapshots.push_back(std::move(snap));
}

void NetContext::RollbackTo(uint32_t tick) {
    // Stub: restore ECS state from snapshot at given tick
    while (!m_snapshots.empty() && m_snapshots.back().tick > tick) {
        m_snapshots.pop_back();
    }
}

void NetContext::ReplayFrom(uint32_t /*tick*/) {
    // Stub: replay input frames from given tick to current tick
}

}
