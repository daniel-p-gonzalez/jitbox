#include <iostream>
#include "jitbox.h"

using namespace std;

void say_hello(long long n)
{
    cout << "Hello, jitbox! Got arg: " << n << endl;
}

void test_square()
{
    // int square (int i)
    // {
    //   return i * i;
    // }

    jitbox::Module module = jitbox::Module("square_test");

    jitbox::ValueType return_type = jitbox::ValueType::i32;
    jitbox::Function* func = module.new_function("square", return_type);
    jitbox::Value* x = func->new_param("x", jitbox::ValueType::i32);
    // TODO: possibly set return val directly instead of end_block_with_return()
    // jitbox::Value* return_value = func->get_return_value(jitbox::ValueType::i32);

    func->begin_block("entry");
    func->end_block_with_return(func->mul(x, x));

    module.compile();

    typedef int (*square_proto)(int);
    square_proto square = (square_proto)func->get();

    cout << "5 * 5 = "   << square(5) << endl;
    cout << "3 * 3 = "   << square(3) << endl;
    cout << "9 * 9 = "   << square(9) << endl;
    cout << "16 * 16 = " << square(16) << endl;
}

int main()
{
    // test of lower level code gen
    jitbox::X64CodeGenerator code_gen = jitbox::X64CodeGenerator();

    // equivalent to:
    // mov rdi, 17
    // mov rax, say_hello
    // call say_hello
    code_gen.mov(jitbox::Register::rdi, 17);
    code_gen.ccall(jitbox::Register::rax, (void*)say_hello);
    code_gen.ret();
    code_gen.finalize();
    code_gen.run();

    test_square();
}

// void test_fibonacci()
// {
//     // int fib(int x)
//     // {
//     //   if(x<=2) return 1;
//     //   return fib(x-1)+fib(x-2);
//     // }
//
//     jitbox::Module module = jitbox::Module("fib_test");
//
//     jitbox::ValueType int_type = jitbox::ValueType::i32;
//     jitbox::Function* func = module.new_function("fibonacci", int_type);
//     jitbox::Value* x = func->new_param("x", jitbox::ValueType::i32);
//
//     const jitbox::Value* one = func->new_constant(int_type, 1);
//     const jitbox::Value* two = func->new_constant(int_type, 2);
//
//     func->begin_block("entry");
//         // !(x <= 2) ?
//         func->branch_if_not(func->cmp_le(x, two), "recurse");
//         // // x > 2 ?
//         // func->branch_if(func->cmp_gt(x, two), "recurse");
//     func->end_block_with_return(one);
//
//     func->begin_block("recurse");
//         jitbox::Value* fib1 = func->call(func, func->sub(x, one));
//         jitbox::Value* fib2 = func->call(func, func->sub(x, two));
//     func->end_block_with_return(func->add(fib1, fib2));
//
//     module.compile();
//
//     typedef int (*fibonacci_proto)(int);
//     fibonacci_proto fibonacci = (fibonacci_proto)func->get();
//
//     cout << "result: " << fibonacci(24) << endl;
// }
