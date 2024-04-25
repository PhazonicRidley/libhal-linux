#include <chrono>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <unistd.h>

using namespace std;

int main()
{
  chrono::time_point<chrono::high_resolution_clock> prev;
  chrono::time_point<chrono::high_resolution_clock> s;
  chrono::time_point<chrono::high_resolution_clock> e;
  auto max_distance = chrono::duration<double>(0);
  for (int i = 0; i < 50; i++) {
    const auto ts = chrono::high_resolution_clock::now();
    std::cout << ts.time_since_epoch().count() << std::endl;
    auto dist = prev - ts;

    prev = ts;
  }

  std::cout << max_distance.count() << std::endl
            << "end: " << e.time_since_epoch().count() << " "
            << s.time_since_epoch().count() << std::endl;
  return 0;
}