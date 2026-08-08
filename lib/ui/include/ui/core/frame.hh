#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "ui/theme/colors.hh"

namespace pbnj::ui {

class frame {
  public:
    explicit frame(glm::vec4 clear_color = theme::colors::dark_grey) noexcept;
    ~frame();
    MAKE_PINNED(frame);

  private:
    i32       width_;
    i32       height_;
    f64       dt_;
    f32       dpi_scale_;
    glm::vec4 clear_color_;
};

} // namespace pbnj::ui
