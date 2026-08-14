#include "ui/theme/style.hh"

#ifdef _WIN32
#    include <dwmapi.h>
#    include <minwindef.h>
#    include <windef.h>
#endif

#include <SDL3/SDL_properties.h>
#include <SDL3/SDL_video.h>
#include <gsl/span>
#include <imgui.hh>

#include "ui/theme/colors.hh"

namespace pbnj::ui::theme {

auto style_manager::apply_dark_mode(SDL_Window* window) noexcept -> void {
#ifdef _WIN32
    if (window) {
        auto* hwnd = static_cast<HWND>(SDL_GetPointerProperty(
            SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
        if (hwnd) {
            BOOL use_dark_mode = TRUE;
            DwmSetWindowAttribute(hwnd, 20, &use_dark_mode, sizeof(use_dark_mode));
        }
    }
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
