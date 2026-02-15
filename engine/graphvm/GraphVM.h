#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>

namespace atlas::vm {

using EntityID = uint32_t;
using Value = int64_t;

enum class OpCode : uint8_t {
    NOP = 0,
    LOAD_CONST,
    LOAD_VAR,
    STORE_VAR,

    ADD,
    SUB,
    MUL,
    DIV,

    CMP_EQ,
    CMP_LT,
    CMP_GT,

    JUMP,
    JUMP_IF_FALSE,

    EMIT_EVENT,
    END
};

struct Instruction {
    OpCode opcode;
    uint32_t a;
    uint32_t b;
    uint32_t c;
};

struct Bytecode {
    std::vector<Instruction> instructions;
    std::vector<Value> constants;
};

struct VMContext {
    EntityID entity = 0;
    uint64_t tick = 0;
};

class GraphVM {
public:
    void Execute(const Bytecode& bytecode, VMContext& ctx);

    Value GetLocal(uint32_t idx) const;
    const std::vector<Value>& GetStack() const { return m_stack; }

private:
    std::vector<Value> m_stack;
    std::unordered_map<uint32_t, Value> m_locals;

    bool PopBool();
    Value Pop();
    void Push(Value v);
};

}
