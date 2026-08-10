#pragma once

#include "ui/pages/page.hh"

namespace pbnj::ui::pages {

class home : public page {
  public:
    auto on_mount(context& ctx) -> void override;
    auto on_unmount(context& ctx) -> void override;
    auto render(context& ctx) -> void override;
};

} // namespace pbnj::ui::pages
