#pragma once
#include <string>

#include <memory.h>
#include <sys/mman.h>
#include <assert.h>

namespace jitbox
{

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

enum class Register
{
    eax, ebx, ecx, edx, edi, esi, esp, ebp,
    rax, rbx, rcx, rdx, rdi, rsi, rsp, rbp,
     r8,  r9, r10, r11, r12, r13, r14, r15
};

enum class ValueType
{
  Int32,
  Float32,
  Pointer,
};

enum class StorageType
{
  Register,
  Stack,
  Heap,
  Unset
};

// named variable, temporary, or constant.
// holds current storage location during code generation.
class Value
{
  Value(std::string name, ValueType value_type)
  : m_name(name), m_value_type(value_type),
    m_heap_location(nullptr), m_storage_type(StorageType::Unset)
  {
  }

  void set_register(Register reg)
  {
    m_storage_type = StorageType::Register;
    m_register = reg;
  }

  void set_stack_offset(size_t offset)
  {
    m_storage_type = StorageType::Stack;
    m_stack_offset = offset;
  }

  void set_heap_location(void* loc)
  {
    m_storage_type = StorageType::Heap;
    m_heap_location = loc;
  }

  Register get_register()
  {
    assert(m_storage_type == StorageType::Register);
    return m_register;
  }

  size_t get_stack_offset()
  {
    assert(m_storage_type == StorageType::Stack);
    return m_stack_offset;
  }

  void* get_heap_location()
  {
    assert(m_storage_type == StorageType::Heap);
    return m_heap_location;
  }

  StorageType get_storage_type()
  {
    return m_storage_type;
  }

public:
  const std::string m_name;
  const ValueType m_value_type;

private:
  size_t m_stack_offset;
  void* m_heap_location;
  Register m_register;
  StorageType m_storage_type;
};

// encapsulates functions/constants for a module
class Module;

// owns its own args/locals/code gen, copied to module on finalize
class Function;

// TODO: move mprotect to module, have functions own (dynamically allocated) mem
class X64CodeGenerator
{
public:
    X64CodeGenerator()
    {
        m_base = m_ip = (u8*)mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        memset(m_base, (u8)0xc3, PAGE_SIZE);
    }

    ~X64CodeGenerator()
    {
        munmap(m_base, PAGE_SIZE);
        m_base = m_ip = nullptr;
    }

    void finalize()
    {
        mprotect(m_base, PAGE_SIZE, PROT_READ | PROT_EXEC);
    }

    void mov(Register reg, void* address)
    {
        switch(reg)
        {
            case Register::rax: EmitByte(0x48); EmitByte(0xb8); break;
            case Register::rbx: EmitByte(0x48); EmitByte(0xbb); break;
            case Register::rcx: EmitByte(0x48); EmitByte(0xb9); break;
            case Register::rdx: EmitByte(0x48); EmitByte(0xba); break;
            case Register::rdi: EmitByte(0x48); EmitByte(0xbf); break;
            case Register::rsi: EmitByte(0x48); EmitByte(0xbe); break;
            case Register::rsp: EmitByte(0x48); EmitByte(0xbc); break;
            case Register::rbp: EmitByte(0x48); EmitByte(0xbd); break;
            case Register::r8:  EmitByte(0x49); EmitByte(0xb8); break;
            case Register::r9:  EmitByte(0x49); EmitByte(0xb9); break;
            case Register::r10: EmitByte(0x49); EmitByte(0xba); break;
            case Register::r11: EmitByte(0x49); EmitByte(0xbb); break;
            case Register::r12: EmitByte(0x49); EmitByte(0xbc); break;
            case Register::r13: EmitByte(0x49); EmitByte(0xbd); break;
            case Register::r14: EmitByte(0x49); EmitByte(0xbe); break;
            case Register::r15: EmitByte(0x49); EmitByte(0xbf); break;
            default: assert(false && "Unhandled register in mov(reg, address)"); break;
        }
        EmitAddress(address);
    }

    void call(Register reg)
    {
        switch(reg)
        {
            case Register::rax: EmitByte(0xff); EmitByte(0xd0); break;
            case Register::rbx: EmitByte(0xff); EmitByte(0xd3); break;
            case Register::rcx: EmitByte(0xff); EmitByte(0xd1); break;
            case Register::rdx: EmitByte(0xff); EmitByte(0xd2); break;
            case Register::rdi: EmitByte(0xff); EmitByte(0xd7); break;
            case Register::rsi: EmitByte(0xff); EmitByte(0xd6); break;
            case Register::rsp: EmitByte(0xff); EmitByte(0xd4); break;
            case Register::rbp: EmitByte(0xff); EmitByte(0xd5); break;
            case Register::r8:  EmitByte(0x41); EmitByte(0xff); EmitByte(0xd0); break;
            case Register::r9:  EmitByte(0x41); EmitByte(0xff); EmitByte(0xd1); break;
            case Register::r10: EmitByte(0x41); EmitByte(0xff); EmitByte(0xd2); break;
            case Register::r11: EmitByte(0x41); EmitByte(0xff); EmitByte(0xd3); break;
            case Register::r12: EmitByte(0x41); EmitByte(0xff); EmitByte(0xd4); break;
            case Register::r13: EmitByte(0x41); EmitByte(0xff); EmitByte(0xd5); break;
            case Register::r14: EmitByte(0x41); EmitByte(0xff); EmitByte(0xd6); break;
            case Register::r15: EmitByte(0x41); EmitByte(0xff); EmitByte(0xd7); break;
            default: assert(false && "Unhandled register in mov(reg, address)"); break;
        }
    }

    // call to (already loaded) c function
    void ccall(Register reg, void* address)
    {
        mov(reg, address);
        call(reg);
    }

    void ret()
    {
        EmitByte(0xc3);
    }

    // temp hack to get work out end-to-end details
    typedef void (*BareFunction) (void);
    void run()
    {
        BareFunction fun = (BareFunction)m_base;
        fun();
    }

private:
    void EmitByte(u8 byte)
    {
//        printf("%hhX ", byte);
        *m_ip++ = byte;
    }

    void EmitAddress(void* _address)
    {
        size_t address = (size_t)_address;
        EmitByte((u8)(address));
        EmitByte((u8)(address >> 8));
        EmitByte((u8)(address >> 16));
        EmitByte((u8)(address >> 24));
        EmitByte((u8)(address >> 32));
        EmitByte((u8)(address >> 40));
        EmitByte((u8)(address >> 48));
        EmitByte((u8)(address >> 56));
    }

private:
    const size_t PAGE_SIZE = 4096;
    u8* m_base;
    u8* m_ip;
};

} // jitbox
