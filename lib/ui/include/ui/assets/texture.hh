#pragma once

#include <SDL3/SDL_gpu.h>
#include <imgui.hh>
#include <stdx/option.hh>
#include <stdx/utility.hh>

#include "ui/core/compact_types.hh" // IWYU pragma: keep

namespace pbnj::ui::assets {

struct texture {
    texture() = default;
    ~texture() { release(); }

    texture(const texture&)                    = delete;
    auto operator=(const texture&) -> texture& = delete;

    texture(texture&& other) noexcept
        : device{std::exchange(other.device, nullptr)},
          handle{std::exchange(other.handle, nullptr)},
          sampler{std::exchange(other.sampler, nullptr)}, imgui_id{other.imgui_id.take()} {}

    auto operator=(texture&& other) noexcept -> texture& {
        if (this != &other) {
            release();
            device   = std::exchange(other.device, nullptr);
            handle   = std::exchange(other.handle, nullptr);
            sampler  = std::exchange(other.sampler, nullptr);
            imgui_id = other.imgui_id.take();
        }
        return *this;
    }

    auto release() noexcept -> void {
        if (device && handle) { SDL_ReleaseGPUTexture(device, std::exchange(handle, nullptr)); }
        sampler = nullptr;
        imgui_id.reset();
        device = nullptr;
    }

    SDL_GPUDevice*            device{nullptr};
    SDL_GPUTexture*           handle{nullptr};
    SDL_GPUSampler*           sampler{nullptr};
    stdx::option<ImTextureID> imgui_id;
};

} // namespace pbnj::ui::assets
