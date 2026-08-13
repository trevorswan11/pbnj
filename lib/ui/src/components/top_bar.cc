#include "ui/components/top_bar.hh"

#include <gsl/util>
#include <imgui.h>
#include <stdx/assert.hh>
#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/assets/texture_cache.hh"
#include "ui/core/context.hh"
#include "ui/pages/home.hh"

namespace pbnj::ui::components {

top_bar::top_bar()
    : menu_{{
          .tag      = "##menu",
          .icon_id  = assets::core_icon_id_t::MENU,
          .on_click = [](context&) {},
      }},
      back_{{
          .tag         = "##back",
          .icon_id     = assets::core_icon_id_t::CHEVRON_LEFT,
          .is_disabled = [](const context& ctx) { return !ctx.router.can_go_back(); },
          .on_click    = [](context& ctx) { ctx.router.go_back(ctx); },
      }},
      forward_{{
          .tag         = "##forward",
          .icon_id     = assets::core_icon_id_t::CHEVRON_RIGHT,
          .is_disabled = [](const context& ctx) { return !ctx.router.can_go_forward(); },
          .on_click    = [](context& ctx) { ctx.router.go_forward(ctx); },
      }},
      home_{{
          .tag      = "##home",
          .icon_id  = assets::core_icon_id_t::HOME,
          .on_click = [](context& ctx) { ctx.router.emplace_page<pages::home>(ctx); },
      }},
      search_{{
          .tag           = "##search",
          .icon_id       = assets::core_icon_id_t::SEARCH,
          .disabled_tint = 1,
          .is_disabled   = [](const context&) { return true; },
      }},
      padded_icon_button_dim_{back_.width() + 2 * back_.frame_padding()} {}

auto top_bar::on_mount(context& ctx) -> void {
    menu_.on_mount(ctx);
    back_.on_mount(ctx);
    forward_.on_mount(ctx);
    home_.on_mount(ctx);
    search_.on_mount(ctx);
    ctx.log.info("Mounted top bar");
}

auto top_bar::on_unmount(context& ctx) -> void {
    search_.on_mount(ctx);
    home_.on_mount(ctx);
    forward_.on_unmount(ctx);
    back_.on_unmount(ctx);
    menu_.on_mount(ctx);
    ctx.log.info("Unmounted top bar");
}

auto top_bar::render(context& ctx) -> void {
    PROFILE_FUNCTION();

    const auto window_w     = ImGui::GetWindowWidth();
    const auto window_h     = ImGui::GetWindowHeight();
    const auto item_spacing = ImGui::GetStyle().ItemSpacing.x;
    const auto vertical_y   = (window_h - padded_icon_button_dim_) * 0.5f;
    ImGui::SetCursorPosY(vertical_y);

    // Left controls
    menu_.render(ctx);
    ImGui::SameLine();
    back_.render(ctx);
    ImGui::SameLine();
    forward_.render(ctx);

    // Center group alignment
    constexpr auto search_bar_w   = 360.0f;
    const auto     home_button_w  = padded_icon_button_dim_;
    const auto     center_group_w = home_button_w + item_spacing + search_bar_w;
    const auto     center_x       = (window_w - center_group_w) * 0.5f;
    if (center_x > ImGui::GetCursorPosX() + item_spacing) {
        ImGui::SameLine(center_x);
    } else {
        ImGui::SameLine();
    }

    // Home button
    home_.render(ctx);
    ImGui::SameLine();
}

} // namespace pbnj::ui::components
