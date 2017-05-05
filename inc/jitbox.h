#pragma once
#include <string>
#include <vector>

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

class CodeGenerator
{
public:
    CodeGenerator() : m_mem(nullptr) {}
    virtual ~CodeGenerator()
    {
      munmap(m_mem, PAGE_SIZE);
      m_mem = nullptr;
    }

    void finalize()
    {
        m_mem = (u8*)mmap(0, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        memcpy(m_mem, &m_code[0], m_code.size());
        mprotect(m_mem, PAGE_SIZE, PROT_READ | PROT_EXEC);
    }

    virtual void mov(Register reg, void* address) = 0;
    virtual void call(Register reg) = 0;
    // call to (already loaded) c function
    virtual void ccall(Register reg, void* address) = 0;
    virtual void ret() = 0;
    // temp hack to work out end-to-end details
    typedef void (*BareFunction) (void);
    void run()
    {
        BareFunction fun = (BareFunction)m_mem;
        fun();
    }

protected:
    void EmitByte(u8 byte)
    {
//        printf("%hhX ", byte);
        m_code.push_back(byte);
    }

    void EmitWord(u16 word)
    {
//        printf("%hhX ", byte);
        m_code.push_back((u8)(word >> 8));
        m_code.push_back((u8)(word));
    }

    void Emit3Bytes(u32 dword)
    {
//        printf("%hhX ", byte);
        m_code.push_back((u8)(dword >> 16));
        m_code.push_back((u8)(dword >> 8));
        m_code.push_back((u8)(dword));
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
    std::vector<u8> m_code;
    const size_t PAGE_SIZE = 4096;
    u8* m_mem;
};

// TODO: move mprotect to module, have functions own (dynamically allocated) mem
class X64CodeGenerator : public CodeGenerator
{
public:
    void mov(Register reg, void* address)
    {
        switch(reg)
        {
            case Register::rax: EmitWord(0x48b8); break;
            case Register::rbx: EmitWord(0x48bb); break;
            case Register::rcx: EmitWord(0x48b9); break;
            case Register::rdx: EmitWord(0x48ba); break;
            case Register::rdi: EmitWord(0x48bf); break;
            case Register::rsi: EmitWord(0x48be); break;
            case Register::rsp: EmitWord(0x48bc); break;
            case Register::rbp: EmitWord(0x48bd); break;
            case Register::r8:  EmitWord(0x49b8); break;
            case Register::r9:  EmitWord(0x49b9); break;
            case Register::r10: EmitWord(0x49ba); break;
            case Register::r11: EmitWord(0x49bb); break;
            case Register::r12: EmitWord(0x49bc); break;
            case Register::r13: EmitWord(0x49bd); break;
            case Register::r14: EmitWord(0x49be); break;
            case Register::r15: EmitWord(0x49bf); break;
            default: assert(false && "Unhandled register in mov(reg, address)"); break;
        }
        EmitAddress(address);
    }

    void call(Register reg)
    {
        switch(reg)
        {
            case Register::rax: EmitWord(0xffd0); break;
            case Register::rbx: EmitWord(0xffd3); break;
            case Register::rcx: EmitWord(0xffd1); break;
            case Register::rdx: EmitWord(0xffd2); break;
            case Register::rdi: EmitWord(0xffd7); break;
            case Register::rsi: EmitWord(0xffd6); break;
            case Register::rsp: EmitWord(0xffd4); break;
            case Register::rbp: EmitWord(0xffd5); break;
            case Register::r8:  Emit3Bytes(0x41ffd0); break;
            case Register::r9:  Emit3Bytes(0x41ffd1); break;
            case Register::r10: Emit3Bytes(0x41ffd2); break;
            case Register::r11: Emit3Bytes(0x41ffd3); break;
            case Register::r12: Emit3Bytes(0x41ffd4); break;
            case Register::r13: Emit3Bytes(0x41ffd5); break;
            case Register::r14: Emit3Bytes(0x41ffd6); break;
            case Register::r15: Emit3Bytes(0x41ffd7); break;
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
};

} // jitbox
