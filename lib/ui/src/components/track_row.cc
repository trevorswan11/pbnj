#include "ui/components/track_row.hh"

#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/core/context.hh"

namespace pbnj::ui::components {

auto track_row::on_mount(context& ctx) -> void { ctx.log.info("Mounted track row"); }

auto track_row::on_unmount(context& ctx) -> void { ctx.log.info("Unmounted track row"); }

auto track_row::render(context& ctx) -> void {
    PROFILE_FUNCTION();
    DISCARD(ctx);
}

} // namespace pbnj::ui::components
