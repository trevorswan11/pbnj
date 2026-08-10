#pragma once

#include "ui/core/component.hh"

namespace pbnj::ui::components {

class top_bar : public component {
  public:
    auto render(context& ctx) -> void override;
};

} // namespace pbnj::ui::components
