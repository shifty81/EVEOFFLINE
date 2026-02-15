#include "GraphCompiler.h"

namespace atlas::vm {

Bytecode GraphCompiler::Compile(const graph::Graph& graph) {
    m_bc = {};

    for (const auto& node : graph.nodes) {
        EmitNode(node);
    }

    m_bc.instructions.push_back({OpCode::END, 0, 0, 0});
    return m_bc;
}

void GraphCompiler::EmitNode(const graph::Node& node) {
    switch (node.type) {
        case graph::NodeType::Constant: {
            uint32_t idx = static_cast<uint32_t>(m_bc.constants.size());
            m_bc.constants.push_back(node.constantValue);
            m_bc.instructions.push_back({OpCode::LOAD_CONST, idx, 0, 0});
            break;
        }

        case graph::NodeType::Add:
            m_bc.instructions.push_back({OpCode::ADD, 0, 0, 0});
            break;

        case graph::NodeType::Sub:
            m_bc.instructions.push_back({OpCode::SUB, 0, 0, 0});
            break;

        case graph::NodeType::Mul:
            m_bc.instructions.push_back({OpCode::MUL, 0, 0, 0});
            break;

        case graph::NodeType::Div:
            m_bc.instructions.push_back({OpCode::DIV, 0, 0, 0});
            break;

        case graph::NodeType::CompareLT:
            m_bc.instructions.push_back({OpCode::CMP_LT, 0, 0, 0});
            break;

        case graph::NodeType::Branch:
            m_bc.instructions.push_back({OpCode::JUMP_IF_FALSE, node.id, 0, 0});
            break;

        case graph::NodeType::Event:
            m_bc.instructions.push_back({OpCode::EMIT_EVENT, static_cast<uint32_t>(node.constantValue), 0, 0});
            break;
    }
}

}
