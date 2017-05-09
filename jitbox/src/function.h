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

enum class Instruction
{
    add, sub, mul, div,
};

struct Expression
{
    std::vector<Value*> args;
    Instruction instruction;
};

class Function
{
public:
    Function(std::string name, ValueType return_type, CodeGenerator* gen)
    : m_name(name), m_return_type(return_type), m_gen(gen)
    {
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

    // TODO: the following is only needed due to end_block_with_return needing
    //  to support returning arbitrary expressions.
    //  add explicit return value to avoid this
    Expression binop(Instruction instr, Value* lhs, Value* rhs)
    {
        Expression expr;
        expr.args.push_back(lhs); expr.args.push_back(rhs);
        expr.instruction = instr;
        return expr;
    }

    Expression add(Value* lhs, Value* rhs)
    {
        return binop(Instruction::add, lhs, rhs);
    }

    Expression sub(Value* lhs, Value* rhs)
    {
        return binop(Instruction::sub, lhs, rhs);
    }

    Expression mul(Value* lhs, Value* rhs)
    {
        return binop(Instruction::mul, lhs, rhs);
    }

    Expression div(Value* lhs, Value* rhs)
    {
        return binop(Instruction::div, lhs, rhs);
    }

    void end_block_with_return()
    {
        m_gen->ret();
    }

    void end_block_with_return(const Expression &expr)
    {
        // TODO: this won't scale to arbitrary expressions
        //       possibly set return val directly instead, e.g.
        // Value* return_val = func->get_return_value(ValueType::i32);
        // func->mul(return_val, lhs, rhs);
        // func->end_block_with_return();
        switch(expr.instruction)
        {
            case Instruction::mul:
                m_gen->mul(Register::rax,
                          expr.args[0]->get_register(),
                          expr.args[1]->get_register());
                break;
        }
        m_gen->ret();
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
    CodeGenerator* m_gen;
};

} // namespace jitbox
