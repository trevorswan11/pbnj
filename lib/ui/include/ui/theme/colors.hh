#pragma once

#include <sokol.h>

namespace pbnj::ui::theme::colors {

constexpr ImVec4 dark_grey{0.07f, 0.07f, 0.08f, 1};
constexpr ImVec4 light_dark_grey{0.11f, 0.11f, 0.13f, 1};
constexpr ImVec4 grey{0.14f, 0.14f, 0.16f, 1};
constexpr ImVec4 dark_light_grey{0.18f, 0.18f, 0.22f, 1};
constexpr ImVec4 light_grey{0.24f, 0.24f, 0.28f, 1};
constexpr ImVec4 dark_light_green{0.11f, 0.73f, 0.33f, 1};
constexpr ImVec4 light_green{0.15f, 0.85f, 0.38f, 1};

[[nodiscard]] constexpr auto as_sokol_color(ImVec4 vec) noexcept -> sg_color {
    return {vec.x, vec.y, vec.z, vec.w};
}

[[nodiscard]] constexpr auto as_imgui_color(sg_color col) noexcept -> ImVec4 {
    return {col.r, col.g, col.b, col.a};
}

} // namespace pbnj::ui::theme::colors
