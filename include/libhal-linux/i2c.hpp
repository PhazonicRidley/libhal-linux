#pragma once

#include <errno.h>
#include <fcntl.h>
#include <libhal/i2c.hpp>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>

namespace {
inline auto linux_read(int fd, void* buf, size_t nbytes)
{
  return read(fd, buf, nbytes);
}

inline auto linux_write(int fd, const void* buf, size_t nbytes)
{
  return write(fd, buf, nbytes);
}
}  // namespace

namespace hal::linux {
class i2c : public hal::i2c
{
public:
  /**
   * @brief Constructs a new i2c linux device
   * @param p_file_path The absolute path the to i2c udev device.
   *
   * @throws hal::io_error if the device was not found or a file descriptor
   * could not be opened.
   */
  i2c(const std::string& p_file_path)
  {
    m_fd = open(p_file_path.c_str(), O_RDWR);
    if (m_fd < 0) {
      throw hal::io_error(this);
    }
  }

  virtual ~i2c()
  {
    close(m_fd);
  }

private:
  void driver_configure(const settings& p_settings) override
  {
    return;
  }
  // TODO: Verify endianess is correct
  void driver_transaction(
    hal::byte p_address,
    std::span<const hal::byte> p_data_out,
    std::span<hal::byte> p_data_in,
    hal::function_ref<hal::timeout_function> p_timeout) override
  {
    // Check if we're a 10 bit address
    // The first 5 bytes MUST be this pattern to be considered a 10 bit address
    constexpr hal::byte ten_bit_mask = 0b1111'0 << 2;
    const auto is_ten_bit = p_address & ten_bit_mask == ten_bit_mask;
    std::uint16_t real_address = 0;
    // TODO: Write than read, right now this only supports one or the other per
    // transaction
    const bool is_reading = p_data_out.empty();
    if (is_ten_bit) {
      real_address = p_address << 8 | p_data_out[0];
    } else {
      real_address = p_address;
    }
    // Enable 10 bit mode if set
    if (ioctl(m_fd, I2C_TENBIT, is_ten_bit) < 0) {
      printf("[DEBUG] Failed 10 bit ioctl, errno is: %d, errno says: %s\n",
             errno,
             strerror(errno));
      throw hal::operation_not_supported(this);
    }

    // Set peripheral address
    if (ioctl(m_fd, I2C_SLAVE, real_address) < 0) {
      printf(
        "[DEBUG] Failed slave setting ioctl, errno is: %d, errno says: %s\n",
        errno,
        strerror(errno));
      throw hal::no_such_device(real_address, this);
    }

    if (is_reading) {
      int res;
      if ((res = linux_read(m_fd, &p_data_in[0], p_data_in.size())) == -1) {
        printf("[DEBUG] Failed reading data, errno is: %d, errno says: %s\n",
               errno,
               strerror(errno));
        throw hal::operation_not_permitted(this);
      }
    } else {
      int res;
      if ((res = linux_write(m_fd, &p_data_out[0], p_data_out.size())) == -1) {
        printf("[DEBUG] Failed writing data, errno is: %d, errno says: %s\n",
               errno,
               strerror(errno));
        throw hal::operation_not_permitted(this);
      }
    }
  }
  int m_fd = 0;
};
}  // namespace hal::linux