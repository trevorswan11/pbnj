#pragma once

#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "ui/theme/fonts.hh"
#include "ui/theme/style.hh"

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
    f32  slider_;
    bool show_test_window_{true};
    bool show_another_window_{false};

    theme::style_manager styles_;
    theme::font_manager  fonts_;
};

} // namespace pbnj::ui
