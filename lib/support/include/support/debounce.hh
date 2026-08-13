#pragma once

#include <chrono>
#include <utility>

#include <stdx/function.hh>

namespace pbnj {

class debounce {
  public:
    debounce(stdx::function<void()> callback, std::chrono::milliseconds interval) noexcept
        : interval_{interval}, callback_{std::move(callback)} {}

    explicit debounce(std::chrono::milliseconds interval) noexcept
        : interval_{interval}, callback_{nullptr} {}

    auto set_callback(stdx::function<void()> callback) noexcept -> void {
        callback_ = std::move(callback);
    }

    // Attempts to call the callback, returning true if the interval was passed
    auto operator()() -> bool {
        if (!callback_) { return false; }
        auto now = std::chrono::high_resolution_clock::now();
        if (last_call_time_ == std::chrono::high_resolution_clock::time_point{} ||
            now - last_call_time_ >= interval_) {
            callback_();
            last_call_time_ = now;
            return true;
        }
        return false;
    }

  private:
    std::chrono::milliseconds                      interval_;
    std::chrono::high_resolution_clock::time_point last_call_time_{};
    stdx::function<void()>                         callback_;
};

} // namespace pbnj
