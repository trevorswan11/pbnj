#include "ui/pages/album.hh"

#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/core/component.hh"
#include "ui/core/context.hh"

namespace pbnj::ui::pages {

auto album::on_mount(context& ctx) -> void { ctx.log.info("Mounted album page"); }

auto album::on_unmount(context& ctx) -> void { ctx.log.info("Unmounted album page"); }

auto album::render(context& ctx) -> void {
    PROFILE_FUNCTION();
    DISCARD(ctx);
}

} // namespace pbnj::ui::pages
