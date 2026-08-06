#pragma once

#include <stdx/types.hh>
#include <stdx/utility.hh>

namespace pbnj::ui {

class frame {
  public:
    frame() noexcept;
    ~frame();
    MAKE_PINNED(frame);

  private:
    i32 width_;
    i32 height_;
    f64 dt_;
    f32 dpi_scale_;
};

} // namespace pbnj::ui
