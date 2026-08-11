#pragma once

#include <imgui.h>

#include "ui/core/component.hh"

namespace pbnj::ui::components {

class top_bar : public component {
  public:
    auto on_mount(context& ctx) -> void override;
    auto on_unmount(context& ctx) -> void override;
    auto render(context& ctx) -> void override;

  private:
    ImTextureID back_;
    ImTextureID forward_;
};

} // namespace pbnj::ui::components
