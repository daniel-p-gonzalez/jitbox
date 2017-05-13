#pragma once
#include <vector>
#include <string>

namespace jitbox
{

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;
typedef signed char        i8;
typedef signed short       i16;
typedef signed int         i32;
typedef signed long long   i64;

enum class ValueType
{
    i8,
    u8,
    i16,
    u16,
    i32,
    u32,
    i64,
    u64,
    f32,
    f64,
    pointer,
    none,
};

enum class StorageType
{
    Register,
    Stack,
    Unset
};

namespace RegisterFlag
{
    const u16 GeneralPurpose = 1 << 0;
    // used for storing temporaries
    const u16 Temp = 1 << 1;
    // used for returning values from functions
    const u16 Return = 1 << 2;
    // used to pass args to functions
    const u16 Parameter = 1 << 3;
    // state should be preserved across function calls by callee
    const u16 Preserved = 1 << 4;
}

struct Register
{
    Register(): idx(0), flags(0) {}
    Register(u16 idx, u16 flags) : idx(idx), flags(flags) {}
    u16 idx;
    u16 flags;
};

class Value
{
public:
    Value(std::string name, ValueType value_type)
    : name(name), value_type(value_type), m_storage_type(StorageType::Unset),
      m_stack_offset(0)
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

    StorageType get_storage_type()
    {
        return m_storage_type;
    }

public:
    const std::string name;
    const ValueType value_type;

private:
    size_t m_stack_offset;
    Register m_register;
    StorageType m_storage_type;
};

} // namespace jitbox
