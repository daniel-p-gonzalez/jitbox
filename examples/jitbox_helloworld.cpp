#include <iostream>
#include "jitbox.h"

void say_hello()
{
  std::cout << "Hello, jitbox!" << std::endl;
}

int main()
{
  // stream needs to be pulled in by linker for "say_hello",
  //  this is an easy way to ensure it gets linked
  std::cout << "Testing code gen." << std::endl;

  // test of lower level code gen
  jitbox::X64CodeGenerator code_gen = jitbox::X64CodeGenerator();

  // equivalent to:
  // mov rax, say_hello
  // call rax
  code_gen.ccall(jitbox::Register::rax, (void*)say_hello);
  code_gen.ret();
  code_gen.finalize();
  code_gen.run();
}
