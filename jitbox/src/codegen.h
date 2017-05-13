#pragma once
#include "coretypes.h"
#include "storagealloc.h"

namespace jitbox
{

class CodeGenerator
{
public:
    CodeGenerator(bool dump_asm)
        : m_mem(nullptr), m_page_count(0), m_dump_asm(dump_asm)
    {
    }

    virtual ~CodeGenerator()
    {
      munmap(m_mem, PAGE_SIZE*m_page_count);
      m_mem = nullptr;
    }

    void finalize()
    {
        m_page_count = 1 + (m_code.size()/PAGE_SIZE);
        m_mem = (u8*)mmap(0, PAGE_SIZE * m_page_count, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        memcpy(m_mem, &m_code[0], m_code.size());
        mprotect(m_mem, PAGE_SIZE * m_page_count, PROT_READ | PROT_EXEC);
    }

    // return index of next instruction
    size_t get_label_offset(std::string label)
    {
        if(m_dump_asm)
            std::cout << label << ":" << std::endl;

        return m_code.size();
    }

    Value* alloc_param(std::string name, ValueType type)
    {
        return m_storage_alloc.alloc_param(name, type);
    }

    Value* alloc_local(std::string name, ValueType type)
    {
        return m_storage_alloc.alloc_local(name, type);
    }

    Value* alloc_constant(ValueType type, i32 value)
    {
        auto constant = m_storage_alloc.alloc_local("", type);
        mov(constant->get_register(), value);

        return constant;
    }

    virtual void mov(Register dest, Register src) = 0;
    virtual void mov(Register reg, void* address) = 0;
    virtual void mov(Register reg, i32 value) = 0;
    virtual void call(Register reg) = 0;
    virtual void call(void* address) = 0;
    virtual Value* add(Value* lhs, Value* rhs) = 0;
    virtual Value* sub(Value* lhs, Value* rhs) = 0;
    virtual Value* imul(Value* lhs, Value* rhs) = 0;
    virtual Value* idiv(Value* lhs, Value* rhs) = 0;
    virtual void ret(Value* value) = 0;
    virtual void ret() = 0;

    void* get_code()
    {
        return (void*)m_mem;
    }

protected:
    void EmitInstruction(u64 instruction, size_t size)
    {
      assert(size > 0);
      for( int i = size - 1; i >= 0; --i )
      {
        m_code.push_back((u8)(instruction >> i*8));
      }
    }

    void EmitValue(u64 value, size_t size)
    {
      for( size_t i = 0; i < size; ++i )
      {
        m_code.push_back((u8)(value >> i*8));
      }
    }

    void EmitAddress(void* _address)
    {
        size_t address = (size_t)_address;
        EmitValue(address, 8);
    }

    StorageAllocator m_storage_alloc;
    bool m_dump_asm;

private:
    std::vector<u8> m_code;
    const size_t PAGE_SIZE = 4096;
    size_t m_page_count;
    u8* m_mem;
};

} // namespace jitbox
