#pragma once

#include <stdx/memory.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

namespace pbnj::ui {

class application {
  public:
    application();
    ~application();
    MAKE_PINNED(application);

    auto launch() noexcept -> void;

  private:
    class impl;

  private:
    stdx::box<impl> impl_;
};

} // namespace pbnj::ui
