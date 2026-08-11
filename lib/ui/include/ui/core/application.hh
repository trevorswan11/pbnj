#pragma once

#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "ui/components/root.hh"
#include "ui/core/context.hh"

struct sapp_event;

namespace pbnj::ui {

class application {
  public:
    application() noexcept = default;
    ~application()         = default;
    MAKE_PINNED(application);

    auto launch() noexcept -> void;

  private:
    auto on_init() noexcept -> void;
    auto on_frame() noexcept -> void;
    auto on_event(const sapp_event* event) noexcept -> void;
    auto on_cleanup() noexcept -> void;

  private:
    context          ctx_;
    components::root root_;
};

} // namespace pbnj::ui
