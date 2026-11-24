module;

// Linux stuff
#include <alsa/asoundlib.h>

export module audio_device;

// STL STUFF
import std;

// Our stuff
import audio_processor;

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

namespace alsa_min {
  static constexpr std::uint32_t     default_channel_count{2};
  static constexpr std::uint32_t     default_sample_rate{48'000};
  static constexpr snd_pcm_uframes_t default_period_size{1024};
  static constexpr snd_pcm_uframes_t default_buffer_size{4096};
  static constexpr int               wait_timeout_ms{1000};
  static constexpr double            pcm_32_scale{2'147'483'648.0}; // 2^31

  /**
   * @struct audio_stats: Optional stats used for monitoring the audio device state
   * @param wakeups: How many times alsa has woken the audio thread
   * @param frames:  How many frames have been through the processor
   * @param xruns:   How many times either stream under/overran and both had to be restarted
   */
  export struct audio_stats {
    std::atomic<std::uint64_t> wakeups;
    std::atomic<std::uint64_t> frames;
    std::atomic<std::uint64_t> xruns;
  };

  /**
   * @struct audio_device_config:
   *
   * struct for keeping device parameters neat and tidy, best to use designated initialisers
   * @param channel_count: Number of channels. TODO: Split this into input & output channels
   * @param sample_rate:   Number of samples per second
   * @param period_size:   Number of frames that alsa hands over per wakeup
   * @param buffer_size:   Size of each ring buffer.
   */
  export struct audio_device_config {
    std::uint32_t     channel_count{default_channel_count};
    std::uint32_t     sample_rate{default_sample_rate};
    snd_pcm_uframes_t period_size{default_period_size};
    snd_pcm_uframes_t buffer_size{default_buffer_size};

    auto operator==(const audio_device_config&) const -> bool = default;
  };

  /**
   * @struct unique_ptr deleter allowing us to use RAII for the pcm handle & hardware/software parameters
   */
  template<auto Free>
  struct c_deleter {
    void operator()(auto* p) const noexcept
      requires(std::invocable<decltype(Free), decltype(p)>)
    {
      Free(p);
    }
  };

  ///> @typedef unique_ptr with custom deleter for pcm handle
  export using pcm_handle = std::unique_ptr<snd_pcm_t, c_deleter<snd_pcm_close>>;
  ///> @typedef unique_ptr with custom deleter for hardware parameters
  export using hw_params_handle = std::unique_ptr<snd_pcm_hw_params_t, c_deleter<snd_pcm_hw_params_free>>;
  ///> @typedef unique_ptr with custom deleter for software parameters
  export using sw_params_handle = std::unique_ptr<snd_pcm_sw_params_t, c_deleter<snd_pcm_sw_params_free>>;

  /**
   * @struct alsa_error:
   */
  export struct alsa_error {
    int         err_num{};
    std::string err_msg;
  };

  ///> @typedef helper alias for functions that use expected for error handling, which is most of them.
  export using alsa_result = std::expected<void, alsa_error>;

  /**
   * @fn make_alsa_error:
   * @brief Helper function for dealing with return values from functions with c-style error handling i.e. returning an error code
   * @param result: Result will always be an error value from alsa
   * @param message: Usually a descriptive message of what we were trying to accomplish
   */
  [[nodiscard]] inline auto make_alsa_error(int result, std::string_view message) NONBLOCKING -> std::unexpected<alsa_error> {
    return std::unexpected(alsa_error{.err_num = result, .err_msg = std::format("{}: {}", message, snd_strerror(result))});
  }

  template<class Func, class... Args>
  concept alsa_call = std::invocable<Func, Args...> && std::same_as<std::invoke_result_t<Func, Args...>, int>;

  /**
   * @class audio_device
   * @brief Always create this class using the open() factory function,
   * if the device fails to be open you'll receive an unexpected instead of an unusable object
   *
   */
  export class audio_device {
    audio_device_config       m_config;
    pcm_handle                m_capture;
    pcm_handle                m_playback;
    bool                      m_linked{};
    std::vector<float>        m_in;
    std::vector<float>        m_out;
    std::vector<std::int32_t> m_in_raw;
    std::vector<std::int32_t> m_out_raw;

    template<class Func, class... Args>
    [[nodiscard]] static auto try_to(std::string_view message, Func func, Args... args) noexcept NONBLOCKING
      requires(alsa_call<Func, Args...>)
    {
      return [=] -> alsa_result {
        if (const int result = func(args...); result < 0) {
          return make_alsa_error(result, message);
        }
        return {};
      };
    }

    [[nodiscard]] static auto configure(snd_pcm_t* pcm, audio_device_config& config) NONALLOCATING -> alsa_result {
      hw_params_handle hw_params{};
      sw_params_handle sw_params{};

      const auto hw_step = [&](std::string_view message, auto func, auto... args) {
        return try_to(message, func, pcm, hw_params.get(), args...);
      };
      const auto sw_step = [&](std::string_view message, auto func, auto... args) {
        return try_to(message, func, pcm, sw_params.get(), args...);
      };

      if (const int err = snd_pcm_hw_params_malloc(std::out_ptr(hw_params)); err < 0) {
        return make_alsa_error(err, "allocate hardware parameter structure");
      }

      const auto hw_result = alsa_result{}
                               .and_then(hw_step("initialize hardware parameter structure", snd_pcm_hw_params_any))
                               .and_then(hw_step("set access type", snd_pcm_hw_params_set_access, SND_PCM_ACCESS_RW_INTERLEAVED))
                               .and_then(hw_step("set sample format", snd_pcm_hw_params_set_format, SND_PCM_FORMAT_S32_LE))
                               .and_then(hw_step("set channel count", snd_pcm_hw_params_set_channels, config.channel_count))
                               .and_then(hw_step("set sample rate", snd_pcm_hw_params_set_rate_near, &config.sample_rate, nullptr))
                               .and_then(hw_step("set period size", snd_pcm_hw_params_set_period_size_near, &config.period_size, nullptr))
                               .and_then(hw_step("set buffer size", snd_pcm_hw_params_set_buffer_size_near, &config.buffer_size))
                               .and_then(hw_step("set parameters", snd_pcm_hw_params));

      if (!hw_result.has_value()) {
        return hw_result;
      }

      if (const int err = snd_pcm_sw_params_malloc(std::out_ptr(sw_params)); err < 0) {
        return make_alsa_error(err, "allocate software parameters structure");
      }

      if (auto r = sw_step("initialize software parameters structure", snd_pcm_sw_params_current)(); !r.has_value()) {
        return r;
      }

      snd_pcm_uframes_t boundary{};
      if (const int err = snd_pcm_sw_params_get_boundary(sw_params.get(), &boundary); err < 0) {
        return make_alsa_error(err, "get boundary");
      }

      return alsa_result{}
        .and_then(sw_step("set minimum available count", snd_pcm_sw_params_set_avail_min, config.period_size))
        .and_then(sw_step("disable auto-start", snd_pcm_sw_params_set_start_threshold, boundary))
        .and_then(sw_step("set software parameters", snd_pcm_sw_params));
    }

    [[nodiscard]] auto read_frames(std::span<std::int32_t> dst) noexcept NONALLOCATING -> snd_pcm_sframes_t {
      while (!dst.empty()) {
        const auto received = snd_pcm_readi(
          m_capture.get(), dst.data(), static_cast<snd_pcm_uframes_t>(dst.size() / m_config.channel_count)
        ); // TODO: change to input channels
        if (received < 0) {
          return received;
        }
        dst = dst.subspan(static_cast<std::size_t>(received) * m_config.channel_count);
      }
      return 0;
    }
    [[nodiscard]] auto write_frames(std::span<const std::int32_t> src) noexcept NONALLOCATING -> snd_pcm_sframes_t {
      while (!src.empty()) {
        const auto sent = snd_pcm_writei(
          m_playback.get(), src.data(), static_cast<snd_pcm_uframes_t>(src.size() / m_config.channel_count)
        ); // TODO: change to output channels
        if (sent < 0) {
          return sent;
        }
        src = src.subspan(static_cast<std::size_t>(sent) * m_config.channel_count);
      }
      return 0;
    }

    [[nodiscard]] auto start_streams() NONALLOCATING -> alsa_result {
      std::ranges::fill(m_out_raw, 0);

      for (snd_pcm_uframes_t filled{0}; filled < m_config.buffer_size;) {
        const auto chunk = std::min(m_config.period_size, m_config.buffer_size - filled);
        if (const auto err = write_frames(std::span{m_out_raw}.first(chunk * m_config.channel_count)); err < 0) {
          return make_alsa_error(static_cast<int>(err), "prefill playback buffer");
        }
        filled += chunk;
      }

      if (const int err = snd_pcm_start(m_capture.get()); err < 0) {
        return make_alsa_error(err, "start capture");
      }
      if (!m_linked) {
        if (const int err = snd_pcm_start(m_playback.get()); err < 0) {
          return make_alsa_error(err, "start playback");
        }
      }
      return {};
    }

    [[nodiscard]] auto reset_streams() NONALLOCATING -> alsa_result {
      static_cast<void>(snd_pcm_drop(m_capture.get()));
      static_cast<void>(snd_pcm_drop(m_playback.get()));
      return alsa_result{}
        .and_then(try_to("prepare capture", snd_pcm_prepare, m_capture.get()))
        .and_then(try_to("prepare playback", snd_pcm_prepare, m_playback.get()));
    }

    [[nodiscard]] auto handle_error(snd_pcm_sframes_t code, std::string_view what, audio_stats& stats) NONBLOCKING -> alsa_result {
      if (code == -EINTR) {
        return {};
      }
      if (code == -EPIPE || code == -ESTRPIPE) {
        stats.xruns.fetch_add(1, std::memory_order_relaxed);
        return reset_streams().and_then([this] { return start_streams(); });
      }
      return make_alsa_error(static_cast<int>(code), what);
    }

    static auto to_pcm_32(const float& sample) noexcept NONBLOCKING -> std::int32_t {
      return static_cast<std::int32_t>(std::clamp(sample, -1.0F, 1.0F) * static_cast<double>(std::numeric_limits<std::int32_t>::max()));
    }

    static auto from_pcm_32(const std::int32_t& sample) noexcept NONBLOCKING -> float { return static_cast<float>(sample / pcm_32_scale); }

    [[nodiscard]] audio_device(pcm_handle capture, pcm_handle playback, const audio_device_config& negotiated, bool linked) :
        m_config(negotiated), m_capture(std::move(capture)), m_playback(std::move(playback)), m_linked(linked) {
      const auto size{m_config.channel_count * m_config.period_size};
      m_in_raw.resize(size);
      m_in.resize(size);
      m_out.resize(size);
      m_out_raw.resize(size);
    }

public:
    [[nodiscard]] static auto open(const std::string& device_name, audio_device_config& requested_config)
      -> std::expected<audio_device, alsa_error> {
      pcm_handle capture;
      pcm_handle playback;

      if (const int err = snd_pcm_open(std::out_ptr(capture), device_name.c_str(), SND_PCM_STREAM_CAPTURE, 0); err < 0) {
        return make_alsa_error(err, "open capture device");
      }
      if (const int err = snd_pcm_open(std::out_ptr(playback), device_name.c_str(), SND_PCM_STREAM_PLAYBACK, 0); err < 0) {
        return make_alsa_error(err, "open playback device");
      }

      if (auto r = configure(capture.get(), requested_config); !r.has_value()) {
        return std::unexpected(r.error());
      }
      auto playback_config{requested_config};
      if (auto r = configure(playback.get(), playback_config); !r.has_value()) {
        return std::unexpected(r.error());
      }
      if (playback_config != requested_config) {
        return make_alsa_error(-EINVAL, "negotiated playback and capture config don't match");
      }

      const int link_err = snd_pcm_link(capture.get(), playback.get());
      if (link_err < 0) {
        std::println(stderr, "link failed: {}", snd_strerror(link_err));
      }
      const bool linked = link_err == 0;

      return audio_device(std::move(capture), std::move(playback), requested_config, linked);
    }

    [[nodiscard]] auto is_linked() const noexcept -> bool { return m_linked; }

    /**
     * @fn run()
     * @brief loops infinitely until the thread tells it to stop
     * @param audio_processor: class which can be inherited from to implement custom effects.
     * @param stats: object outside of this class which is updated atomically
     * @return std::expected<void> on success, std::unexpected<alsa_error> on failure
     */
    [[nodiscard]] auto run(const std::stop_token& stop, audio_processor& proc, audio_stats& stats) NONALLOCATING -> alsa_result { // NOLINT
      proc.prepare({.sample_rate = m_config.sample_rate, .channels = m_config.channel_count, .max_frames = m_config.period_size});

      if (auto r = start_streams(); !r.has_value()) {
        return r;
      }

      while (!stop.stop_requested()) {
        const int ready = snd_pcm_wait(m_capture.get(), wait_timeout_ms);
        if (ready == 0) {
          return make_alsa_error(-EIO, "device stopped responding");
        }

        if (ready < 0) {
          if (auto r = handle_error(ready, "wait for capture", stats); !r.has_value()) {
            return r;
          }
          continue;
        }

        stats.wakeups.fetch_add(1, std::memory_order_relaxed);

        const auto avail = snd_pcm_avail_update(m_capture.get());
        if (avail < 0) {
          if (auto r = handle_error(avail, "check captured frames", stats); !r.has_value()) {
            return r;
          }
          continue;
        }
        if (std::cmp_less(avail, m_config.period_size)) {
          continue;
        }

        if (const auto err = read_frames(m_in_raw); err < 0) {
          if (auto r = handle_error(err, "read from capture", stats); !r.has_value()) {
            return r;
          }
          continue;
        }
        std::ranges::transform(m_in_raw, m_in.begin(), from_pcm_32);

        proc.process(m_in, m_out);
        std::ranges::transform(m_out, m_out_raw.begin(), to_pcm_32);

        if (const auto err = write_frames(m_out_raw); err < 0) {
          if (auto r = handle_error(err, "write to playback", stats); !r.has_value()) {
            return r;
          }
          continue;
        }
        stats.frames.fetch_add(m_config.period_size, std::memory_order_relaxed);
      }
      return reset_streams();
    }

    /**
     * @fn run()
     * @brief override of run() where we throw away the stats as I think they should be optional.
     * @param audio_processor: class which can be inherited from to implement custom effects.
     * I'll need to have a think about how the chaining of effects will work.
     * @return std::expected<void> on success, std::unexpected<alsa_error> on failure
     */
    auto run(const std::stop_token& stop, audio_processor& proc) NONALLOCATING -> alsa_result {
      audio_stats stats; // Don't (always) want this
      return run(stop, proc, stats);
    }
  };
} // namespace alsa_min
