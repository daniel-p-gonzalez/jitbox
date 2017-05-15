#pragma once
#include <unistd.h>
#include "coretypes.h"
#include "storagealloc.h"

#ifndef MAP_32BIT
#define MAP_32BIT 0
#endif

namespace jitbox
{

class CodeGenerator
{
public:
    CodeGenerator(bool dump_asm)
        : m_mem(nullptr), m_page_count(0),
          m_dump_asm(dump_asm), PAGE_SIZE(sysconf(_SC_PAGESIZE))
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
        m_mem = (u8*)mmap(0, PAGE_SIZE * m_page_count, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_32BIT, -1, 0);
        memcpy(m_mem, &m_code[0], m_code.size());

        // link
        for( auto patch_loc : m_addresses_to_patch )
        {
            u8* patch_address = m_mem + patch_loc;
            // target address stored, just need relative offset from this.
            // (int*)patch_address points to location in m_mem of function
            //   address operand (under assumption loaded into lower 32bits).
            // *(int*)patch_address loads this stored address.
            // (size_t) cast to expand to 64bit address (to avoid warning).
            // (u8*) to treat this address as a pointer.
            u8* target_address = (u8*)(size_t)*((int*)patch_address);
            // patch_address + 4 to account for address operand
            // (call expects offset from address after instruction and operands)
            int call_offset = (int)(target_address - (patch_address + 4));
            *((int*)patch_address) = (int)call_offset;
        }

        // mark as executable and read-only
        mprotect(m_mem, PAGE_SIZE * m_page_count, PROT_READ | PROT_EXEC);
    }

    // return index of next instruction
    size_t get_offset()
    {
        return m_code.size();
    }

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

    // until code is moved into its final location, cannot calculate relative
    //  jumps. so, store addresses to patch during linking
    std::vector<size_t> m_addresses_to_patch;
    StorageAllocator m_storage_alloc;
    bool m_dump_asm;

private:
    std::vector<u8> m_code;
    const size_t PAGE_SIZE;
    size_t m_page_count;
    u8* m_mem;
};

} // namespace jitbox
