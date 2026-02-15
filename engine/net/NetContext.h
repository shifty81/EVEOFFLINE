#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <queue>

namespace atlas::net {

enum class NetMode {
    Standalone,
    Client,
    Server,
    P2P_Host,
    P2P_Peer
};

struct Packet {
    uint16_t type = 0;
    uint16_t size = 0;
    uint32_t tick = 0;
    std::vector<uint8_t> payload;
};

struct NetPeer {
    uint32_t id = 0;
    float rtt = 0.0f;
    bool connected = false;
};

struct InputFrame {
    uint32_t tick = 0;
    uint32_t playerID = 0;
    float moveX = 0.0f;
    float moveY = 0.0f;
};

struct WorldSnapshot {
    uint32_t tick = 0;
    std::vector<uint8_t> ecsState;
};

struct QueuedPacket {
    uint32_t destPeerID = 0; // 0 = broadcast
    Packet packet;
};

class NetContext {
public:
    void Init(NetMode mode);
    void Shutdown();

    void Poll();
    void Send(uint32_t peerID, const Packet& pkt);
    void Broadcast(const Packet& pkt);
    void Flush();

    NetMode Mode() const;
    const std::vector<NetPeer>& Peers() const;
    bool IsAuthority() const;

    // Peer management
    uint32_t AddPeer();
    void RemovePeer(uint32_t peerID);

    // Receive incoming packets (from local queue after Poll)
    bool Receive(Packet& outPkt);

    // Lockstep / Rollback
    void SaveSnapshot(uint32_t tick);
    void RollbackTo(uint32_t tick);
    void ReplayFrom(uint32_t tick);

private:
    NetMode m_mode = NetMode::Standalone;
    std::vector<NetPeer> m_peers;
    std::vector<WorldSnapshot> m_snapshots;
    uint32_t m_nextPeerID = 1;

    // Local packet queues for testability
    std::queue<QueuedPacket> m_outgoing;
    std::queue<Packet> m_incoming;
};

}
