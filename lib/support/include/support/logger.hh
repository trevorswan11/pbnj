#pragma once

#include <fmt/base.h>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <stdx/memory.hh>
#include <stdx/types.hh>

namespace pbnj {

class logger {
  public:
    logger();

    template <typename... Args>
    auto trace(fmt::format_string<Args...> fmt, Args&&... args) -> void {
        logger_->log(spdlog::level::trace, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> auto info(fmt::format_string<Args...> fmt, Args&&... args) -> void {
        logger_->log(spdlog::level::info, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args> auto warn(fmt::format_string<Args...> fmt, Args&&... args) -> void {
        logger_->log(spdlog::level::warn, fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto error(fmt::format_string<Args...> fmt, Args&&... args) -> void {
        logger_->log(spdlog::level::err, fmt, std::forward<Args>(args)...);
    }

  private:
    stdx::rc<spdlog::logger> logger_;
};

} // namespace pbnj
