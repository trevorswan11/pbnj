#include "ui/components/fluff/icon_button.hh"

#include <gsl/util>
#include <imgui.hh>
#include <stdx/profiler.hh>

#include "ui/core/context.hh"
#include "ui/theme/colors.hh"

namespace pbnj::ui::components {

auto icon_button::on_mount(context& ctx) -> void { ctx.log.info("Mounted icon_button '{}'", tag); }

auto icon_button::on_unmount(context& ctx) -> void {
    ctx.log.info("Unmounted icon_button '{}'", tag);
}

auto icon_button::render(context& ctx) -> void {
    PROFILE_FUNCTION();
    const auto texture = ctx.textures.get_core_icon(icon_id);

    ImGui::PushStyleColor(ImGuiCol_Button, theme::colors::transparent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, theme::colors::ghost_hover);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, theme::colors::ghost_active);
    const auto color_cleanup = gsl::finally([] { ImGui::PopStyleColor(3); });

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {padding, padding});
    const auto style_cleanup = gsl::finally([] { ImGui::PopStyleVar(2); });

    const bool disabled = is_disabled && is_disabled(ctx);
    ImVec4     tint{theme::colors::white};
    if (disabled) { tint.w = disabled_tint; }

    if (disabled) { ImGui::BeginDisabled(); }
    if (ImGui::ImageButton(
            tag.c_str(), texture, size, {0, 0}, {1, 1}, theme::colors::transparent, tint)) {
        if (on_click) { on_click(ctx); }
    }
    if (disabled) { ImGui::EndDisabled(); }
}

} // namespace pbnj::ui::components
