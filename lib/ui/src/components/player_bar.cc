#include "ui/components/player_bar.hh"

#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/core/context.hh"

namespace pbnj::ui::components {

auto player_bar::on_mount(context& ctx) -> void { ctx.log.info("Mounted player bar"); }

auto player_bar::on_unmount(context& ctx) -> void { ctx.log.info("Unmounted player bar"); }

auto player_bar::render(context& ctx) -> void {
    PROFILE_FUNCTION();
    DISCARD(ctx);
}

} // namespace pbnj::ui::components
