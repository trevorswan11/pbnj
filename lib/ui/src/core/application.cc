#include "ui/core/application.hh"

#include <gsl/pointers>
#include <sokol.h>
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
    auto on_init() noexcept -> void;
    auto on_frame() noexcept -> void;
    auto on_event(const sapp_event* event) noexcept -> void;
    auto on_cleanup() noexcept -> void;

  private:
    context          ctx_;
    components::root root_;
    f32              current_dpi_;
};

application::application() : impl_(stdx::make_box<impl>()) {}
application::~application() = default;

auto application::launch() noexcept -> void {
    PROFILE_FUNCTION();
    sapp_desc desc{};
    desc.init_userdata_cb = [](void* data) noexcept -> void {
        gsl::not_null app = static_cast<impl*>(data);
        app->on_init();
    };
    desc.frame_userdata_cb = [](void* data) noexcept -> void {
        gsl::not_null app = static_cast<impl*>(data);
        app->on_frame();
    };
    desc.event_userdata_cb = [](const sapp_event* e, void* data) noexcept -> void {
        gsl::not_null app = static_cast<impl*>(data);
        app->on_event(e);
    };
    desc.cleanup_userdata_cb = [](void* data) noexcept -> void {
        gsl::not_null app = static_cast<impl*>(data);
        app->on_cleanup();
    };
    desc.user_data                   = impl_.get();
    desc.window_title                = "PBnJ";
    desc.ios.keyboard_resizes_canvas = false;
    desc.icon.sokol_default          = false;
    desc.enable_clipboard            = true;
    desc.logger.func                 = slog_func;

    // Custom image loaded for app icon
    i32   width, height, comp;
    auto* pixels               = stbi_load_from_memory(assets::app_icon.data(),
                                         static_cast<i32>(assets::app_icon.size()),
                                         &width,
                                         &height,
                                         &comp,
                                         4);
    desc.icon.images[0].width  = width;
    desc.icon.images[0].height = height;
    desc.icon.images[0].pixels = {
        .ptr  = pixels,
        .size = static_cast<usize>(width * height * 4),
    };

    sapp_run(desc);
}

auto application::impl::on_init() noexcept -> void {
    PROFILE_FUNCTION();
    sg_desc desc{};
    desc.environment = sglue_environment();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    simgui_desc_t simgui_desc{};
    simgui_desc.logger.func = slog_func;
    simgui_setup(&simgui_desc);

    current_dpi_ = sapp_dpi_scale();
    ctx_.fonts.init(current_dpi_);
    ctx_.log.info("Initialized font manager");
    ctx_.styles.apply_dark_mode();
    ctx_.log.info("Applied application-wide dark mode");

    ctx_.router.emplace_page<pages::home>(ctx_);
    root_.on_mount(ctx_);
}

auto application::impl::on_frame() noexcept -> void {
    PROFILE_FUNCTION();
    const ui::frame frame;

    const auto dt = frame.get_dt();
    ctx_.router.update_current(ctx_, dt);

    root_.render(ctx_);
}

auto application::impl::on_event(const sapp_event* event) noexcept -> void {
    PROFILE_FUNCTION();
    simgui_handle_event(event);

    if (event->type == SAPP_EVENTTYPE_RESIZED) {
        const auto dpi = sapp_dpi_scale();
        if (dpi != current_dpi_) {
            current_dpi_ = dpi;
            ctx_.textures.clear();
        }
    }
}

auto application::impl::on_cleanup() noexcept -> void {
    PROFILE_FUNCTION();
    root_.on_unmount(ctx_);
    ctx_.textures.clear();
    simgui_shutdown();
    sg_shutdown();
}

} // namespace pbnj::ui
