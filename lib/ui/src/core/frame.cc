#include "ui/core/frame.hh"

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_video.h>
#include <imgui.hh>
#include <stdx/types.hh>

#include "ui/theme/colors.hh"

namespace pbnj::ui {

frame::frame(SDL_Window* window, SDL_GPUDevice* device, f64 dt, ImVec4 clear_color) noexcept
    : window_{window}, device_{device}, dt_{dt}, clear_color_{clear_color} {
    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

frame::~frame() {
    ImGui::Render();

    auto* draw_data = ImGui::GetDrawData();
    auto* cmd       = SDL_AcquireGPUCommandBuffer(device_);
    if (!cmd) { return; }

    // Upload vertex/index buffers for SDL_GPU before beginning render pass
    ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, cmd);

    SDL_GPUTexture* swapchain_tex = nullptr;
    if (SDL_AcquireGPUSwapchainTexture(cmd, window_, &swapchain_tex, nullptr, nullptr) &&
        swapchain_tex) {
        SDL_GPUColorTargetInfo color_target{};
        color_target.texture     = swapchain_tex;
        color_target.clear_color = theme::colors::as_sdl_color(clear_color_);
        color_target.load_op     = SDL_GPU_LOADOP_CLEAR;
        color_target.store_op    = SDL_GPU_STOREOP_STORE;

        if (auto* pass = SDL_BeginGPURenderPass(cmd, &color_target, 1, nullptr)) {
            ImGui_ImplSDLGPU3_RenderDrawData(draw_data, cmd, pass);
            SDL_EndGPURenderPass(pass);
        }
    }

    SDL_SubmitGPUCommandBuffer(cmd);
}

} // namespace pbnj::ui
