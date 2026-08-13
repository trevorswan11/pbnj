#pragma once

#include <stdx/types.hh>

#include "ui/components/fluff/icon_button.hh"
#include "ui/components/fluff/search_input.hh"
#include "ui/core/component.hh"

namespace pbnj::ui::components {

class top_bar : public component {
  public:
    top_bar();

    auto on_mount(context& ctx) -> void override;
    auto on_unmount(context& ctx) -> void override;
    auto render(context& ctx) -> void override;

  private:
    icon_button  menu_;
    icon_button  back_;
    icon_button  forward_;
    icon_button  home_;
    search_input search_;

    f32 padded_icon_button_dim_;
};

} // namespace pbnj::ui::components
