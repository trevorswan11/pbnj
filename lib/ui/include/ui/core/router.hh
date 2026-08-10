#pragma once

#include <vector>

#include <stdx/memory.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "ui/pages/page.hh"

namespace pbnj::ui::core {

class router {
  public:
    using location = stdx::box<pages::page>;

  public:
    router()  = default;
    ~router() = default;
    MAKE_PINNED(router);

    // Construct and emplace a new page onto the history stack
    template <typename Page, typename... Args>
    auto emplace_page(context& ctx, Args&&... args) -> void {
        transition_to(ctx, stdx::make_box<Page>(std::forward<Args>(args)...));
    }

    auto go_back(context& ctx) -> bool;
    auto go_forward(context& ctx) -> bool;

    [[nodiscard]] auto can_go_back() const noexcept -> bool { return history_idx_ > 0; }
    [[nodiscard]] auto can_go_forward() const noexcept -> bool {
        return history_idx_ + 1 < history_.size();
    }

    auto render_current(context& ctx) -> void;
    auto update_current(context& ctx, f64 dt) -> void;

  private:
    auto transition_to(context& ctx, location next_page) -> void;

  private:
    std::vector<location> history_;
    usize                 history_idx_;
};

} // namespace pbnj::ui::core
