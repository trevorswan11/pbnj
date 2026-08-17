#include "ui/core/application.hh"

#include <utility>

#include <pbnj/config.h>
#if PBNJ_WINDOWS
#    include <windows.h>
#    define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <glfw.hh>
#include <gsl/pointers>
#include <gsl/util>
#include <imgui.hh>
#include <stb_image.h>
#include <stdx/memory.hh>
#include <stdx/profiler.hh>
#include <stdx/types.hh>
#include <vulkan/vulkan_core.h>

#include "ui/assets/app_icon.hh"
#include "ui/components/root.hh"
#include "ui/core/component.hh"
#include "ui/core/context.hh"
#include "ui/core/frame.hh"
#include "ui/core/vk_context.hh"
#include "ui/pages/home.hh"
#include "ui/theme/style.hh"

namespace pbnj::ui {

class application::impl {
  public:
    impl() : vk_ctx_{ctx_.log} {}

    auto on_init() noexcept -> bool;
    auto on_frame(f64 dt) noexcept -> void;
    auto on_cleanup() noexcept -> void;
    auto run() noexcept -> void;

    [[nodiscard]] auto valid() const noexcept -> bool { return is_initialized_; }
    auto               update() noexcept -> void;

#if PBNJ_WINDOWS
    static auto CALLBACK window_proc_hook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        -> LRESULT;
    static inline WNDPROC     original_wndproc_{nullptr};
    static inline const char* app_prop_a{"PBnJ_AppImpl"};
#endif

  public:
    context          ctx_;
    vk_context       vk_ctx_;
    GLFWwindow*      window_{nullptr};
    components::root root_;
    f32              current_dpi_{1.0f};
    bool             is_running_{false};
    bool             is_initialized_{false};
    bool             in_frame_render_{false};
    f64              last_time_{0.0};
};

#if PBNJ_WINDOWS
auto CALLBACK application::impl::window_proc_hook(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    -> LRESULT {
    auto* app = static_cast<application::impl*>(GetPropA(hwnd, app_prop_a));

    LRESULT result = CallWindowProc(original_wndproc_, hwnd, msg, wparam, lparam);

    if (app && app->is_initialized_) {
        if (msg == WM_SIZE || msg == WM_SIZING || msg == WM_PAINT) {
            int w = 0, h = 0;
            glfwGetFramebufferSize(app->window_, &w, &h);
            if (w > 0 && h > 0) {
                const f64 now   = glfwGetTime();
                const f64 dt    = now - app->last_time_;
                app->last_time_ = now;
                app->on_frame(dt > 0.0 ? dt : 0.016);
            }
        }
    }

    return result;
}
#endif

application::application() : impl_(stdx::make_box<impl>()) {}
application::~application() = default;

auto application::launch() noexcept -> void {
    PROFILE_FUNCTION();
    impl_->run();
}

auto application::impl::on_init() noexcept -> bool {
    PROFILE_FUNCTION();

    glfwSetErrorCallback([](i32, const char*) {});

    if (!glfwInit()) {
        ctx_.log.error("Failed to initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window_ = glfwCreateWindow(1'280, 720, "PBnJ", nullptr, nullptr);
    if (!window_) {
        ctx_.log.error("Failed to create GLFW window");
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
        const GLFWimage icon_image{
            .width  = width,
            .height = height,
            .pixels = pixels,
        };
        glfwSetWindowIcon(window_, 1, &icon_image);
    }

    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* w, int width, int height) {
        if (auto* app = static_cast<application::impl*>(glfwGetWindowUserPointer(w))) {
            app->vk_ctx_.notify_framebuffer_resized();
            if (app->valid() && width > 0 && height > 0) { app->update(); }
        }
    });

    glfwSetWindowRefreshCallback(window_, [](GLFWwindow* w) {
        if (auto* app = static_cast<application::impl*>(glfwGetWindowUserPointer(w))) {
            if (app->is_initialized_) { app->update(); }
        }
    });

    glfwSetWindowContentScaleCallback(window_, [](GLFWwindow* w, f32 xscale, f32) {
        if (auto* app = static_cast<application::impl*>(glfwGetWindowUserPointer(w))) {
            if (xscale != app->current_dpi_) {
                app->current_dpi_ = xscale;
                app->ctx_.textures.set_dpi_scale(xscale);
                app->ctx_.textures.clear();
            }
        }
    });

#if PBNJ_WINDOWS
    if (HWND hwnd = glfwGetWin32Window(window_)) {
        SetPropA(hwnd, app_prop_a, this);
        original_wndproc_ = reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(window_proc_hook)));
    }
#endif

#ifndef NDEBUG
    if (!vk_ctx_.init(window_, true)) {
#else
    if (!vk_ctx_.init(window_, false)) {
#endif
        ctx_.log.error("Failed to initialize Vulkan context");
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    ImGui_ImplGlfw_InitForVulkan(window_, true);

    ImGui_ImplVulkan_InitInfo init_info{};
    init_info.Instance                     = vk_ctx_.instance();
    init_info.PhysicalDevice               = vk_ctx_.physical_device();
    init_info.Device                       = vk_ctx_.device();
    init_info.QueueFamily                  = vk_ctx_.queue_family_index();
    init_info.Queue                        = vk_ctx_.queue();
    init_info.DescriptorPool               = vk_ctx_.descriptor_pool();
    init_info.MinImageCount                = vk_ctx_.min_image_count();
    init_info.ImageCount                   = vk_ctx_.swapchain_image_count();
    init_info.PipelineInfoMain.RenderPass  = vk_ctx_.render_pass();
    init_info.PipelineInfoMain.Subpass     = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    ImGui_ImplVulkan_Init(&init_info);

    f32 xscale = 1.0f, yscale = 1.0f;
    glfwGetWindowContentScale(window_, &xscale, &yscale);
    current_dpi_ = xscale;

    ctx_.fonts.init(current_dpi_);
    ctx_.log.info("Initialized font manager");
    ctx_.styles.set_mode(window_, theme::mode_t::DARK);
    ctx_.log.info("Applied application-wide dark mode");
    ctx_.textures.init(&vk_ctx_, current_dpi_);

    ctx_.router.emplace_page<pages::home>(ctx_);
    root_.on_mount(ctx_);

    last_time_      = glfwGetTime();
    is_initialized_ = true;

    // Pre-render the first frame and then reveal the window for seamless startup
    on_frame(0.016);
    glfwShowWindow(window_);
    return true;
}

auto application::impl::on_frame(f64 dt) noexcept -> void {
    PROFILE_FUNCTION();
    if (!is_initialized_ || in_frame_render_) { return; }
    in_frame_render_       = true;
    const auto reset_guard = gsl::finally([this] { in_frame_render_ = false; });

    const ui::frame frame{vk_ctx_, window_, dt, ctx_.styles.clear_color()};
    if (frame.is_active()) {
        ctx_.router.update_current(ctx_, dt);
        root_.render(ctx_);
    }
}

auto application::impl::on_cleanup() noexcept -> void {
    PROFILE_FUNCTION();
    is_initialized_ = false;

#if PBNJ_WINDOWS
    if (window_) {
        HWND hwnd = glfwGetWin32Window(window_);
        if (hwnd && original_wndproc_) {
            SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(original_wndproc_));
            RemovePropA(hwnd, app_prop_a);
            original_wndproc_ = nullptr;
        }
    }
#endif

    if (window_) {
        glfwSetWindowRefreshCallback(window_, nullptr);
        glfwSetFramebufferSizeCallback(window_, nullptr);
        glfwSetWindowContentScaleCallback(window_, nullptr);
    }

    if (vk_ctx_.is_initialized()) {
        vk_ctx_.wait_idle();

        root_.on_unmount(ctx_);
        ctx_.textures.clear();

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        vk_ctx_.cleanup();
    }

    if (window_) { glfwDestroyWindow(std::exchange(window_, nullptr)); }
    glfwTerminate();
}

auto application::impl::run() noexcept -> void {
    if (!on_init()) {
        on_cleanup();
        return;
    }

    is_running_ = true;
    while (is_running_ && !glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        update();
    }

    on_cleanup();
}

auto application::impl::update() noexcept -> void {
    const f64 now = glfwGetTime();
    const f64 dt  = now - last_time_;
    last_time_    = now;
    on_frame(dt > 0.0 ? dt : 0.016);
}

} // namespace pbnj::ui
