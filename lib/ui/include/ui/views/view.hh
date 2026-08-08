#pragma once

#include <string>
#include <string_view>
#include <utility>

#include "ui/core/component.hh"

namespace pbnj::ui::views {

class view : public component {
  public:
    explicit view(std::string title) noexcept : title_{std::move(title)} {}

    [[nodiscard]] virtual auto get_title() const noexcept -> std::string_view { return title_; }

    virtual auto render_top_bar(context& /* ctx */) -> void {}

  protected:
    std::string title_;
};

} // namespace pbnj::ui::views
