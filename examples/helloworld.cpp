#include <iostream>
#include "jitbox.h"

using namespace std;

void print(std::string str)
{
    cout << str << endl;
}

int main()
{
    // Generate equivalent of this function:
    // void hello(std::string str)
    // {
    //   print(str);
    // }

    jitbox::Module module = jitbox::Module("helloworld");

    // module.set_option(jitbox::JitOption::DUMP_ASM, true);

    jitbox::ValueType return_type = jitbox::ValueType::none;
    jitbox::Function* func = module.new_function("hello", return_type);
    jitbox::Value* str = func->new_param("str", jitbox::ValueType::pointer);

    func->begin_block("entry");
    func->call((void*)print);
    func->end_block_with_return();

    module.compile();

    typedef void (*hello_proto)(std::string);
    hello_proto hello = (hello_proto)func->get();

    hello("Hello, jitbox!");

    return 0;
}
