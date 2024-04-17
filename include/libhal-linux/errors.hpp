#pragma once
#include <cstring>
#include <errno.h>
#include <libhal/error.hpp>
#include <string>
// This is to be internal, will be in the precompiled shared object

namespace hal::linux {
struct errno_exception : public hal::exception
{
  int m_saved_errno;
  constexpr errno_exception(int p_errno, std::errc p_errc, void* p_instance)
    : exception(p_errc, p_instance)
    , m_saved_errno(p_errno)
  {
  }

  inline void print_errno()
  {
    printf("Exception thrown, saved errno is: %d, errno message: %s\n",
           m_saved_errno,
           strerror(m_saved_errno));
  }
};
struct invalid_character_device : public errno_exception
{
  constexpr invalid_character_device(const std::string& p_file_name,
                                     int p_errno,
                                     void* p_instance)
    : errno_exception(p_errno, std::errc::no_such_device, p_instance)
    , m_invalid_device(p_file_name)
  {
  }

  const std::string& m_invalid_device;
};

}  // namespace hal::linux