// third party stuff
#include <catch2/catch_approx.hpp>
#include <catch2/catch_message.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

// STL stuff
import std;

// our stuff
import audio_processor;
import audio_device;

#if defined(__has_feature) && __has_feature(realtime_sanitizer)
#define NONBLOCKING [[clang::nonblocking]]
#define BLOCKING [[clang::blocking]]
#define NONALLOCATING [[clang::nonallocating]]
#define ALLOCATING [[clang::allocating]]
#elif
#define NONBLOCKING
#define BLOCKING
#define NONALLOCATING
#define ALLOCATING
#endif

// NOLINTBEGIN
#define CHECK_RESULT(var_expr)                                                                                                             \
  do {                                                                                                                                     \
    const auto& _result = (var_expr);                                                                                                      \
    if (!_result.has_value()) {                                                                                                            \
      UNSCOPED_INFO(std::format("{}", _result.error().err_msg));                                                                           \
    }                                                                                                                                      \
    REQUIRE(_result.has_value());                                                                                                          \
  } while (0)
// NOLINTEND

static constexpr double default_sine_frequency{440.0};

TEST_CASE("passthrough copies input to output") {
  alsa_min::passthrough proc;
  proc.prepare({.sample_rate = 48'000, .channels = 2, .max_frames = 4}); // NOLINT

  std::vector<float> in{0.1F, -0.2F, 0.3F, -0.4F, 0.5F, -0.6F, 0.7F, -0.8F}; // NOLINT
  std::vector<float> out(in.size());
  proc.process(in, out);

  REQUIRE(out == in);
}

TEST_CASE("duplex passthrough without xruns") {
  using namespace std::chrono_literals;
  constexpr auto duration = 4s;

  const char*       env  = std::getenv("ALSA_MIN_DEVICE"); // NOLINT
  const std::string name = env != nullptr ? env : "default";

  alsa_min::audio_device_config config{}; // NOLINT
  auto                          device = alsa_min::audio_device::open(name, config);
  CHECK_RESULT(device);

  // open() writes the negotiated values back into config
  std::println(
    "rate {}  period {}  buffer {} linked {} name {}", config.sample_rate, config.period_size, config.buffer_size, device->is_linked(), name
  );

  alsa_min::passthrough proc;
  alsa_min::audio_stats stats;
  alsa_min::alsa_result result{};

  {
    std::jthread audio{[&](const std::stop_token& stop) { result = device->run(stop, proc, stats); }};
    std::this_thread::sleep_for(duration);
  }

  CHECK_RESULT(result);
  CHECK(stats.xruns.load() == 0);

  const auto expected = config.sample_rate * std::chrono::duration<double>(duration).count();
  CHECK(static_cast<double>(stats.frames.load()) == Catch::Approx(expected).epsilon(0.1)); // NOLINT
}

TEST_CASE("sine wave plays without xruns") {
  using namespace std::chrono_literals;
  constexpr auto duration = 4s;

  const char*       env  = std::getenv("ALSA_MIN_DEVICE"); // NOLINT
  const std::string name = env != nullptr ? env : "default";

  alsa_min::audio_device_config config{}; // NOLINT
  auto                          device = alsa_min::audio_device::open(name, config);
  CHECK_RESULT(device);

  // open() writes the negotiated values back into config
  std::println("rate {}  period {}  buffer {}", config.sample_rate, config.period_size, config.buffer_size);

  alsa_min::sine        proc{default_sine_frequency};
  alsa_min::audio_stats stats;
  alsa_min::alsa_result result{};

  {
    std::jthread audio{[&](const std::stop_token& stop) { result = device->run(stop, proc, stats); }};
    std::this_thread::sleep_for(duration);
  }

  CHECK_RESULT(result);
  CHECK(stats.xruns.load() == 0);

  const auto expected = config.sample_rate * std::chrono::duration<double>(duration).count();
  CHECK(static_cast<double>(stats.frames.load()) == Catch::Approx(expected).epsilon(0.1)); // NOLINT
}

// NOLINTBEGIN
TEST_CASE("README") {
  using namespace std::chrono_literals;
  alsa_min::audio_device_config config{};
  alsa_min::passthrough         proc; // audio processor that passes the input to the output
  auto                          device = alsa_min::audio_device::open("default", config); // factory that return std::unexpected on failure
  if (device) {
    std::jthread audio{[&](const std::stop_token& stop) { const auto result = device->run(stop, proc); }};
    std::this_thread::sleep_for(4s);
  } else {
    std::println("{}", device.error().err_msg);
  }

  // thus endeth the example
  REQUIRE(device.has_value());
}
// NOLINTEND