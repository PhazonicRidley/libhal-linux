#pragma once

#include "include/libhal-linux/errors.hpp"
#include <fcntl.h>
#include <libhal/error.hpp>
#include <libhal/input_pin.hpp>
#include <linux/gpio.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
typedef struct gpio_v2_line_request gpio_line_request;
typedef struct gpio_v2_line_values gpio_values;

}  // namespace

namespace hal::linux {

/**
 * @brief Input pin for the linux kernel. Wraps libgpiod 2.1 at the earlist.
 * Assumes a GPIO driver exists and is properly written for the specific
 * hardware to interface with the linux kernel.
 */
class input_pin : public hal::input_pin
{
public:
  /**
   * @brief Constructor. Takes a *full path* to the GPIO character device and a
   * numeber that is known to said device.
   * @param p_chip_name Full path to GPIO character device.
   * @param p_pin Pin number for said device
   *
   * @throws std::invalid_argument if an invalid chip path was given, an invalid
   * pin number was given, or if a request to said line failed.
   */
  input_pin(const std::string& p_chip_name, const std::uint16_t p_pin)
    : m_pin(p_pin)
  {
    m_chip_fd = open(p_chip_name.c_str(), O_RDONLY);
    if (m_chip_fd < 0) {
      throw invalid_character_device(p_chip_name, errno, this);
    }
    memset(&m_line_request, 0, sizeof(gpio_line_request));
    memset(&m_values, 0, sizeof(gpio_values));
    m_line_request.offsets[0] = p_pin;
    m_line_request.num_lines = 1;
    m_line_request.config.flags =
      GPIO_V2_LINE_FLAG_INPUT | GPIO_V2_LINE_FLAG_BIAS_PULL_UP;
    if (ioctl(m_chip_fd, GPIO_V2_GET_LINE_IOCTL, &m_line_request) < 0) {
      throw errno_exception(errno, std::errc::connection_refused, this);
    }
    m_values.mask = 1;  // only use a single channel
  }

  virtual ~input_pin()
  {
    close(m_line_request.fd);
    close(m_chip_fd);
  }

private:
  int m_chip_fd = -1;
  int m_pin;
  gpio_line_request m_line_request;
  gpio_values m_values;
  bool driver_level() override
  {
    if (ioctl(m_line_request.fd, GPIO_V2_LINE_GET_VALUES_IOCTL, &m_values) <
        0) {
      char err_msg[20];
      sprintf(err_msg, "Getting Pin: %d", m_pin);
      perror(err_msg);
      throw hal::io_error(this);
    }
    return static_cast<bool>(m_values.bits & m_values.mask);
  }

  void driver_configure(const settings& p_settings) override
  {
    // Resistor settings
    switch (p_settings.resistor) {
      default:
      case hal::pin_resistor::none:
        break;
      case hal::pin_resistor::pull_up:
        m_line_request.config.flags |= GPIO_V2_LINE_FLAG_BIAS_PULL_UP;
        break;
      case hal::pin_resistor::pull_down:
        m_line_request.config.flags |= GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN;
        break;
    }

    if (ioctl(m_line_request.fd,
              GPIO_V2_LINE_SET_CONFIG_IOCTL,
              &m_line_request.config) < 0) {
      perror("Failed to configured");
      throw hal::operation_not_permitted(this);
    }
  }
};
}  // namespace hal::linux