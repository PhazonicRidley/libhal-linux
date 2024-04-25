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

#include "../include/libhal-linux/steady_clock.hpp"
#include <chrono>
#include <cstdio>
#include <libhal-util/steady_clock.hpp>
#include <libhal/error.hpp>
#include <unistd.h>

int main()
{
  using namespace std::chrono_literals;
  using namespace hal::literals;
  auto sc = hal::linux::steady_clock<std::chrono::steady_clock>();
  printf("Clock made!\n");
  for (int i = 0; i < 10; i++) {
    hal::delay(sc, 5s);
    printf("Delayed for a second\n");
  }
}