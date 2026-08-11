#include "ui/components/root.hh"

#include <sokol.h>
#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/core/context.hh"

namespace pbnj::ui::components {

auto root::on_mount(context& ctx) -> void {
    top_bar_.on_mount(ctx);
    side_bar_.on_mount(ctx);
    player_bar_.on_mount(ctx);
    ctx.log.info("Mounted root component");
}

auto root::on_unmount(context& ctx) -> void {
    player_bar_.on_unmount(ctx);
    side_bar_.on_unmount(ctx);
    top_bar_.on_unmount(ctx);
    ctx.log.info("Unmounted root component");
}

auto root::render(context& ctx) -> void {
    PROFILE_FUNCTION();
    const auto dpi    = sapp_dpi_scale();
    const auto width  = sapp_widthf() / dpi;
    const auto height = sapp_heightf() / dpi;

    const auto top_h     = 48.0f;
    const auto player_h  = 88.0f;
    const auto side_w    = 220.0f;
    const auto content_w = width - side_w;
    const auto content_h = height - top_h - player_h;

    constexpr auto panel_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    // Top navbar
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({width, top_h});
    if (ImGui::Begin("##top_bar", nullptr, panel_flags | ImGuiWindowFlags_NoScrollbar)) {
        top_bar_.render(ctx);
    }
    ImGui::End();

    // Left side bar
    ImGui::SetNextWindowPos({0, top_h});
    ImGui::SetNextWindowSize({side_w, content_h});
    if (ImGui::Begin("##side_bar", nullptr, panel_flags)) { side_bar_.render(ctx); }
    ImGui::End();

    // Central page
    ImGui::SetNextWindowPos({side_w, top_h});
    ImGui::SetNextWindowSize({content_w, content_h});
    if (ImGui::Begin("##viewport", nullptr, panel_flags)) { ctx.router.render_current(ctx); }
    ImGui::End();

    // Bottom player bar
    ImGui::SetNextWindowPos({0, top_h + content_h});
    ImGui::SetNextWindowSize({width, player_h});
    if (ImGui::Begin("##player_bar", nullptr, panel_flags | ImGuiWindowFlags_NoScrollbar)) {
        player_bar_.render(ctx);
    }
    ImGui::End();
}

} // namespace pbnj::ui::components
