#include "ui/core/frame.hh"

#include <sokol.h>

#include "ui/theme/colors.hh"

namespace pbnj::ui {

frame::frame(ImVec4 clear_color) noexcept
    : width_{sapp_width()}, height_{sapp_height()}, dt_{sapp_frame_duration()},
      dpi_scale_{sapp_dpi_scale()}, clear_color_{clear_color} {
    const simgui_frame_desc_t frame_desc{
        .width      = width_,
        .height     = height_,
        .delta_time = dt_,
        .dpi_scale  = dpi_scale_,
    };
    simgui_new_frame(frame_desc);
}

frame::~frame() {
    sg_pass_action pass_action{};
    pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass_action.colors[0].clear_value = theme::colors::as_sokol_color(clear_color_);

    sg_pass pass   = {};
    pass.action    = pass_action;
    pass.swapchain = sglue_swapchain();

    sg_begin_pass(pass);
    simgui_render();
    sg_end_pass();
    sg_commit();
}

} // namespace pbnj::ui
