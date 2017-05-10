#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>

#include "coretypes.h"
#include "codegen.h"
#include "value.h"

namespace jitbox
{

class Function
{
public:
    Function(std::string name, ValueType return_type, CodeGenerator* gen)
    : m_name(name), m_return_type(return_type),
      m_gen(gen), m_return_value(new Value("", return_type))
    {
        m_return_value->set_register(m_gen->get_return_register());
    }

    Value* new_param(std::string name, ValueType type)
    {
        // TODO: implement proper register allocation
        Register reg = m_params.size() == 0 ? Register::rdi : Register::rsi;
        m_params.emplace_back(new Value(name, type));
        Value* param = m_params.back().get();
        param->set_register(reg);
        return param;
    }

    void begin_block(std::string block_name)
    {
        assert(m_blocks.find(block_name) == m_blocks.end());
        // associate label with offset of next instruction
        m_blocks[block_name] = m_gen->get_offset();
    }

    void mul(Value* dest, Value* lhs, Value* rhs)
    {
        if( dest->get_value_type() >= ValueType::i8 &&
            dest->get_value_type() <= ValueType::u64 )
        {
            assert(dest->get_value_type() == lhs->get_value_type());
            assert(dest->get_value_type() == rhs->get_value_type());

            m_gen->imul(dest->get_register(),
                       lhs->get_register(),
                       rhs->get_register());
        }
        else
        {
            assert(false && "Unsupported value type in mul(dest,lhs,rhs)");
        }
    }

    void end_block_with_return()
    {
        m_gen->ret();
    }

    Value* get_return_value()
    {
        return m_return_value.get();
    }

    void call(void* address)
    {
        m_gen->ccall(Register::rax, address);
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
    std::vector<std::unique_ptr<Value>> m_params;
    std::unique_ptr<Value> m_return_value;
    CodeGenerator* m_gen;
};

} // namespace jitbox
