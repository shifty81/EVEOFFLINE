#include "GraphVM.h"
#include <cassert>

namespace atlas::vm {

void GraphVM::Push(Value v) {
    m_stack.push_back(v);
}

Value GraphVM::Pop() {
    assert(!m_stack.empty());
    Value v = m_stack.back();
    m_stack.pop_back();
    return v;
}

bool GraphVM::PopBool() {
    return Pop() != 0;
}

void GraphVM::Execute(const Bytecode& bc, VMContext& ctx) {
    m_stack.clear();
    m_locals.clear();

    uint32_t ip = 0;

    while (ip < bc.instructions.size()) {
        const Instruction& inst = bc.instructions[ip];

        switch (inst.opcode) {
            case OpCode::NOP:
                break;

            case OpCode::LOAD_CONST:
                Push(bc.constants[inst.a]);
                break;

            case OpCode::LOAD_VAR:
                Push(m_locals[inst.a]);
                break;

            case OpCode::STORE_VAR:
                m_locals[inst.a] = Pop();
                break;

            case OpCode::ADD: {
                auto b = Pop();
                auto a = Pop();
                Push(a + b);
                break;
            }

            case OpCode::SUB: {
                auto b = Pop();
                auto a = Pop();
                Push(a - b);
                break;
            }

            case OpCode::MUL: {
                auto b = Pop();
                auto a = Pop();
                Push(a * b);
                break;
            }

            case OpCode::DIV: {
                auto b = Pop();
                auto a = Pop();
                Push(b != 0 ? a / b : 0);
                break;
            }

            case OpCode::CMP_EQ: {
                auto b = Pop();
                auto a = Pop();
                Push(a == b ? 1 : 0);
                break;
            }

            case OpCode::CMP_LT: {
                auto b = Pop();
                auto a = Pop();
                Push(a < b ? 1 : 0);
                break;
            }

            case OpCode::CMP_GT: {
                auto b = Pop();
                auto a = Pop();
                Push(a > b ? 1 : 0);
                break;
            }

            case OpCode::JUMP:
                ip = inst.a;
                continue;

            case OpCode::JUMP_IF_FALSE:
                if (!PopBool()) {
                    ip = inst.a;
                    continue;
                }
                break;

            case OpCode::EMIT_EVENT:
                // Stub: will be routed to ECS / EventBus
                break;

            case OpCode::END:
                return;
        }

        ++ip;
    }
}

Value GraphVM::GetLocal(uint32_t idx) const {
    auto it = m_locals.find(idx);
    return it != m_locals.end() ? it->second : 0;
}

}
