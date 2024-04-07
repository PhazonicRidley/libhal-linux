#pragma once
#include <libhal/error.hpp>
#include <string>
// This is to be internal, will be in the precompiled shared object

namespace hal::linux {
struct invalid_character_device : public hal::exception
{
  constexpr invalid_character_device(const std::string& p_file_name,
                                     void* p_instance)
    : exception(std::errc::no_such_device, p_instance)
    , m_invalid_device(p_file_name)
  {
  }

  const std::string& m_invalid_device;
};
}  // namespace hal::linux