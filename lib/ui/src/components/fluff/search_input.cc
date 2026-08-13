#include "ui/components/fluff/search_input.hh"

#include <gsl/util>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "ui/core/context.hh"
#include "ui/theme/colors.hh"

namespace pbnj::ui::components {

auto search_input::on_mount(context& ctx) -> void {
    ctx.log.info("Mounted search_input '{}'", tag);
}
auto search_input::on_unmount(context& ctx) -> void {
    ctx.log.info("Unmounted search_input '{}'", tag);
}

auto search_input::render(context& ctx) -> void {
    PROFILE_FUNCTION();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {text_pad_left, 6.0f});
    const auto style_cleanup = gsl::finally([] { ImGui::PopStyleVar(2); });
    ImGui::PushStyleColor(ImGuiCol_FrameBg, theme::colors::frame_bg);
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, theme::colors::frame_bg_hovered);
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, theme::colors::frame_bg_active);
    const auto color_cleanup = gsl::finally([] { ImGui::PopStyleColor(3); });

    ImGui::SetNextItemWidth(width);
    const bool changed = ImGui::InputTextWithHint(
        tag.c_str(), placeholder.c_str(), &ctx.search_input, ImGuiInputTextFlags_EnterReturnsTrue);
    if (changed && on_submit) {
        on_submit(ctx, ctx.search_input); // TODO: Debounce this
    } else if (ImGui::IsItemEdited() && on_change) {
        on_change(ctx, ctx.search_input);
    }

    // Draw the search icon inside the left padding area
    const auto min        = ImGui::GetItemRectMin();
    const auto max        = ImGui::GetItemRectMax();
    const auto icon_x     = min.x + icon_pad_x;
    const auto icon_y     = min.y + (max.y - min.y - icon_size) * 0.5f;
    const auto search_tex = ctx.textures.get_core_icon(icon_id);

    const ImColor icon_col =
        ImGui::IsItemActive() ? theme::colors::icon_active : theme::colors::icon_idle;
    ImGui::GetWindowDrawList()->AddImage(search_tex,
                                         {icon_x, icon_y},
                                         {icon_x + icon_size, icon_y + icon_size},
                                         {0, 0},
                                         {1, 1},
                                         icon_col);
}

} // namespace pbnj::ui::components
