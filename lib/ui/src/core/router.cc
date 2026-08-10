#include "ui/core/router.hh"

#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "ui/core/context.hh"

namespace pbnj::ui::core {

auto router::go_back(context& ctx) -> bool {
    DISCARD(ctx);
    return false;
}

auto router::go_forward(context& ctx) -> bool {
    DISCARD(ctx);
    return false;
}

auto router::render_current(context& ctx) -> void { DISCARD(ctx); }

auto router::update_current(context& ctx, f64 dt) -> void {
    DISCARD(ctx);
    DISCARD(dt);
}

auto router::transition_to(context& ctx, location next_page) -> void {
    DISCARD(ctx);
    DISCARD(next_page);
}

} // namespace pbnj::ui::core
