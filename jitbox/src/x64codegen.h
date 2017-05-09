#pragma once
#include "codegen.h"

namespace jitbox
{

// TODO: move mprotect to module, have functions own (dynamically allocated) mem
class X64CodeGenerator : public CodeGenerator
{
public:
    void mov(Register reg, long long value)
    {
        switch(reg)
        {
            case Register::rdi: EmitInstruction(0x48c7c7, 3); break;
            default: assert(false && "Unhandled register in mov(reg, value)"); break;
        }
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
        switch(reg)
        {
            case Register::rax: EmitInstruction(0x48b8, 2); break;
            case Register::rbx: EmitInstruction(0x48bb, 2); break;
            case Register::rcx: EmitInstruction(0x48b9, 2); break;
            case Register::rdx: EmitInstruction(0x48ba, 2); break;
            case Register::rdi: EmitInstruction(0x48bf, 2); break;
            case Register::rsi: EmitInstruction(0x48be, 2); break;
            case Register::rsp: EmitInstruction(0x48bc, 2); break;
            case Register::rbp: EmitInstruction(0x48bd, 2); break;
            case Register::r8:  EmitInstruction(0x49b8, 2); break;
            case Register::r9:  EmitInstruction(0x49b9, 2); break;
            case Register::r10: EmitInstruction(0x49ba, 2); break;
            case Register::r11: EmitInstruction(0x49bb, 2); break;
            case Register::r12: EmitInstruction(0x49bc, 2); break;
            case Register::r13: EmitInstruction(0x49bd, 2); break;
            case Register::r14: EmitInstruction(0x49be, 2); break;
            case Register::r15: EmitInstruction(0x49bf, 2); break;
            default: assert(false && "Unhandled register in mov(reg, address)"); break;
        }
        EmitAddress(address);
    }

    void call(Register reg)
    {
        switch(reg)
        {
            case Register::rax: EmitInstruction(0xffd0, 2); break;
            case Register::rbx: EmitInstruction(0xffd3, 2); break;
            case Register::rcx: EmitInstruction(0xffd1, 2); break;
            case Register::rdx: EmitInstruction(0xffd2, 2); break;
            case Register::rdi: EmitInstruction(0xffd7, 2); break;
            case Register::rsi: EmitInstruction(0xffd6, 2); break;
            case Register::rsp: EmitInstruction(0xffd4, 2); break;
            case Register::rbp: EmitInstruction(0xffd5, 2); break;
            case Register::r8:  EmitInstruction(0x41ffd0, 3); break;
            case Register::r9:  EmitInstruction(0x41ffd1, 3); break;
            case Register::r10: EmitInstruction(0x41ffd2, 3); break;
            case Register::r11: EmitInstruction(0x41ffd3, 3); break;
            case Register::r12: EmitInstruction(0x41ffd4, 3); break;
            case Register::r13: EmitInstruction(0x41ffd5, 3); break;
            case Register::r14: EmitInstruction(0x41ffd6, 3); break;
            case Register::r15: EmitInstruction(0x41ffd7, 3); break;
            default: assert(false && "Unhandled register in mov(reg, address)"); break;
        }
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

    void mul(Register dest, Register lhs, Register rhs)
    {
        mov(dest, lhs);
        EmitInstruction(0x48f7, 2);
        switch(rhs)
        {
            case Register::rdi: EmitInstruction(0xef, 1); break;
            case Register::rsi: EmitInstruction(0xee, 1); break;
            case Register::rdx: EmitInstruction(0xea, 1); break;
            default: assert(false && "Unhandled register in mul(dest, lhs, rhs)"); break;
        }
    }

    void ret()
    {
        EmitInstruction(0xc3, 1);
    }
};

} // namespace jitbox
