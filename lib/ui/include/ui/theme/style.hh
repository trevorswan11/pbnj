#pragma once

#include <imgui.hh>
#include <stdx/types.hh>

struct GLFWwindow;

namespace pbnj::ui::theme {

enum class mode_t : u8 {
    DARK,
    LIGHT,
};

class style_manager {
  public:
    auto set_mode(GLFWwindow* window, mode_t mode) noexcept -> void;

    [[nodiscard]] auto mode() const noexcept -> mode_t { return mode_; }
    [[nodiscard]] auto is_dark() const noexcept -> bool { return mode_ == mode_t::DARK; }
    [[nodiscard]] auto is_light() const noexcept -> bool { return mode_ == mode_t::LIGHT; }

    [[nodiscard]] auto window_bg() const noexcept -> ImVec4;
    [[nodiscard]] auto clear_color() const noexcept -> ImVec4;
    [[nodiscard]] auto child_bg() const noexcept -> ImVec4;
    [[nodiscard]] auto popup_bg() const noexcept -> ImVec4;
    [[nodiscard]] auto border() const noexcept -> ImVec4;
    [[nodiscard]] auto text() const noexcept -> ImVec4;
    [[nodiscard]] auto text_disabled() const noexcept -> ImVec4;

    [[nodiscard]] auto button() const noexcept -> ImVec4;
    [[nodiscard]] auto button_hovered() const noexcept -> ImVec4;
    [[nodiscard]] auto button_active() const noexcept -> ImVec4;

    [[nodiscard]] auto ghost_hover() const noexcept -> ImVec4;
    [[nodiscard]] auto ghost_active() const noexcept -> ImVec4;

    [[nodiscard]] auto frame_bg() const noexcept -> ImVec4;
    [[nodiscard]] auto frame_bg_hovered() const noexcept -> ImVec4;
    [[nodiscard]] auto frame_bg_active() const noexcept -> ImVec4;

    [[nodiscard]] auto icon_active() const noexcept -> ImVec4;
    [[nodiscard]] auto icon_idle() const noexcept -> ImVec4;
    [[nodiscard]] auto icon_tint() const noexcept -> ImVec4;

    [[nodiscard]] auto slider_grab() const noexcept -> ImVec4;
    [[nodiscard]] auto slider_grab_active() const noexcept -> ImVec4;

    [[nodiscard]] auto transparent() const noexcept -> ImVec4;
    [[nodiscard]] auto black() const noexcept -> ImVec4;
    [[nodiscard]] auto white() const noexcept -> ImVec4;

  private:
    mode_t mode_{mode_t::DARK};
};

} // namespace pbnj::ui::theme
