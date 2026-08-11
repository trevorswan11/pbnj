#include "ui/components/top_bar.hh"

#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/core/context.hh"

namespace pbnj::ui::components {

auto top_bar::on_mount(context& ctx) -> void { ctx.log.info("Mounted top bar"); }

auto top_bar::on_unmount(context& ctx) -> void { ctx.log.info("Unmounted top bar"); }

auto top_bar::render(context& ctx) -> void {
    PROFILE_FUNCTION();
    DISCARD(ctx);
}

} // namespace pbnj::ui::components
