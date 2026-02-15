#pragma once
#include "GraphIR.h"
#include "GraphVM.h"

namespace atlas::vm {

class GraphCompiler {
public:
    Bytecode Compile(const graph::Graph& graph);

private:
    void EmitNode(const graph::Node& node);

    Bytecode m_bc;
};

}
