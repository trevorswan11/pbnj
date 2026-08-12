#pragma once

#include <imgui.h>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "ui/theme/colors.hh"

namespace pbnj::ui {

class frame {
  public:
    explicit frame(ImVec4 clear_color = theme::colors::dark_grey) noexcept;
    ~frame();
    MAKE_PINNED(frame);

    MAKE_GETTER(dt, f64)

  private:
    i32    width_;
    i32    height_;
    f64    dt_;
    f32    dpi_scale_;
    ImVec4 clear_color_;
};

} // namespace pbnj::ui
