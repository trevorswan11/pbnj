#include "ui/theme/style.hh"

#ifdef _WIN32
#    include <dwmapi.h>
#    include <minwindef.h>
#    include <windef.h>
#endif

#include <gsl/span>
#include <sokol.h>

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

    gsl::span colors                  = style.Colors;
    colors[ImGuiCol_WindowBg]         = {0.07f, 0.07f, 0.08f, 1};
    colors[ImGuiCol_ChildBg]          = {0.11f, 0.11f, 0.13f, 1};
    colors[ImGuiCol_PopupBg]          = {0.14f, 0.14f, 0.16f, 1};
    colors[ImGuiCol_Header]           = {0.18f, 0.18f, 0.22f, 1};
    colors[ImGuiCol_HeaderHovered]    = {0.24f, 0.24f, 0.28f, 1};
    colors[ImGuiCol_Button]           = {0.18f, 0.18f, 0.22f, 1};
    colors[ImGuiCol_ButtonHovered]    = {0.24f, 0.24f, 0.28f, 1};
    colors[ImGuiCol_SliderGrab]       = {0.11f, 0.73f, 0.33f, 1};
    colors[ImGuiCol_SliderGrabActive] = {0.15f, 0.85f, 0.38f, 1};
}

} // namespace pbnj::ui::theme
