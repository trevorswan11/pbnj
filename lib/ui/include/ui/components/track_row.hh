#pragma once

#include <chrono>
#include <string>
#include <utility>

#include <stdx/function.hh>
#include <stdx/types.hh>

#include "ui/core/component.hh"

namespace pbnj::ui::components {

struct track_row_props {
    std::string          track_id;
    std::string          title;
    std::string          artist;
    std::string          album;
    std::string          thumbnail_url;
    std::chrono::seconds duration{0};
    bool                 is_active{false};
    bool                 is_playing{false};

    stdx::function<void()> on_play;
    stdx::function<void()> on_queue;
    stdx::function<void()> on_open_artist;
    stdx::function<void()> on_open_album;
};

class track_row : public component, private track_row_props {
  public:
    explicit track_row(track_row_props props) noexcept : track_row_props{std::move(props)} {}

    auto on_mount(context& ctx) -> void override;
    auto on_unmount(context& ctx) -> void override;
    auto render(context& ctx) -> void override;
};

} // namespace pbnj::ui::components
