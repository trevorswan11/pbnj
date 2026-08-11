#include "ui/pages/home.hh"

#include <stdx/profiler.hh>
#include <stdx/utility.hh>

#include "ui/core/component.hh"
#include "ui/core/context.hh"

namespace pbnj::ui::pages {

auto home::on_mount(context& ctx) -> void { ctx.log.info("Mounted home page"); }

auto home::on_unmount(context& ctx) -> void { ctx.log.info("Unmounted home page"); }

auto home::render(context& ctx) -> void {
    PROFILE_FUNCTION();
    DISCARD(ctx);
}

} // namespace pbnj::ui::pages
