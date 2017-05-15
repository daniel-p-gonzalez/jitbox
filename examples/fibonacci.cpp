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

    const jitbox::Value* one = func->new_constant(int_type, 1);
    const jitbox::Value* two = func->new_constant(int_type, 2);

    func->begin_block("entry");
        // !(x <= 2) ?
        func->branch_if_not(func->cmp_le(x, two), "recurse");
        // // x > 2 ?
        // func->branch_if(func->cmp_gt(x, two), "recurse");
    func->end_block_with_return(one);

    func->begin_block("recurse");
        jitbox::Value* fib1 = func->call(func, func->sub(x, one));
        jitbox::Value* fib2 = func->call(func, func->sub(x, two));
    func->end_block_with_return(func->add(fib1, fib2));

    module.compile();

    typedef int (*fibonacci_proto)(int);
    fibonacci_proto fibonacci = (fibonacci_proto)func->get();

    cout << "result: " << fibonacci(24) << endl;

    return 0;
}
