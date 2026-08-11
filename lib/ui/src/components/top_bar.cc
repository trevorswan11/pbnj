#include "ui/components/top_bar.hh"

#include <gsl/util>
#include <imgui.h>
#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/assets/lucide.hh"
#include "ui/core/context.hh"

namespace pbnj::ui::components {

auto top_bar::on_mount(context& ctx) -> void {
    back_ = ctx.textures.get_or_load_svg("chevron_left", assets::chevron_left)
                .value_or(ImTextureID_Invalid);
    ctx.log.info("Created back button at texture: {}", back_);
    forward_ = ctx.textures.get_or_load_svg("chevron_right", assets::chevron_right)
                   .value_or(ImTextureID_Invalid);
    ctx.log.info("Created forward button at texture: {}", forward_);
    ctx.log.info("Mounted top bar");
}

auto top_bar::on_unmount(context& ctx) -> void { ctx.log.info("Unmounted top bar"); }

auto top_bar::render(context& ctx) -> void {
    PROFILE_FUNCTION();

    ImGui::PushStyleColor(ImGuiCol_Button, {0, 0, 0, 0});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {1, 1, 1, 0.08f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, {1, 1, 1, 0.16f});
    const auto color_cleanup = gsl::finally([] { ImGui::PopStyleColor(3); });
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {6, 6});
    const auto style_cleanup = gsl::finally([] { ImGui::PopStyleVar(2); });

    const auto can_go_back = ctx.router.can_go_back();
    if (!can_go_back) { ImGui::BeginDisabled(); }
    if (ImGui::ImageButton("#back", back_, {18, 18})) { ctx.router.go_back(ctx); }
    if (!can_go_back) { ImGui::EndDisabled(); }

    ImGui::SameLine();

    const auto can_go_fwd = ctx.router.can_go_back();
    if (!can_go_fwd) { ImGui::BeginDisabled(); }
    if (ImGui::ImageButton("#forward", forward_, {18, 18})) { ctx.router.go_forward(ctx); }
    if (!can_go_fwd) { ImGui::EndDisabled(); }
}

} // namespace pbnj::ui::components
