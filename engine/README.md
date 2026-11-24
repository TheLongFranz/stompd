# alsa_min

## Introduction

Bare bones example of getting an alsa audio device working before committing over in stompd. So if you see a large number of files being dropped in in one commit it was from here.

## Prerequisites

There's probably no need for you to use this repo but if you really want to then you need the following...

- GCC 16 (For the standard library)
- LLVM 23 (For clang and all the lovely tools (I do mean ALL of them))
- Cmake 4.5.0 (For 'import std' & module support)
- Ninja (For obvious reason)

## Minimal example

```cpp
import std;

import audio_processor;
import audio_device;

int main() {
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
}
```