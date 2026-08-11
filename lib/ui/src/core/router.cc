#include "ui/core/router.hh"

#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "ui/core/context.hh"

namespace pbnj::ui::core {

auto router::go_back(context& ctx) -> bool {
    if (!can_go_back()) { return false; }
    history_[history_idx_]->on_unmount(ctx);
    --history_idx_;
    history_[history_idx_]->on_mount(ctx);
    return true;
}

auto router::go_forward(context& ctx) -> bool {
    if (!can_go_forward()) { return false; }
    history_[history_idx_]->on_unmount(ctx);
    ++history_idx_;
    history_[history_idx_]->on_mount(ctx);
    return true;
}

auto router::render_current(context& ctx) -> void {
    if (!history_.empty() && history_idx_ < history_.size()) {
        history_[history_idx_]->render(ctx);
    }
}

auto router::update_current(context& ctx, f64 dt) -> void {
    if (!history_.empty() && history_idx_ < history_.size()) {
        history_[history_idx_]->on_update(ctx, dt);
    }
}

auto router::transition_to(context& ctx, location next_page) -> void {
    if (!history_.empty() && history_idx_ < history_.size()) {
        history_[history_idx_]->on_unmount(ctx);
        history_.erase(history_.begin() + static_cast<idiff>(history_idx_ + 1), history_.end());
    }

    next_page->on_mount(ctx);
    history_.emplace_back(std::move(next_page));
    history_idx_ = history_.size() - 1;
}

} // namespace pbnj::ui::core
