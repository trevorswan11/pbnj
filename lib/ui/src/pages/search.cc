#include "ui/pages/search.hh"

#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/core/component.hh"
#include "ui/core/context.hh"

namespace pbnj::ui::pages {

auto search::on_mount(context& ctx) -> void { ctx.log.info("Mounted search page"); }

auto search::on_unmount(context& ctx) -> void { ctx.log.info("Unmounted search page"); }

auto search::render(context& ctx) -> void {
    PROFILE_FUNCTION();
    DISCARD(ctx);
}

} // namespace pbnj::ui::pages
