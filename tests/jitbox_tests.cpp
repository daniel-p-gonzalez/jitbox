#include <iostream>
#include "jitbox.h"

using namespace std;
using namespace jitbox;

int main()
{
    Module module = Module("square_test");

    ValueType return_type = ValueType::i32;
    Function* func = module.new_function("square", return_type);
    Value* x = func->new_param("x", ValueType::i32);

    func->begin_block("entry");
    func->end_block_with_return(func->mul(x, x));

    module.compile();

    typedef int (*square_proto)(int);
    square_proto square = (square_proto)func->get();

    if( square(5) != 25 )
    {
      cerr << "Test FAILED: " << square(5) << " != 25" << endl;
      return 1;
    }
    if( square(16) != 256 )
    {
      cerr << "Test FAILED: " << square(16) << " != 256" << endl;
      return 1;
    }

    cout << "Test PASSED." << endl;
    return 0;
}
