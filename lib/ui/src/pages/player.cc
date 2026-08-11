#include "ui/pages/player.hh"

#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/core/component.hh"
#include "ui/core/context.hh"

namespace pbnj::ui::pages {

auto player::on_mount(context& ctx) -> void { ctx.log.info("Mounted player page"); }

auto player::on_unmount(context& ctx) -> void { ctx.log.info("Unmounted player page"); }

auto player::render(context& ctx) -> void {
    PROFILE_FUNCTION();
    DISCARD(ctx);
}

} // namespace pbnj::ui::pages
