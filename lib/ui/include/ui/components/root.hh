#pragma once

#include "ui/components/player_bar.hh"
#include "ui/components/side_bar.hh"
#include "ui/components/top_bar.hh"
#include "ui/core/component.hh"

namespace pbnj::ui::components {

class root : public component {
  public:
    auto render(context& ctx) -> void override;

  private:
    top_bar    top_bar_;
    side_bar   side_bar_;
    player_bar player_bar_;
};

} // namespace pbnj::ui::components
