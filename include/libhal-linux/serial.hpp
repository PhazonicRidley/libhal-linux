#pragma once
#include <libhal-util/math.hpp>
#include <libhal/error.hpp>
#include <libhal/serial.hpp>

// Internal includes TODO: move to source
#include "errors.hpp"
#include <algorithm>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

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
class serial : public hal::serial
{

public:
  serial(const std::string& p_file_path, serial::settings p_settings = {})
  {
    configure(p_settings);
    m_fd = open(p_file_path.c_str(), O_RDWR | O_NDELAY | O_NOCTTY);
    if (m_fd < 0) {
      printf("[DEBUG] Failed to open, errno: %s\n", strerror(errno));
      hal::safe_throw(invalid_character_device(p_file_path, this));
    }
  };

  virtual ~serial()
  {
    int res = close(m_fd);
    if (res < 0) {
      printf("[DEBUG] Failed to close\n");
      hal::safe_throw(hal::io_error(this));
    }
  };

private:
  void driver_configure(const settings& p_settings) override
  {
    tcflag_t control_flags = CREAD | CS8 | CLOCAL;
    tcflag_t input_flags = 0;

    // Stop Bit
    if (p_settings.stop == settings::stop_bits::two) {
      control_flags |= CSTOPB;
    }

    // Parity Settings
    switch (p_settings.parity) {
      default:  // shouldn't be needed, fail safe
      case settings::parity::none:
        input_flags |= IGNPAR;
        break;

      case settings::parity::odd:
        control_flags |= PARODD;
      case settings::parity::even:
        control_flags |= PARENB;
        input_flags |= INPCK;
        break;
      case settings::parity::forced0:
      case settings::parity::forced1:
        hal::safe_throw(hal::operation_not_supported(this));
    }

    // Baudrate settings
    uint64_t baud = static_cast<int>(p_settings.baud_rate);
    int internal_baud = 0;

    switch (baud) {
      case 0:
        internal_baud = B0;
        break;
      case 50:
        internal_baud = B50;
        break;
      case 75:
        internal_baud = B75;
        break;
      case 110:
        internal_baud = B110;
        break;
      case 134:
        internal_baud = B134;
        break;
      case 150:
        internal_baud = B150;
        break;
      case 200:
        internal_baud = B200;
        break;
      case 300:
        internal_baud = B300;
        break;
      case 600:
        internal_baud = B600;
        break;
      case 1200:
        internal_baud = B1200;
        break;
      case 1800:
        internal_baud = B1800;
        break;
      case 2400:
        internal_baud = B2400;
        break;
      case 4800:
        internal_baud = B4800;
        break;
      case 9600:
        internal_baud = B9600;
        break;
      case 19200:
        internal_baud = B19200;
        break;
      case 38400:
        internal_baud = B38400;
        break;
      case 57600:
        internal_baud = B57600;
        break;
      case 115200:
        internal_baud = B115200;
        break;
      case 230400:
        internal_baud = B230400;
        break;
      case 460800:
        internal_baud = B460800;
        break;
      case 500000:
        internal_baud = B500000;
        break;
      case 576000:
        internal_baud = B576000;
        break;
      case 921600:
        internal_baud = B921600;
        break;
      case 1000000:
        internal_baud = B1000000;
        break;
      case 1152000:
        internal_baud = B1152000;
        break;
      case 1500000:
        internal_baud = B1500000;
        break;
      case 2000000:
        internal_baud = B2000000;
        break;
      default:
        hal::safe_throw(hal::argument_out_of_domain(this));
    }

    control_flags |= internal_baud;

    m_options.c_cflag = control_flags;
    m_options.c_iflag = input_flags;
    m_options.c_oflag = 0;
    m_options.c_lflag = 0;

    flush();
    tcsetattr(m_fd, TCSANOW, &m_options);
  }

  write_t driver_write(std::span<const hal::byte> p_data) override
  {
    uint32_t write_res = linux_write(m_fd, &p_data.data()[0], p_data.size());
    if (write_res < 0) {
      printf("[DEBUG] Failed to write\n");
      hal::safe_throw(hal::io_error(this));
    }
    return write_t{ .data = p_data.subspan(0, write_res) };
  }

  read_t driver_read(std::span<hal::byte> p_data) override
  {
    uint32_t read_res = linux_read(m_fd, &p_data.data()[0], p_data.size());
    if (read_res < 0) {
      printf("[DEBUG] Failed to read\n");
      hal::safe_throw(hal::io_error(this));
    }
    return read_t{ .data = p_data.subspan(0, read_res),
                   .available = read_res,
                   .capacity = p_data.size() };
  }

  void driver_flush() override
  {
    if (m_fd < 0) {
      printf("[DEBUG] Failed to flush\n");
      hal::safe_throw(hal::operation_not_permitted(this));
    }
    tcflush(m_fd, TCIOFLUSH);  // Flushes both TX and RX
  }

  int m_fd = 0;
  struct termios m_options;
};

}  // namespace hal::linux