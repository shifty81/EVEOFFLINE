#include "../engine/net/NetContext.h"
#include <iostream>
#include <cassert>

using namespace atlas::net;

void test_net_init() {
    NetContext net;
    net.Init(NetMode::Server);

    assert(net.Mode() == NetMode::Server);
    assert(net.IsAuthority());
    assert(net.Peers().empty());

    std::cout << "[PASS] test_net_init" << std::endl;
}

void test_net_authority() {
    NetContext net;

    net.Init(NetMode::Client);
    assert(!net.IsAuthority());

    net.Init(NetMode::Server);
    assert(net.IsAuthority());

    net.Init(NetMode::P2P_Host);
    assert(net.IsAuthority());

    net.Init(NetMode::P2P_Peer);
    assert(!net.IsAuthority());

    net.Init(NetMode::Standalone);
    assert(!net.IsAuthority());

    std::cout << "[PASS] test_net_authority" << std::endl;
}

void test_net_shutdown() {
    NetContext net;
    net.Init(NetMode::Server);
    net.Shutdown();

    assert(net.Mode() == NetMode::Standalone);
    assert(net.Peers().empty());

    std::cout << "[PASS] test_net_shutdown" << std::endl;
}
