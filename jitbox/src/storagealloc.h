#pragma once
#include <memory>
#include "coretypes.h"

namespace jitbox
{

// Allocates storage (register/stack) for a Value
class StorageAllocator
{
public:
    StorageAllocator()
    {
    }

    void set_registers(const std::vector<Register> &registers)
    {
        m_registers = registers;
        m_reg_allocations.resize(registers.size(), nullptr);
    }

    Value* alloc_value(std::string name, ValueType type, u16 flag)
    {
        Value* value = nullptr;
        // TODO: handle spill
        for( auto reg : m_registers )
        {
            if( (reg.flags & flag) &&
                m_reg_allocations[reg.idx] == nullptr )
            {
                m_values.emplace_back(new Value(name, type));
                value = m_values.back().get();
                m_reg_allocations[reg.idx] = value;
                value->set_register(reg);
                break;
            }
        }

        return value;
    }

    // TODO: need to track evictions for codegen to push to stack
    Value* alloc_local(std::string name, ValueType type)
    {
        return alloc_value(name, type, RegisterFlag::GeneralPurpose);
    }

    Value* alloc_temp(ValueType type)
    {
        Value* value = nullptr;
        std::string name = "$temp";
        for( auto reg : m_registers )
        {
            if( reg.flags & RegisterFlag::Temp )
            {
                if( m_reg_allocations[reg.idx] == nullptr )
                {
                    m_values.emplace_back(new Value(name, type));
                    value = m_values.back().get();
                    m_reg_allocations[reg.idx] = value;
                    value->set_register(reg);
                }
                else
                {
                    // fall back to using general purpose register
                    return alloc_value(name, type, RegisterFlag::GeneralPurpose);
                }
                break;
            }
        }

        return value;
    }

    Value* alloc_return(ValueType type)
    {
        Value* value = nullptr;
        std::string name = "$ret";
        for( auto reg : m_registers )
        {
            if( reg.flags & RegisterFlag::Return )
            {
                if( m_reg_allocations[reg.idx] == nullptr )
                {
                    m_values.emplace_back(new Value(name, type));
                    value = m_values.back().get();
                    m_reg_allocations[reg.idx] = value;
                    value->set_register(reg);
                }
                else
                {
                    // steal register
                    // TODO: handle spill
                    value = m_reg_allocations[reg.idx];
                }
                break;
            }
        }

        return value;
    }

    Value* alloc_param(std::string name, ValueType type)
    {
        return alloc_value(name, type, RegisterFlag::Parameter);
    }

private:
    std::vector<Register> m_registers;
    // registers currently reserved by a Value
    // indexed by register idx
    std::vector<Value*> m_reg_allocations;
    std::vector<std::unique_ptr<Value>> m_values;
};

} // namespace jitbox
