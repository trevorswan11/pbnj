#pragma once

#include <stdx/types.hh>

namespace pbnj::ui {

struct context;

class component {
  public:
    virtual ~component() = default;

    virtual auto on_mount(context& /* ctx */) -> void {}
    virtual auto on_unmount(context& /* ctx */) -> void {}
    virtual auto on_update(context& /* ctx */, f64 /* dt */) -> void {}

    virtual auto render(context& ctx) -> void = 0;
};

} // namespace pbnj::ui
