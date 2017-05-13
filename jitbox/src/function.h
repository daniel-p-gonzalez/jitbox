#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "coretypes.h"
#include "codegen.h"
#include "coretypes.h"

namespace jitbox
{

class Function
{
public:
    Function(std::string name, ValueType return_type, CodeGenerator* gen)
    : m_name(name), m_return_type(return_type),
      m_gen(gen)
    {
    }

    Value* new_param(std::string name, ValueType type)
    {
        return m_gen->alloc_param(name, type);
    }

    Value* new_local(std::string name, ValueType type)
    {
        return m_gen->alloc_local(name, type);
    }

    Value* new_constant(ValueType type, i32 value)
    {
        return m_gen->alloc_constant(type, value);
    }

    void begin_block(std::string block_name)
    {
        assert(m_blocks.find(block_name) == m_blocks.end());
        // associate label with offset of next instruction
        m_blocks[block_name] = m_gen->get_label_offset(block_name);
    }

    Value* mul(Value* lhs, Value* rhs)
    {
        Value* temp = nullptr;
        if( lhs->value_type >= ValueType::i8 &&
            lhs->value_type <= ValueType::u64 )
        {
            assert(lhs->value_type == rhs->value_type);

            temp = m_gen->imul(lhs, rhs);
        }
        else
        {
            assert(false && "Unsupported value type in mul(lhs,rhs)");
        }

        return temp;
    }

    Value* div(Value* lhs, Value* rhs)
    {
        Value* temp = nullptr;
        if( lhs->value_type >= ValueType::i8 &&
            lhs->value_type <= ValueType::u64 )
        {
            assert(lhs->value_type == rhs->value_type);

            temp = m_gen->idiv(lhs, rhs);
        }
        else
        {
            assert(false && "Unsupported value type in div(lhs,rhs)");
        }

        return temp;
    }

    Value* add(Value* lhs, Value* rhs)
    {
        Value* temp = nullptr;
        if( lhs->value_type >= ValueType::i8 &&
            lhs->value_type <= ValueType::u64 )
        {
            assert(lhs->value_type == rhs->value_type);

            temp = m_gen->add(lhs, rhs);
        }
        else
        {
            assert(false && "Unsupported value type in add(lhs,rhs)");
        }

        return temp;
    }

    Value* sub(Value* lhs, Value* rhs)
    {
        Value* temp = nullptr;
        if( lhs->value_type >= ValueType::i8 &&
            lhs->value_type <= ValueType::u64 )
        {
            assert(lhs->value_type == rhs->value_type);

            temp = m_gen->sub(lhs, rhs);
        }
        else
        {
            assert(false && "Unsupported value type in sub(lhs,rhs)");
        }

        return temp;
    }

    void end_block_with_return()
    {
        m_gen->ret();
    }

    void end_block_with_return(Value* value)
    {
        assert(value->value_type == m_return_type);
        m_gen->ret(value);
    }

    void call(void* address)
    {
        m_gen->call(address);
    }

    void finalize()
    {
        m_gen->finalize();
    }

    void* get()
    {
        return m_gen->get_code();
    }

private:
    std::map<std::string, size_t> m_blocks;
    std::string m_name;
    ValueType m_return_type;
    CodeGenerator* m_gen;
};

} // namespace jitbox
