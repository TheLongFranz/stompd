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