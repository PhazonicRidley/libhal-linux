#pragma once
#include <gpiod.hpp>
#include <iostream>
#include <libhal/error.hpp>
#include <libhal/output_pin.hpp>
#include <string>

namespace hal::linux {

namespace line = gpiod::line;
using gpiod_value = line::value;

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
    : m_chip(p_chip_name)
    , m_line_request(create_line_request(m_chip, p_pin))
    , m_pin(p_pin)
  {
  }
  virtual ~output_pin()
  {
    m_line_request.release();
  }

private:
  gpiod::chip m_chip;
  gpiod::line_request m_line_request;
  std::uint16_t m_pin;

  static gpiod::line_request create_line_request(gpiod::chip& p_chip,
                                                 const std::uint16_t p_pin)
  {
    auto line_request =
      p_chip.prepare_request()
        .add_line_settings(
          p_pin, gpiod::line_settings().set_direction(line::direction::OUTPUT))
        .do_request();
    return line_request;
  }

  void driver_level(bool p_high) override
  {
    gpiod_value value = gpiod_value(p_high);
    try {
      m_line_request.set_value(m_pin, value);
    } catch (...) {
      throw hal::operation_not_permitted(this);
    }
  }

  bool driver_level() override
  {
    gpiod_value res;
    try {
      res = m_line_request.get_value(m_pin);
    } catch (...) {
      throw hal::operation_not_permitted(this);
    }
    return res == gpiod_value::ACTIVE;
  }

  void driver_configure(const settings& p_settings) override
  {
    line::drive new_drive;
    switch (p_settings.resistor) {
      case hal::pin_resistor::none:
        new_drive = line::drive::PUSH_PULL;
        break;
      case hal::pin_resistor::pull_up:
        new_drive = line::drive::OPEN_DRAIN;
        break;
      case hal::pin_resistor::pull_down:
        new_drive = line::drive::OPEN_SOURCE;
        break;
      default:
        throw hal::operation_not_supported(static_cast<void*>(this));
    }
    auto req_builder = m_chip.prepare_request();
    req_builder.add_line_settings(m_pin,
                                  gpiod::line_settings()
                                    .set_direction(line::direction::OUTPUT)
                                    .set_drive(new_drive));
    m_line_request.release();
    m_line_request = req_builder.do_request();
  }
};
}  // namespace hal::linux