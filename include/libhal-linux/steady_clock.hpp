#pragma once
#include <chrono>
#include <concepts>
#include <cstdint>
#include <cstdio>
#include <libhal/steady_clock.hpp>
#include <libhal/units.hpp>
#include <ratio>
#include <type_traits>

namespace {
template<typename T>
concept Clock = requires
{
  {
    T::now()
    } -> std::convertible_to<std::chrono::time_point<T>>;

  std::is_integral_v<typename T::rep>;

  std::is_same_v<typename T::period,
                 std::ratio<T::period::num, T::period::den>>;
  std::is_same_v<typename T::duration,
                 std::chrono::duration<typename T::rep, typename T::period>>;
  std::is_same_v<typename T::time_point, std::chrono::time_point<T>>;
  {
    T::is_steady
    } -> std::convertible_to<const bool>;
};

}  // namespace

namespace hal::linux {

template<Clock C>
class steady_clock : public hal::steady_clock
{
public:
  steady_clock()
  {
    printf("Constructed\n");
    std::chrono::time_point<C> res = C::now();
  }
  constexpr hertz driver_frequency() override
  {
    using period = C::period;
    double freq = period::den / period::num;
    return static_cast<hertz>(freq);
  }

  std::uint64_t driver_uptime() override
  {
    return static_cast<std::uint64_t>(C::now().time_since_epoch().count());
  }
};

}  // namespace hal::linux