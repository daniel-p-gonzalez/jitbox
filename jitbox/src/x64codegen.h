#pragma once
#include "codegen.h"

namespace jitbox
{

// TODO: move mprotect to module, have functions own (dynamically allocated) mem
class X64CodeGenerator : public CodeGenerator
{
public:
    X64CodeGenerator() : CodeGenerator(), m_reg_idx(16)
    {
        m_reg_idx[(size_t)Register::rax] = 0;
        m_reg_idx[(size_t)Register::rcx] = 1;
        m_reg_idx[(size_t)Register::rdx] = 2;
        m_reg_idx[(size_t)Register::rbx] = 3;
        m_reg_idx[(size_t)Register::rsp] = 4;
        m_reg_idx[(size_t)Register::rbp] = 5;
        m_reg_idx[(size_t)Register::rsi] = 6;
        m_reg_idx[(size_t)Register::rdi] = 7;
        m_reg_idx[(size_t)Register::r8] = 8;
        m_reg_idx[(size_t)Register::r9] = 9;
        m_reg_idx[(size_t)Register::r10] = 10;
        m_reg_idx[(size_t)Register::r11] = 11;
        m_reg_idx[(size_t)Register::r12] = 12;
        m_reg_idx[(size_t)Register::r13] = 13;
        m_reg_idx[(size_t)Register::r14] = 14;
        m_reg_idx[(size_t)Register::r15] = 15;
    }

    Register get_return_register()
    {
        return Register::rax;
    }

    void mov(Register reg, long long value)
    {
        u8 reg_idx = m_reg_idx[(size_t)reg];
        long long instr = 0x48c7c0 | (reg_idx >= 8 ? 0x010000 : 0);
        reg_idx %= 8;
        EmitInstruction(instr+reg_idx, 3);
        EmitValue(value, 4);
    }

    void mov(Register dest, Register src)
    {
        EmitInstruction(0x4889, 2);
        // TODO: support arbitrary pairs
        if( dest == Register::rax && src == Register::rdi )
        {
            EmitInstruction(0xf8, 1);
        }
        else if( dest == Register::rax && src == Register::rsi )
        {
            EmitInstruction(0xf0, 1);
        }
        else
        {
            assert(false && "Unhandled register in mov(reg, src)");
        }
    }

    void mov(Register reg, void* address)
    {
        u8 reg_idx = m_reg_idx[(size_t)reg];
        long long instr = 0x48b8 | (reg_idx >= 8 ? 0x0100 : 0);
        reg_idx %= 8;
        EmitInstruction(instr+reg_idx, 2);
        EmitAddress(address);
    }

    void call(Register reg)
    {
        u8 reg_idx = m_reg_idx[(size_t)reg];
        long long instr = 0xffd0 | (reg_idx >= 8 ? 0x410000 : 0);
        size_t byte_count = reg_idx >= 8 ? 3 : 2;
        reg_idx %= 8;
        EmitInstruction(instr+reg_idx, byte_count);
    }

    // call to (already loaded) c function
    void ccall(Register reg, void* address)
    {
        mov(reg, address);
        call(reg);
    }

    void call(void* address)
    {

    }

    void imul(Register dest, Register lhs, Register rhs)
    {
        mov(dest, lhs);
        u8 reg_idx = m_reg_idx[(size_t)rhs];
        long long instr = 0x48f7e8 | (reg_idx >= 8 ? 0x010000 : 0);
        reg_idx %= 8;
        EmitInstruction(instr+reg_idx, 3);
    }

    void ret()
    {
        EmitInstruction(0xc3, 1);
    }
private:
    std::vector<u8> m_reg_idx;
};

} // namespace jitbox
