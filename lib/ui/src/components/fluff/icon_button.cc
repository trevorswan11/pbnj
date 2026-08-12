#include "ui/components/fluff/icon_button.hh"

#include <gsl/util>
#include <imgui.h>
#include <stdx/profiler.hh>

#include "ui/core/context.hh"

namespace pbnj::ui::components {

auto icon_button::on_mount(context& ctx) -> void {
    texture_ = ctx.textures.get_core_icon(icon_id);
    ctx.log.info("Mounted icon_button '{}' at texture: {}", tag, texture_);
}

auto icon_button::on_unmount(context& ctx) -> void {
    ctx.log.info("Unmounted icon_button '{}'", tag);
}

auto icon_button::render(context& ctx) -> void {
    PROFILE_FUNCTION();

    ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {1, 1, 1, 0.08f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {1, 1, 1, 0.16f});
    const auto color_cleanup = gsl::finally([] { ImGui::PopStyleColor(3); });

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {padding, padding});
    const auto style_cleanup = gsl::finally([] { ImGui::PopStyleVar(2); });

    const bool   disabled = is_disabled && is_disabled(ctx);
    const ImVec4 tint{1, 1, 1, disabled ? disabled_tint : 1};
    if (disabled) { ImGui::BeginDisabled(); }
    if (ImGui::ImageButton(tag.c_str(), texture_, size, {0, 0}, {1, 1}, {0, 0, 0, 0}, tint)) {
        if (on_click) { on_click(ctx); }
    }
    if (disabled) { ImGui::EndDisabled(); }
}

} // namespace pbnj::ui::components
