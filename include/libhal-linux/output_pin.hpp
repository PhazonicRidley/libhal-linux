#pragma once
#include "include/libhal-linux/errors.hpp"
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <libhal/error.hpp>
#include <libhal/output_pin.hpp>
#include <libhal/units.hpp>
#include <linux/gpio.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
typedef struct gpio_v2_line_request gpio_line_request;
typedef struct gpio_v2_line_values gpio_values;
typedef struct gpio_v2_line_config gpio_config;

}  // namespace

namespace hal::linux {

/**
 * @brief Output pin for the linux kernel. Wraps libgpiod 2.1 at the earlist.
 * Assumes a GPIO driver exists and is properly written for the specific
 * hardware to interface with the linux kernel.
 */
class output_pin : public hal::output_pin
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
  output_pin(const std::string& p_chip_name, const std::uint16_t p_pin)
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
    m_line_request.config.flags = GPIO_V2_LINE_FLAG_OUTPUT;
    if (ioctl(m_chip_fd, GPIO_V2_GET_LINE_IOCTL, &m_line_request) < 0) {
      throw errno_exception(errno, std::errc::connection_refused, this);
    }
    m_values.mask = 1;  // only use a single channel
  }
  virtual ~output_pin()
  {
    close(m_line_request.fd);
    close(m_chip_fd);
  }

private:
  int m_chip_fd = -1;
  int m_pin;
  gpio_line_request m_line_request;
  gpio_values m_values;

  void driver_level(bool p_high) override
  {
    m_values.bits = p_high & m_values.mask;
    if (ioctl(m_line_request.fd, GPIO_V2_LINE_SET_VALUES_IOCTL, &m_values) <
        0) {
      char err_msg[20];
      sprintf(err_msg, "Setting Pin: %d", m_pin);
      perror(err_msg);
      throw hal::io_error(this);
    }
  }

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

    // Open Drain
    if (p_settings.open_drain) {
      m_line_request.config.flags |= GPIO_V2_LINE_FLAG_OPEN_DRAIN;
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