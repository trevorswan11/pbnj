#include "ui/core/application.hh"

#include <utility>

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <gsl/pointers>
#include <gsl/util>
#include <imgui.hh>
#include <stb_image.h>
#include <stdx/memory.hh>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "ui/assets/app_icon.hh"
#include "ui/components/root.hh"
#include "ui/core/component.hh"
#include "ui/core/context.hh"
#include "ui/core/frame.hh"
#include "ui/pages/home.hh"

namespace pbnj::ui {

class application::impl {
  public:
    auto on_init() noexcept -> bool;
    auto on_frame(f64 dt) noexcept -> void;
    auto on_event(const SDL_Event& event) noexcept -> void;
    auto on_cleanup() noexcept -> void;
    auto run() noexcept -> void;

    static auto SDLCALL event_watch_callback(void* userdata, SDL_Event* event) -> bool;

  public:
    bool is_initialized_{false};
    u64  last_ticks_ns_{0};

  private:
    SDL_Window*      window_{nullptr};
    SDL_GPUDevice*   gpu_device_{nullptr};
    context          ctx_;
    components::root root_;
    f32              current_dpi_{1.0f};
    bool             is_running_{false};
};

application::application() : impl_(stdx::make_box<impl>()) {}
application::~application() = default;

auto application::launch() noexcept -> void {
    PROFILE_FUNCTION();
    impl_->run();
}

auto SDLCALL application::impl::event_watch_callback(void* userdata, SDL_Event* event) -> bool {
    auto* app = static_cast<application::impl*>(userdata);
    if (!app || !app->is_initialized_) { return true; }
    if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED ||
        event->type == SDL_EVENT_WINDOW_EXPOSED) {
        const u64 now       = SDL_GetTicksNS();
        const f64 dt        = static_cast<f64>(now - app->last_ticks_ns_) * 1e-9;
        app->last_ticks_ns_ = now;
        app->on_frame(dt > 0.0 ? dt : 0.016);
    }
    return true;
}

auto application::impl::on_init() noexcept -> bool {
    PROFILE_FUNCTION();
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        ctx_.log.error("Failed to initialize SDL3: {}", SDL_GetError());
        return false;
    }

    // Keep window hidden initially to avoid black frame while initializing
    window_ =
        SDL_CreateWindow("PBnJ",
                         1'280,
                         720,
                         SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_HIDDEN);
    if (!window_) {
        ctx_.log.error("Failed to create SDL3 window: {}", SDL_GetError());
        return false;
    }

    // Set window icon
    i32        width, height, comp;
    auto*      pixels         = stbi_load_from_memory(assets::app_icon.data(),
                                         static_cast<i32>(assets::app_icon.size()),
                                         &width,
                                         &height,
                                         &comp,
                                         4);
    const auto pixels_cleanup = gsl::finally([pixels] { stbi_image_free(pixels); });

    if (pixels) {
        auto* surface =
            SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGBA32, pixels, width * 4);
        if (surface) {
            SDL_SetWindowIcon(window_, surface);
            SDL_DestroySurface(surface);
        }
    }

    gpu_device_ = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXBC |
                                          SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB,
                                      false,
                                      nullptr);
    if (!gpu_device_) {
        ctx_.log.error("Failed to create SDL GPU device: {}", SDL_GetError());
        return false;
    }

    if (!SDL_ClaimWindowForGPUDevice(gpu_device_, window_)) {
        ctx_.log.error("Failed to claim window for SDL GPU device: {}", SDL_GetError());
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    ImGui_ImplSDL3_InitForSDLGPU(window_);

    ImGui_ImplSDLGPU3_InitInfo init_info{};
    init_info.Device            = gpu_device_;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(gpu_device_, window_);
    init_info.MSAASamples       = SDL_GPU_SAMPLECOUNT_1;
    ImGui_ImplSDLGPU3_Init(&init_info);

    current_dpi_ = SDL_GetWindowDisplayScale(window_);
    ctx_.fonts.init(current_dpi_);
    ctx_.log.info("Initialized font manager");
    ctx_.styles.apply_dark_mode(window_);
    ctx_.log.info("Applied application-wide dark mode");
    ctx_.textures.init(gpu_device_, current_dpi_);

    ctx_.router.emplace_page<pages::home>(ctx_);
    root_.on_mount(ctx_);

    last_ticks_ns_  = SDL_GetTicksNS();
    is_initialized_ = true;

    // Attach synchronous event watch for live, smooth window resizing during modal drag
    SDL_AddEventWatch(event_watch_callback, this);

    // Pre-render the first frame and then reveal the window for seamless startup
    on_frame(0.016);
    SDL_ShowWindow(window_);
    return true;
}

auto application::impl::on_frame(f64 dt) noexcept -> void {
    PROFILE_FUNCTION();
    if (!is_initialized_) { return; }

    const ui::frame frame{window_, gpu_device_, dt};

    ctx_.router.update_current(ctx_, dt);
    root_.render(ctx_);
}

auto application::impl::on_event(const SDL_Event& event) noexcept -> void {
    PROFILE_FUNCTION();
    ImGui_ImplSDL3_ProcessEvent(&event);

    if (event.type == SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED) {
        const auto dpi = SDL_GetWindowDisplayScale(window_);
        if (dpi != current_dpi_) {
            current_dpi_ = dpi;
            ctx_.textures.set_dpi_scale(dpi);
            ctx_.textures.clear();
        }
    }
}

auto application::impl::on_cleanup() noexcept -> void {
    PROFILE_FUNCTION();
    is_initialized_ = false;
    SDL_RemoveEventWatch(event_watch_callback, this);

    root_.on_unmount(ctx_);
    ctx_.textures.clear();

    ImGui_ImplSDLGPU3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (gpu_device_) { SDL_DestroyGPUDevice(std::exchange(gpu_device_, nullptr)); }
    if (window_) { SDL_DestroyWindow(std::exchange(window_, nullptr)); }
    SDL_Quit();
}

auto application::impl::run() noexcept -> void {
    if (!on_init()) {
        on_cleanup();
        return;
    }

    is_running_ = true;
    while (is_running_) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
                is_running_ = false;
            }
            on_event(event);
        }
        if (!is_running_) { break; }

        const u64 now  = SDL_GetTicksNS();
        const f64 dt   = static_cast<f64>(now - last_ticks_ns_) * 1e-9;
        last_ticks_ns_ = now;

        on_frame(dt > 0.0 ? dt : 0.016);
    }

    on_cleanup();
}

} // namespace pbnj::ui
