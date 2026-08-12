#include "ui/components/top_bar.hh"

#include <gsl/util>
#include <imgui.h>
#include <stdx/assert.hh>
#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/assets/texture_cache.hh"
#include "ui/core/context.hh"

namespace pbnj::ui::components {

top_bar::top_bar()
    : back_{{
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
      padded_nav_button_height_{back_.width() + 2 * back_.frame_padding()} {
    ASSERT(back_.width() == back_.height());
    ASSERT(back_.width() == forward_.width());
    ASSERT(back_.frame_padding() == forward_.frame_padding());
}

auto top_bar::on_mount(context& ctx) -> void {
    back_.on_mount(ctx);
    forward_.on_mount(ctx);
    ctx.log.info("Mounted top bar");
}

auto top_bar::on_unmount(context& ctx) -> void {
    forward_.on_unmount(ctx);
    back_.on_unmount(ctx);
    ctx.log.info("Unmounted top bar");
}

auto top_bar::render(context& ctx) -> void {
    PROFILE_FUNCTION();
    ImGui::SetCursorPosY((ImGui::GetWindowHeight() - padded_nav_button_height_) * 0.5f);
    back_.render(ctx);
    ImGui::SameLine();
    forward_.render(ctx);
}

} // namespace pbnj::ui::components
