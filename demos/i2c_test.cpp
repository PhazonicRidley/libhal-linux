// Copyright 2024 Khalil Estell
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "../include/libhal-linux/i2c.hpp"
#include <array>
#include <iostream>
#include <libhal-util/i2c.hpp>
#include <libhal/error.hpp>
#include <unistd.h>

double rad_to_deg(double rad)
{
  return rad * 180 / 3.14;
}

void print_data(std::array<hal::byte, 6>& data)
{
  uint16_t x = data[0] << 8 | data[1];
  uint16_t y = data[2] << 8 | data[3];
  uint16_t z = data[4] << 8 | data[5];
  std::cout << "X: " << x << " Y: " << y << " Z: " << z << std::endl;
}

int main()
{
  auto bus = hal::linux::i2c("/dev/i2c-1");
  const auto addr = 0x68;
  const auto wake_sensor = std::array<hal::byte, 2>{ 0x6B, 0 };
  hal::write(bus, addr, wake_sensor);
  const auto set_scale = std::array<hal::byte, 2>{ 0xC1, 1 };
  while (true) {
    auto read_buffer = std::array<hal::byte, 6>{};
    auto write_op = std::array<hal::byte, 1>{ 0x3B };
    hal::write(bus, addr, write_op);
    hal::read(bus, addr, read_buffer);
    print_data(read_buffer);
  }

  return 0;
}