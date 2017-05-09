#include <iostream>
#include "jitbox.h"

using namespace std;

int main()
{
    // Generate equivalent of this function:
    // int square (int i)
    // {
    //   return i * i;
    // }

    jitbox::Module module = jitbox::Module("square_test");

    jitbox::ValueType return_type = jitbox::ValueType::i32;
    jitbox::Function* func = module.new_function("square", return_type);
    jitbox::Value* x = func->new_param("x", jitbox::ValueType::i32);

    func->begin_block("entry");
    func->end_block_with_return(func->mul(x, x));

    module.compile();

    typedef int (*square_proto)(int);
    square_proto square = (square_proto)func->get();

    cout << "5 * 5 = "   << square(5) << endl;
    cout << "3 * 3 = "   << square(3) << endl;
    cout << "9 * 9 = "   << square(9) << endl;
    cout << "16 * 16 = " << square(16) << endl;

    return 0;
}
