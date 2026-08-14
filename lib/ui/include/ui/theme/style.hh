#pragma once

#include <SDL3/SDL_video.h>

namespace pbnj::ui::theme {

class style_manager {
  public:
    auto apply_dark_mode(SDL_Window* window = nullptr) noexcept -> void;
};

} // namespace pbnj::ui::theme
