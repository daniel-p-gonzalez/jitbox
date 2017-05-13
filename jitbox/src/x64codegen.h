#pragma once
#include "codegen.h"

namespace jitbox
{

class X64CodeGenerator : public CodeGenerator
{
public:
    X64CodeGenerator(bool dump_asm) : CodeGenerator(dump_asm)
    {
        m_reg_names.push_back("rax");
        m_reg_names.push_back("rcx");
        m_reg_names.push_back("rdx");
        m_reg_names.push_back("rbx");
        m_reg_names.push_back("rsp");
        m_reg_names.push_back("rbp");
        m_reg_names.push_back("rsi");
        m_reg_names.push_back("rdi");
        m_reg_names.push_back("r8");
        m_reg_names.push_back("r9");
        m_reg_names.push_back("r10");
        m_reg_names.push_back("r11");
        m_reg_names.push_back("r12");
        m_reg_names.push_back("r13");
        m_reg_names.push_back("r14");
        m_reg_names.push_back("r15");

        std::vector<Register> registers =
        {
             Register(7, RegisterFlag::GeneralPurpose | // rdi
                         RegisterFlag::Parameter),
             Register(6, RegisterFlag::GeneralPurpose | // rsi
                         RegisterFlag::Parameter),
             // TODO: rdx used for mul/div, commenting out general use
             //        until register spill is handled properly
             Register(2, //RegisterFlag::GeneralPurpose | // rdx
                         RegisterFlag::Parameter),
             Register(1, RegisterFlag::GeneralPurpose | // rcx
                         RegisterFlag::Parameter),
             Register(8, RegisterFlag::GeneralPurpose | // r8
                         RegisterFlag::Parameter),
             Register(9, RegisterFlag::GeneralPurpose | // r9
                         RegisterFlag::Parameter),
             Register(10, RegisterFlag::GeneralPurpose), // r10
             Register(11, RegisterFlag::GeneralPurpose), // r11
             Register(12, RegisterFlag::GeneralPurpose | // r12
                          RegisterFlag::Preserved),
             Register(13, RegisterFlag::GeneralPurpose | // r13
                          RegisterFlag::Preserved),
             Register(14, RegisterFlag::GeneralPurpose | // r14
                          RegisterFlag::Preserved),
             Register(15, RegisterFlag::GeneralPurpose | // r15
                          RegisterFlag::Preserved),
             Register(0, RegisterFlag::GeneralPurpose | // rax
                         RegisterFlag::Temp |
                         RegisterFlag::Return),
             Register(3, RegisterFlag::GeneralPurpose | // rbx
                         RegisterFlag::Preserved),
        };

        m_storage_alloc.set_registers(registers);
    }

    std::string reg2str(Register reg)
    {
        return m_reg_names[reg.idx];
    }

    void mov(Register reg, i32 value)
    {
        if(m_dump_asm)
            std::cout << "  mov " << reg2str(reg) << ", " << value << std::endl;

        u64 instr = 0x48c7c0 + (reg.idx >= 8 ? 0x010000 : 0);
        u8 offset = reg.idx % 8;
        EmitInstruction(instr+offset, 3);
        EmitValue(value, 4);
    }

    void mov(Register dest, Register src)
    {
        if( dest.idx == src.idx )
        {
            return;
        }

        if(m_dump_asm)
            std::cout << "  mov " << reg2str(dest) << ", " << reg2str(src) << std::endl;

        u64 instr = 0x4889c0 + (dest.idx >= 8 ? 0x010000 : 0)
                             + (src.idx >= 8 ? 0x040000 : 0);
        u8 dest_offset = dest.idx % 8;
        u8 src_offset = src.idx % 8;
        EmitInstruction(instr+dest_offset+src_offset*8, 3);
    }

    void mov(Register reg, void* address)
    {
        if(m_dump_asm)
            std::cout << "  mov " << reg2str(reg) << ", " << address << std::endl;

        u64 instr = 0x48b8 + (reg.idx >= 8 ? 0x0100 : 0);
        u8 offset = reg.idx % 8;
        EmitInstruction(instr+offset, 2);
        EmitAddress(address);
    }

    void call(Register reg)
    {
        if(m_dump_asm)
            std::cout << "  call " << reg2str(reg) << std::endl;

        u64 instr = 0xffd0 + (reg.idx >= 8 ? 0x410000 : 0);
        size_t byte_count = reg.idx >= 8 ? 3 : 2;
        u8 offset = reg.idx % 8;
        EmitInstruction(instr+offset, byte_count);
    }

    // TODO: this is unecessary once linking is implemented
    void call(void* address)
    {
        Value* temp = m_storage_alloc.alloc_temp(ValueType::pointer);
        Register reg = temp->get_register();
        mov(reg, address);
        call(reg);
    }

    // TODO: refactor arithmetic ops to remove redundency
    Value* imul(Value* lhs, Value* rhs)
    {
        // no destination passed in, so grab a temp register
        Value* result = m_storage_alloc.alloc_temp(lhs->value_type);
        Register src = lhs->get_register();
        Register dest = result->get_register();
        mov(dest, src);

        Register rhs_reg = rhs->get_register();
        auto reg_idx = rhs_reg.idx;
        u64 instr = 0x48f7e8 + (reg_idx >= 8 ? 0x010000 : 0);
        reg_idx %= 8;

        if(m_dump_asm)
            std::cout << "  imul " << reg2str(rhs_reg) << std::endl;

        EmitInstruction(instr+reg_idx, 3);
        return result;
    }

    Value* idiv(Value* lhs, Value* rhs)
    {
        // no destination passed in, so grab a temp register
        Value* result = m_storage_alloc.alloc_temp(lhs->value_type);
        Register src = lhs->get_register();
        Register dest = result->get_register();
        mov(dest, src);

        // sign extend rax to rdx:rax
        // TODO: this special case needs to go through storagealloc in
        //       some way to ensure anything in rdx is spilled out to stack
        if(m_dump_asm)
            std::cout << "  cqo" << std::endl;
        EmitInstruction(0x4899, 2);

        Register rhs_reg = rhs->get_register();
        auto reg_idx = rhs_reg.idx;
        u64 instr = 0x48f7f8 + (reg_idx >= 8 ? 0x010000 : 0);
        reg_idx %= 8;

        if(m_dump_asm)
            std::cout << "  idiv " << reg2str(rhs_reg) << std::endl;

        EmitInstruction(instr+reg_idx, 3);
        return result;
    }

    Value* add(Value* lhs, Value* rhs)
    {
        // no destination passed in, so grab a temp register
        Value* result = m_storage_alloc.alloc_temp(lhs->value_type);
        Register src = lhs->get_register();
        Register dest = result->get_register();
        mov(dest, src);

        Register rhs_reg = rhs->get_register();

        u64 instr = 0x4801c0 + (dest.idx >= 8 ? 0x010000 : 0)
                             + (rhs_reg.idx >= 8 ? 0x040000 : 0);
        u8 dest_offset = dest.idx % 8;
        u8 rhs_offset = rhs_reg.idx % 8;

        if(m_dump_asm)
            std::cout << "  add " << reg2str(dest) << ", "
                      << reg2str(rhs_reg) << std::endl;

        EmitInstruction(instr+dest_offset+rhs_offset*8, 3);
        return result;
    }

    Value* sub(Value* lhs, Value* rhs)
    {
        // no destination passed in, so grab a temp register
        Value* result = m_storage_alloc.alloc_temp(lhs->value_type);
        Register src = lhs->get_register();
        Register dest = result->get_register();
        mov(dest, src);

        Register rhs_reg = rhs->get_register();

        u64 instr = 0x4829c0 + (dest.idx >= 8 ? 0x010000 : 0)
                             + (rhs_reg.idx >= 8 ? 0x040000 : 0);
        u8 dest_offset = dest.idx % 8;
        u8 rhs_offset = rhs_reg.idx % 8;

        if(m_dump_asm)
            std::cout << "  sub " << reg2str(dest) << ", "
                      << reg2str(rhs_reg) << std::endl;

        EmitInstruction(instr+dest_offset+rhs_offset*8, 3);
        return result;
    }

    void ret(Value* value)
    {
        // allocate return register (rax)
        Value* result = m_storage_alloc.alloc_return(value->value_type);
        // mov rax, value_register (assuming it lives in register for now)
        Register src = value->get_register();
        Register dest = result->get_register();
        mov(dest, src);

        if(m_dump_asm)
            std::cout << "  ret" << std::endl;

        EmitInstruction(0xc3, 1);
    }

    void ret()
    {
        if(m_dump_asm)
            std::cout << "  ret" << std::endl;

        EmitInstruction(0xc3, 1);
    }

private:
    std::vector<std::string> m_reg_names;
};

} // namespace jitbox
