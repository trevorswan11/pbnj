#include "ui/core/application.hh"

#include <gsl/pointers>
#include <sokol.h>
#include <stb_image.h>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "ui/assets/app_icon.hh"
#include "ui/core/frame.hh"
#include "ui/pages/home.hh"

namespace pbnj::ui {

auto application::launch() noexcept -> void {
    PROFILE_FUNCTION();
    sapp_desc desc{};
    desc.init_userdata_cb = [](void* data) noexcept -> void {
        gsl::not_null app = static_cast<application*>(data);
        app->on_init();
    };
    desc.frame_userdata_cb = [](void* data) noexcept -> void {
        gsl::not_null app = static_cast<application*>(data);
        app->on_frame();
    };
    desc.event_userdata_cb = [](const sapp_event* e, void* data) noexcept -> void {
        gsl::not_null app = static_cast<application*>(data);
        app->on_event(e);
    };
    desc.cleanup_userdata_cb = [](void* data) noexcept -> void {
        gsl::not_null app = static_cast<application*>(data);
        app->on_cleanup();
    };
    desc.user_data                   = this;
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

auto application::on_init() noexcept -> void {
    PROFILE_FUNCTION();
    sg_desc desc{};
    desc.environment = sglue_environment();
    desc.logger.func = slog_func;
    sg_setup(&desc);

    simgui_desc_t simgui_desc{};
    simgui_desc.logger.func = slog_func;
    simgui_setup(&simgui_desc);

    ctx_.fonts.init(sapp_dpi_scale());
    ctx_.log.info("Initialized font manager");
    ctx_.styles.apply_dark_mode();
    ctx_.log.info("Applied application-wide dark mode");

    ctx_.router.emplace_page<pages::home>(ctx_);
}

auto application::on_frame() noexcept -> void {
    PROFILE_FUNCTION();
    const ui::frame frame;

    const auto dt = frame.get_dt();
    ctx_.router.update_current(ctx_, dt);

    root_.render(ctx_);
}

auto application::on_event(const sapp_event* event) noexcept -> void {
    PROFILE_FUNCTION();
    simgui_handle_event(event);
}

auto application::on_cleanup() noexcept -> void {
    PROFILE_FUNCTION();
    simgui_shutdown();
    sg_shutdown();
}

} // namespace pbnj::ui
