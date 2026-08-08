#include "ui/core/application.hh"

#include <gsl/pointers>
#include <sokol.h>
#include <stb_image.h>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "ui/assets/app_icon.hh"
#include "ui/core/frame.hh"

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
}

auto application::on_frame() noexcept -> void {
    PROFILE_FUNCTION();
    const ui::frame frame;

    // 1. Show a simple window
    ImGui::Text("Hello, world!");
    ImGui::SliderFloat("float", &slider_, 0.0f, 1.0f);
    if (ImGui::Button("Test Window")) { show_test_window_ ^= true; }
    if (ImGui::Button("Another Window")) { show_another_window_ ^= true; }
    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::Text("w: %d, h: %d, dpi_scale: %.1f", sapp_width(), sapp_height(), sapp_dpi_scale());
    if (ImGui::Button(sapp_is_fullscreen() ? "Switch to windowed" : "Switch to fullscreen")) {
        sapp_toggle_fullscreen();
    }
    ImGui::Text("sapp_frame_duration: %.6f", sapp_frame_duration());
    ImGui::Text("sapp_frame_duration_unfiltered: %.6f", sapp_frame_duration_unfiltered());

    // 2. Show another simple window, this time using an explicit Begin/End pair
    if (show_another_window_) {
        ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiCond_FirstUseEver);
        ImGui::Begin("Another Window", &show_another_window_);
        ImGui::Text("Hello");
        ImGui::End();
    }

    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowDemoWindow()
    if (show_test_window_) {
        ImGui::SetNextWindowPos(ImVec2(460, 20), ImGuiCond_FirstUseEver);
        ImGui::ShowDemoWindow();
    }
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
