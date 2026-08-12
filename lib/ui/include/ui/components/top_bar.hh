#pragma once

#include <stdx/types.hh>

#include "ui/components/fluff/icon_button.hh"
#include "ui/core/component.hh"

namespace pbnj::ui::components {

class top_bar : public component {
  public:
    top_bar();

    auto on_mount(context& ctx) -> void override;
    auto on_unmount(context& ctx) -> void override;
    auto render(context& ctx) -> void override;

  private:
    icon_button back_;
    icon_button forward_;
    f32         padded_nav_button_height_;
};

} // namespace pbnj::ui::components
