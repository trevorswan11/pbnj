#pragma once

#include <string>
#include <string_view>
#include <utility>

#include <stdx/fixed/string.hh>
#include <stdx/function.hh>
#include <stdx/types.hh>

#include "ui/assets/texture_cache.hh"
#include "ui/core/component.hh"
#include "ui/core/context.hh"

namespace pbnj::ui::components {

struct search_input_props {
    std::string            tag;
    stdx::fixed::string    placeholder{"What do you want to play?"};
    assets::core_icon_id_t icon_id{assets::core_icon_id_t::SEARCH};
    f32                    width;
    f32                    icon_size{16.0f};
    f32                    icon_pad_x{10.0f};
    f32                    text_pad_left{icon_pad_x + icon_size + 8.0f};
    stdx::function<void(context&, std::string_view query)> on_change{nullptr};
    stdx::function<void(context&, std::string_view query)> on_submit{nullptr};
};

class search_input : public component, private search_input_props {
  public:
    explicit search_input(search_input_props props) noexcept
        : search_input_props{std::move(props)} {}

    auto on_mount(context& ctx) -> void override;
    auto on_unmount(context& ctx) -> void override;
    auto render(context& ctx) -> void override;

    auto set_width(f32 w) noexcept -> void { width = w; }
};

} // namespace pbnj::ui::components
