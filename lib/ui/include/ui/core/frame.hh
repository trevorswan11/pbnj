#pragma once

#include <imgui.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

namespace pbnj::ui {

class vk_context;

class frame {
  public:
    explicit frame(vk_context& vk_ctx, GLFWwindow* window, f64 dt, ImVec4 clear_color) noexcept;
    ~frame();
    MAKE_PINNED(frame);

    MAKE_GETTER(dt, f64)
    [[nodiscard]] auto is_active() const noexcept -> bool { return is_active_; }

  private:
    vk_context& vk_ctx_;
    GLFWwindow* window_;
    f64         dt_;
    ImVec4      clear_color_;
    bool        is_active_{false};
};

} // namespace pbnj::ui
