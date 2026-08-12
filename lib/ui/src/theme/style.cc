#include "ui/theme/style.hh"

#ifdef _WIN32
#    include <dwmapi.h>
#    include <minwindef.h>
#    include <windef.h>
#endif

#include <gsl/span>
#include <sokol.h>

#include "ui/theme/colors.hh"

namespace pbnj::ui::theme {

auto style_manager::apply_dark_mode() noexcept -> void {
#ifdef _WIN32
    HWND hwnd          = reinterpret_cast<HWND>(const_cast<void*>(sapp_win32_get_hwnd()));
    BOOL use_dark_mode = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &use_dark_mode, sizeof(use_dark_mode));
#endif

    auto& style             = ImGui::GetStyle();
    style.WindowRounding    = 0;
    style.ChildRounding     = 8;
    style.FrameRounding     = 6;
    style.PopupRounding     = 8;
    style.ScrollbarRounding = 12;
    style.GrabRounding      = 6;
    style.WindowPadding     = {12, 12};
    style.ItemSpacing       = {8, 8};

    using namespace colors;

    // cppcheck-suppress-begin unreadVariable
    gsl::span colors                  = style.Colors;
    colors[ImGuiCol_WindowBg]         = dark_grey;
    colors[ImGuiCol_ChildBg]          = light_dark_grey;
    colors[ImGuiCol_PopupBg]          = grey;
    colors[ImGuiCol_Header]           = dark_light_grey;
    colors[ImGuiCol_HeaderHovered]    = light_grey;
    colors[ImGuiCol_Button]           = dark_light_grey;
    colors[ImGuiCol_ButtonHovered]    = light_grey;
    colors[ImGuiCol_SliderGrab]       = dark_light_green;
    colors[ImGuiCol_SliderGrabActive] = light_green;
    // cppcheck-suppress-end unreadVariable
}

} // namespace pbnj::ui::theme
