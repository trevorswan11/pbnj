#pragma once

#include <string>
#include <utility>

#include <gsl/span>
#include <imgui.h>
#include <stdx/function.hh>
#include <stdx/option.hh>
#include <stdx/types.hh>

#include "ui/assets/texture_cache.hh"
#include "ui/core/component.hh"
#include "ui/core/context.hh"

namespace pbnj::ui::components {

struct icon_button_props {
    std::string                          tag;
    assets::core_icon_id_t               icon_id;
    f32                                  padding{6.0f};
    f32                                  disabled_tint{0.25f};
    ImVec2                               size{18.0f, 18.0f};
    stdx::function<bool(const context&)> is_disabled{[](const context&) { return false; }};
    stdx::function<void(context&)>       on_click{nullptr};
};

class icon_button : public component, private icon_button_props {
  public:
    explicit icon_button(icon_button_props props) noexcept : icon_button_props{std::move(props)} {}

    auto on_mount(context& ctx) -> void override;
    auto on_unmount(context& ctx) -> void override;
    auto render(context& ctx) -> void override;

    [[nodiscard]] auto frame_padding() const noexcept { return padding; }
    [[nodiscard]] auto width() const noexcept { return size.x; }
    [[nodiscard]] auto height() const noexcept { return size.y; }
};

} // namespace pbnj::ui::components
