#include "../engine/graphvm/GraphVM.h"
#include <iostream>
#include <cassert>

using namespace atlas::vm;

void test_basic_arithmetic() {
    GraphVM vm;
    Bytecode bc;

    // Program: push 10, push 20, add, store to var 0
    bc.constants = {10, 20};
    bc.instructions = {
        {OpCode::LOAD_CONST, 0, 0, 0},
        {OpCode::LOAD_CONST, 1, 0, 0},
        {OpCode::ADD, 0, 0, 0},
        {OpCode::STORE_VAR, 0, 0, 0},
        {OpCode::END, 0, 0, 0}
    };

    VMContext ctx;
    vm.Execute(bc, ctx);

    assert(vm.GetLocal(0) == 30);
    std::cout << "[PASS] test_basic_arithmetic" << std::endl;
}

void test_subtraction() {
    GraphVM vm;
    Bytecode bc;

    bc.constants = {100, 30};
    bc.instructions = {
        {OpCode::LOAD_CONST, 0, 0, 0},
        {OpCode::LOAD_CONST, 1, 0, 0},
        {OpCode::SUB, 0, 0, 0},
        {OpCode::STORE_VAR, 0, 0, 0},
        {OpCode::END, 0, 0, 0}
    };

    VMContext ctx;
    vm.Execute(bc, ctx);

    assert(vm.GetLocal(0) == 70);
    std::cout << "[PASS] test_subtraction" << std::endl;
}

void test_multiplication() {
    GraphVM vm;
    Bytecode bc;

    bc.constants = {6, 7};
    bc.instructions = {
        {OpCode::LOAD_CONST, 0, 0, 0},
        {OpCode::LOAD_CONST, 1, 0, 0},
        {OpCode::MUL, 0, 0, 0},
        {OpCode::STORE_VAR, 0, 0, 0},
        {OpCode::END, 0, 0, 0}
    };

    VMContext ctx;
    vm.Execute(bc, ctx);

    assert(vm.GetLocal(0) == 42);
    std::cout << "[PASS] test_multiplication" << std::endl;
}

void test_division() {
    GraphVM vm;
    Bytecode bc;

    bc.constants = {100, 5};
    bc.instructions = {
        {OpCode::LOAD_CONST, 0, 0, 0},
        {OpCode::LOAD_CONST, 1, 0, 0},
        {OpCode::DIV, 0, 0, 0},
        {OpCode::STORE_VAR, 0, 0, 0},
        {OpCode::END, 0, 0, 0}
    };

    VMContext ctx;
    vm.Execute(bc, ctx);

    assert(vm.GetLocal(0) == 20);
    std::cout << "[PASS] test_division" << std::endl;
}

void test_division_by_zero() {
    GraphVM vm;
    Bytecode bc;

    bc.constants = {100, 0};
    bc.instructions = {
        {OpCode::LOAD_CONST, 0, 0, 0},
        {OpCode::LOAD_CONST, 1, 0, 0},
        {OpCode::DIV, 0, 0, 0},
        {OpCode::STORE_VAR, 0, 0, 0},
        {OpCode::END, 0, 0, 0}
    };

    VMContext ctx;
    vm.Execute(bc, ctx);

    assert(vm.GetLocal(0) == 0);
    std::cout << "[PASS] test_division_by_zero" << std::endl;
}

void test_comparison() {
    GraphVM vm;
    Bytecode bc;

    bc.constants = {5, 10};
    bc.instructions = {
        {OpCode::LOAD_CONST, 0, 0, 0},
        {OpCode::LOAD_CONST, 1, 0, 0},
        {OpCode::CMP_LT, 0, 0, 0},
        {OpCode::STORE_VAR, 0, 0, 0},
        {OpCode::END, 0, 0, 0}
    };

    VMContext ctx;
    vm.Execute(bc, ctx);

    assert(vm.GetLocal(0) == 1);
    std::cout << "[PASS] test_comparison" << std::endl;
}

void test_conditional_jump() {
    GraphVM vm;
    Bytecode bc;

    // Push 0 (false), jump_if_false to instruction 4 (skip store of 999), store 42
    bc.constants = {0, 999, 42};
    bc.instructions = {
        {OpCode::LOAD_CONST, 0, 0, 0},      // 0: push 0
        {OpCode::JUMP_IF_FALSE, 4, 0, 0},    // 1: jump to 4 if false
        {OpCode::LOAD_CONST, 1, 0, 0},       // 2: push 999 (skipped)
        {OpCode::STORE_VAR, 0, 0, 0},        // 3: store 999 (skipped)
        {OpCode::LOAD_CONST, 2, 0, 0},       // 4: push 42
        {OpCode::STORE_VAR, 0, 0, 0},        // 5: store 42
        {OpCode::END, 0, 0, 0}               // 6: end
    };

    VMContext ctx;
    vm.Execute(bc, ctx);

    assert(vm.GetLocal(0) == 42);
    std::cout << "[PASS] test_conditional_jump" << std::endl;
}

void test_variables() {
    GraphVM vm;
    Bytecode bc;

    // Store 100 to var 0, load var 0, add 50, store to var 1
    bc.constants = {100, 50};
    bc.instructions = {
        {OpCode::LOAD_CONST, 0, 0, 0},
        {OpCode::STORE_VAR, 0, 0, 0},
        {OpCode::LOAD_VAR, 0, 0, 0},
        {OpCode::LOAD_CONST, 1, 0, 0},
        {OpCode::ADD, 0, 0, 0},
        {OpCode::STORE_VAR, 1, 0, 0},
        {OpCode::END, 0, 0, 0}
    };

    VMContext ctx;
    vm.Execute(bc, ctx);

    assert(vm.GetLocal(0) == 100);
    assert(vm.GetLocal(1) == 150);
    std::cout << "[PASS] test_variables" << std::endl;
}
