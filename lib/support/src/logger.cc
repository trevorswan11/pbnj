#include "support/logger.hh"

#include <atomic>
#include <vector>

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <stdx/assert.hh>
#include <stdx/memory.hh>

namespace pbnj {

namespace { constinit std::atomic<bool> initialized{false}; } // namespace

logger::logger() {
    ASSERT(!initialized, "Logger can only be initialized once");
    std::vector<spdlog::sink_ptr> log_sinks;

    log_sinks.emplace_back(stdx::make_rc<spdlog::sinks::stdout_color_sink_mt>());
    log_sinks.back()->set_pattern("%^[%T] %n: %v%$");

    log_sinks.emplace_back(stdx::make_rc<spdlog::sinks::basic_file_sink_mt>("PBnJ.log", true));
    log_sinks.back()->set_pattern("[%T] [%l] %n: %v");

    logger_ = stdx::make_rc<spdlog::logger>("PBnJ", log_sinks.begin(), log_sinks.end());
    initialized.store(true);
}

} // namespace pbnj
