#pragma once
#include <vector>
#include <string>

namespace jitbox
{

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef signed char  i8;
typedef signed short i16;
typedef signed int   i32;

enum class Register
{
    eax, ebx, ecx, edx, edi, esi, esp, ebp,
    rax, rbx, rcx, rdx, rdi, rsi, rsp, rbp,
     r8,  r9, r10, r11, r12, r13, r14, r15
};

enum class ValueType
{
  i8,
  u8,
  i16,
  u16,
  i32,
  u32,
  i64,
  u64,
  f32,
  f64,
  pointer,
};

enum class StorageType
{
  Register,
  Stack,
  Heap,
  Unset
};

} // namespace jitbox
