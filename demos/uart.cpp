#include "../include/libhal-linux/serial.hpp"
#include <algorithm>
#include <iostream>
#include <libhal-util/serial.hpp>
#include <span>
#include <unistd.h>

int main()
{
  std::cout << "UART test\n";
  auto serial_file_path = "/dev/serial0";
  auto serial_bus = hal::linux::serial(serial_file_path);
  std::string test_str = "Hello from libhal\n";
  std::array<hal::byte, 255> input_buffer = { 0 };
  while (true) {
    hal::print(serial_bus, test_str);
    sleep(1);
    auto read_res = serial_bus.read(input_buffer);
    if (input_buffer.at(0) == '\0') {
      std::cout << "Nothing to read\n";
      continue;
    }
    std::cout << "Len of res buffer: " << read_res.data.size()
              << " len of input buffer: " << input_buffer.size() << "\n";
    auto subspan = read_res.data.subspan(0, read_res.data.size());
    auto read_string = std::string(subspan.begin(), subspan.end());
    sleep(1);
    std::cout << "Read from serial:" << read_string;
  }

  return 0;
}