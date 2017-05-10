#pragma once
#include "coretypes.h"

namespace jitbox
{

class CodeGenerator
{
public:
    CodeGenerator() : m_mem(nullptr), m_page_count(0)
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
    size_t get_offset()
    {
        return m_code.size();
    }

    virtual void mov(Register dest, Register src) = 0;
    virtual void mov(Register reg, void* address) = 0;
    virtual void mov(Register reg, long long value) = 0;
    virtual void call(Register reg) = 0;
    virtual void call(void* address) = 0;
    // call to (already loaded) c function
    virtual void ccall(Register reg, void* address) = 0;
    virtual void imul(Register dest, Register lhs, Register rhs) = 0;
    virtual Register get_return_register() = 0;
    virtual void ret() = 0;
    // temp hack to work out end-to-end details
    typedef void (*BareFunction) (void);
    void run()
    {
        BareFunction fun = (BareFunction)m_mem;
        fun();
    }

    void* get_code()
    {
        return (void*)m_mem;
    }

protected:
    void EmitInstruction(long long instruction, size_t size)
    {
      assert(size > 0);
      for( int i = size - 1; i >= 0; --i )
      {
        m_code.push_back((u8)(instruction >> i*8));
      }
    }

    void EmitValue(long long value, size_t size)
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

private:
    std::vector<u8> m_code;
    const size_t PAGE_SIZE = 4096;
    size_t m_page_count;
    u8* m_mem;
};

} // namespace jitbox
