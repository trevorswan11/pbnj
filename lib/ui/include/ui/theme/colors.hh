#pragma once

#include <SDL3/SDL_pixels.h>
#include <imgui.hh>
#include <stdx/types.hh>

namespace pbnj::ui::theme::colors {

constexpr ImVec4 transparent{0.0f, 0.0f, 0.0f, 0.0f};
constexpr ImVec4 black{0.0f, 0.0f, 0.0f, 1.0f};
constexpr ImVec4 white{1.0f, 1.0f, 1.0f, 1.0f};

constexpr ImVec4 dark_grey{0.07f, 0.07f, 0.08f, 1.0f};
constexpr ImVec4 light_dark_grey{0.11f, 0.11f, 0.13f, 1.0f};
constexpr ImVec4 grey{0.14f, 0.14f, 0.16f, 1.0f};
constexpr ImVec4 dark_light_grey{0.18f, 0.18f, 0.22f, 1.0f};
constexpr ImVec4 light_grey{0.24f, 0.24f, 0.28f, 1.0f};

constexpr ImVec4 dark_light_green{0.11f, 0.73f, 0.33f, 1.0f};
constexpr ImVec4 light_green{0.15f, 0.85f, 0.38f, 1.0f};

constexpr ImVec4 ghost_hover{1.0f, 1.0f, 1.0f, 0.08f};
constexpr ImVec4 ghost_active{1.0f, 1.0f, 1.0f, 0.16f};

constexpr ImVec4 frame_bg{0.14f, 0.14f, 0.14f, 1.0f};
constexpr ImVec4 frame_bg_hovered{0.18f, 0.18f, 0.18f, 1.0f};
constexpr ImVec4 frame_bg_active{0.22f, 0.22f, 0.22f, 1.0f};

constexpr ImVec4 icon_active{1.0f, 1.0f, 1.0f, 1.0f};
constexpr ImVec4 icon_idle{0.71f, 0.71f, 0.71f, 0.78f};

[[nodiscard]] constexpr auto as_sdl_color(ImVec4 vec) noexcept -> SDL_FColor {
    return {vec.x, vec.y, vec.z, vec.w};
}

[[nodiscard]] constexpr auto as_imgui_color(SDL_FColor col) noexcept -> ImVec4 {
    return {col.r, col.g, col.b, col.a};
}

} // namespace pbnj::ui::theme::colors
