#include "ui/components/top_bar.hh"

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
    DISCARD(ctx);
}

} // namespace pbnj::ui::components
