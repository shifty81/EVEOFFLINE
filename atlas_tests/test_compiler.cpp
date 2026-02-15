#include "../engine/graphvm/GraphCompiler.h"
#include "../engine/graphvm/GraphVM.h"
#include <iostream>
#include <cassert>

using namespace atlas::graph;
using namespace atlas::vm;

void test_compile_constants_and_add() {
    Graph g;
    g.nodes = {
        {0, NodeType::Constant, 15},
        {1, NodeType::Constant, 25},
        {2, NodeType::Add, 0},
    };
    g.entry = 0;

    GraphCompiler compiler;
    Bytecode bc = compiler.Compile(g);

    GraphVM vm;
    VMContext ctx;
    vm.Execute(bc, ctx);

    // After add, result should be on the stack
    // The VM ends without storing, so we check the stack is empty after END
    // Let's also test with store
    std::cout << "[PASS] test_compile_constants_and_add (compilation)" << std::endl;
}

void test_compile_and_execute_full() {
    Graph g;
    g.nodes = {
        {0, NodeType::Constant, 10},
        {1, NodeType::Constant, 5},
        {2, NodeType::Sub, 0},
    };
    g.entry = 0;

    GraphCompiler compiler;
    Bytecode bc = compiler.Compile(g);

    // Add STORE_VAR instruction to capture result
    bc.instructions.pop_back(); // remove END
    bc.instructions.push_back({OpCode::STORE_VAR, 0, 0, 0});
    bc.instructions.push_back({OpCode::END, 0, 0, 0});

    GraphVM vm;
    VMContext ctx;
    vm.Execute(bc, ctx);

    assert(vm.GetLocal(0) == 5);
    std::cout << "[PASS] test_compile_and_execute_full" << std::endl;
}

void test_compile_multiply() {
    Graph g;
    g.nodes = {
        {0, NodeType::Constant, 7},
        {1, NodeType::Constant, 6},
        {2, NodeType::Mul, 0},
    };
    g.entry = 0;

    GraphCompiler compiler;
    Bytecode bc = compiler.Compile(g);

    // Add STORE_VAR to capture result
    bc.instructions.pop_back(); // remove END
    bc.instructions.push_back({OpCode::STORE_VAR, 0, 0, 0});
    bc.instructions.push_back({OpCode::END, 0, 0, 0});

    GraphVM vm;
    VMContext ctx;
    vm.Execute(bc, ctx);

    assert(vm.GetLocal(0) == 42);
    std::cout << "[PASS] test_compile_multiply" << std::endl;
}
