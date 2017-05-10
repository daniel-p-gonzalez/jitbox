// NOTE: This example uses features that haven't yet been implemented
#include <iostream>
#include "jitbox.h"

using namespace std;

int main()
{
    // Generate equivalent of this function:
    // int fib(int x)
    // {
    //   if(x<=2) return 1;
    //   return fib(x-1)+fib(x-2);
    // }

    jitbox::Module module = jitbox::Module("fib_test");

    jitbox::ValueType int_type = jitbox::ValueType::i32;
    jitbox::Function* func = module.new_function("fibonacci", int_type);
    jitbox::Value* x = func->new_param("x", jitbox::ValueType::i32);
    jitbox::Value* return_value = func->get_return_value();

    const jitbox::Value* one = func->new_constant(int_type, 1);
    const jitbox::Value* two = func->new_constant(int_type, 2);

    func->begin_block("entry");
        // !(x <= 2) ?
        func->branch_if_not(func->cmp_le(x, two), "recurse");
        // // x > 2 ?
        // func->branch_if(func->cmp_gt(x, two), "recurse");
        func->assign(return_value, one);
    func->end_block_with_return();

    func->begin_block("recurse");
        jitbox::Value* fib1 = func->call(func, func->sub(x, one));
        jitbox::Value* fib2 = func->call(func, func->sub(x, two));
        func->add(return_value, fib1, fib2)
    func->end_block_with_return();

    module.compile();

    typedef int (*fibonacci_proto)(int);
    fibonacci_proto fibonacci = (fibonacci_proto)func->get();

    cout << "result: " << fibonacci(24) << endl;

    return 0;
}
