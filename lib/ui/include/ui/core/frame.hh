#pragma once

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>
#include <imgui.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

namespace pbnj::ui {

class frame {
  public:
    explicit frame(SDL_Window* window, SDL_GPUDevice* device, f64 dt, ImVec4 clear_color) noexcept;
    ~frame();
    MAKE_PINNED(frame);

    MAKE_GETTER(dt, f64)

  private:
    SDL_Window*    window_;
    SDL_GPUDevice* device_;
    f64            dt_;
    ImVec4         clear_color_;
};

} // namespace pbnj::ui
