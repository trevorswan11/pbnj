#include "ui/theme/colors.hh"

#include <sokol.h>

namespace pbnj::ui::theme::colors {

auto as_sokol_color(const glm::vec4& vec) noexcept -> sg_color {
    return {vec.r, vec.g, vec.b, vec.w};
}

auto as_imgui_color(const glm::vec4& vec) noexcept -> ImVec4 {
    return {vec.r, vec.g, vec.b, vec.w};
}

} // namespace pbnj::ui::theme::colors
