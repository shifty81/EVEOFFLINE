#include "../engine/net/NetContext.h"
#include <iostream>
#include <cassert>

using namespace atlas::net;

void test_net_add_peer() {
    NetContext net;
    net.Init(NetMode::Server);

    uint32_t id1 = net.AddPeer();
    uint32_t id2 = net.AddPeer();

    assert(id1 != id2);
    assert(net.Peers().size() == 2);
    assert(net.Peers()[0].connected);
    assert(net.Peers()[1].connected);

    std::cout << "[PASS] test_net_add_peer" << std::endl;
}

void test_net_remove_peer() {
    NetContext net;
    net.Init(NetMode::Server);

    uint32_t id1 = net.AddPeer();
    net.AddPeer();
    assert(net.Peers().size() == 2);

    net.RemovePeer(id1);
    assert(net.Peers().size() == 1);

    std::cout << "[PASS] test_net_remove_peer" << std::endl;
}

void test_net_send_receive() {
    NetContext net;
    net.Init(NetMode::Server);

    uint32_t peer = net.AddPeer();

    Packet pkt;
    pkt.type = 42;
    pkt.tick = 10;
    pkt.payload = {1, 2, 3, 4};
    pkt.size = 4;

    net.Send(peer, pkt);

    // Before Poll, nothing in incoming
    Packet received;
    assert(!net.Receive(received));

    // Poll moves outgoing to incoming
    net.Poll();

    assert(net.Receive(received));
    assert(received.type == 42);
    assert(received.tick == 10);
    assert(received.payload.size() == 4);
    assert(received.payload[0] == 1);

    // No more packets
    assert(!net.Receive(received));

    std::cout << "[PASS] test_net_send_receive" << std::endl;
}

void test_net_broadcast_receive() {
    NetContext net;
    net.Init(NetMode::Server);

    net.AddPeer();
    net.AddPeer();

    Packet pkt;
    pkt.type = 100;
    pkt.tick = 5;

    net.Broadcast(pkt);
    net.Poll();

    Packet received;
    assert(net.Receive(received));
    assert(received.type == 100);
    // Only one broadcast packet in the queue
    assert(!net.Receive(received));

    std::cout << "[PASS] test_net_broadcast_receive" << std::endl;
}

void test_net_shutdown_clears_queues() {
    NetContext net;
    net.Init(NetMode::Server);

    uint32_t peer = net.AddPeer();

    Packet pkt;
    pkt.type = 1;
    net.Send(peer, pkt);
    net.Poll();

    net.Shutdown();

    Packet received;
    assert(!net.Receive(received));
    assert(net.Peers().empty());
    assert(net.Mode() == NetMode::Standalone);

    std::cout << "[PASS] test_net_shutdown_clears_queues" << std::endl;
}
