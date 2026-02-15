#pragma once
#include <vector>
#include <cstdint>

namespace atlas::graph {

using NodeID = uint32_t;

enum class NodeType {
    Constant,
    Add,
    Sub,
    Mul,
    Div,
    CompareLT,
    Branch,
    Event,
};

struct Node {
    NodeID id;
    NodeType type;
    int64_t constantValue = 0;
};

struct Edge {
    NodeID from;
    NodeID to;
};

struct Graph {
    std::vector<Node> nodes;
    std::vector<Edge> edges;
    NodeID entry = 0;
};

}
