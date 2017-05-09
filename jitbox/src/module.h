#pragma once
#include <string>
#include <vector>
#include <memory>

#include "coretypes.h"
#include "function.h"
#include "x64codegen.h"

namespace jitbox
{

class Module
{
public:
    Module(std::string name) : m_name(name)
    {
    }

    Function* new_function(std::string name, ValueType return_type)
    {
        m_jitters.emplace_back(new X64CodeGenerator());
        m_functions.emplace_back(new Function(name, return_type, m_jitters.back().get()));
        return m_functions.back().get();
    }

    void compile()
    {
        for( auto &func : m_functions )
        {
            func.get()->finalize();
        }
    }
private:
    std::vector<std::unique_ptr<Function>> m_functions;
    std::vector<std::unique_ptr<CodeGenerator>> m_jitters;
    std::string m_name;
};

} // namespace jitbox
