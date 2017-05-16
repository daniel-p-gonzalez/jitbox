#include <iostream>
#include <jitbox.h>

using namespace std;
using namespace jitbox;

int main()
{
    X64CodeGenerator gen(false);

    Value* return_value = gen.alloc_constant(ValueType::i32, 42);
    gen.ret(return_value);
    gen.finalize();
    typedef i32 (*func_proto)(void);
    func_proto func = (func_proto)gen.get_code();

    i32 result = func();
    if( result != 42 )
    {
      cerr << "Test FAILED: " << result << " != 42" << endl;
      return 1;
    }

    cout << "Test PASSED." << endl;
    return 0;
}
