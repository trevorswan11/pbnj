#include "ui/components/side_bar.hh"

#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/core/context.hh"

namespace pbnj::ui::components {

auto side_bar::on_mount(context& ctx) -> void { ctx.log.info("Mounted side bar"); }

auto side_bar::on_unmount(context& ctx) -> void { ctx.log.info("Unmounted side bar"); }

auto side_bar::render(context& ctx) -> void {
    PROFILE_FUNCTION();
    DISCARD(ctx);
}

} // namespace pbnj::ui::components
