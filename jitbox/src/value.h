#pragma once

#include "coretypes.h"

namespace jitbox
{
// named variable, temporary, or constant.
// holds current storage location during code generation.
class Value
{
public:
  Value(std::string name, ValueType value_type)
  : m_name(name), m_value_type(value_type),
    m_heap_location(nullptr), m_storage_type(StorageType::Unset)
  {
  }

  void set_register(Register reg)
  {
    m_storage_type = StorageType::Register;
    m_register = reg;
  }

  void set_stack_offset(size_t offset)
  {
    m_storage_type = StorageType::Stack;
    m_stack_offset = offset;
  }

  void set_heap_location(void* loc)
  {
    m_storage_type = StorageType::Heap;
    m_heap_location = loc;
  }

  Register get_register()
  {
    assert(m_storage_type == StorageType::Register);
    return m_register;
  }

  size_t get_stack_offset()
  {
    assert(m_storage_type == StorageType::Stack);
    return m_stack_offset;
  }

  void* get_heap_location()
  {
    assert(m_storage_type == StorageType::Heap);
    return m_heap_location;
  }

  StorageType get_storage_type()
  {
    return m_storage_type;
  }

  ValueType get_value_type()
  {
    return m_value_type;
  }

  std::string get_name()
  {
    return m_name;
  }

private:
  const std::string m_name;
  size_t m_stack_offset;
  void* m_heap_location;
  Register m_register;
  StorageType m_storage_type;
  const ValueType m_value_type;
};

} // namespace jitbox
