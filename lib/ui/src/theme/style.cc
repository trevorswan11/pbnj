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
#include <stdx/utility.hh>

namespace pbnj::ui::theme {

namespace colors {

constexpr ImVec4 transparent{0.0f, 0.0f, 0.0f, 0.0f};
constexpr ImVec4 black{0.0f, 0.0f, 0.0f, 1.0f};
constexpr ImVec4 white{1.0f, 1.0f, 1.0f, 1.0f};

// Dark theme palette
constexpr ImVec4 dark_grey{0.07f, 0.07f, 0.08f, 1.0f};
constexpr ImVec4 light_dark_grey{0.11f, 0.11f, 0.13f, 1.0f};
constexpr ImVec4 grey{0.14f, 0.14f, 0.16f, 1.0f};
constexpr ImVec4 dark_light_grey{0.18f, 0.18f, 0.22f, 1.0f};
constexpr ImVec4 light_grey{0.24f, 0.24f, 0.28f, 1.0f};
constexpr ImVec4 dark_border{0.20f, 0.20f, 0.24f, 0.60f};
constexpr ImVec4 dark_text{0.95f, 0.95f, 0.96f, 1.0f};
constexpr ImVec4 dark_text_disabled{0.55f, 0.55f, 0.60f, 1.0f};
constexpr ImVec4 dark_table_row_alt{0.14f, 0.14f, 0.16f, 0.50f};
constexpr ImVec4 dark_text_selected_bg{0.11f, 0.73f, 0.33f, 0.35f};

// Light theme palette
constexpr ImVec4 light_bg{0.96f, 0.96f, 0.97f, 1.0f};
constexpr ImVec4 light_surface{0.99f, 0.99f, 1.00f, 1.0f};
constexpr ImVec4 light_popup{1.00f, 1.00f, 1.00f, 1.0f};
constexpr ImVec4 light_border{0.88f, 0.89f, 0.91f, 1.0f};
constexpr ImVec4 light_header{0.92f, 0.93f, 0.95f, 1.0f};
constexpr ImVec4 light_header_hover{0.87f, 0.88f, 0.91f, 1.0f};
constexpr ImVec4 light_button{0.92f, 0.93f, 0.95f, 1.0f};
constexpr ImVec4 light_button_hover{0.86f, 0.87f, 0.90f, 1.0f};
constexpr ImVec4 light_button_active{0.80f, 0.82f, 0.85f, 1.0f};
constexpr ImVec4 light_text{0.0f, 0.0f, 0.0f, 1.0f};
constexpr ImVec4 light_text_disabled{0.45f, 0.47f, 0.52f, 1.0f};
constexpr ImVec4 light_text_selected_bg{0.11f, 0.73f, 0.33f, 0.25f};

// Shared brand / accent colors
constexpr ImVec4 dark_light_green{0.11f, 0.73f, 0.33f, 1.0f};
constexpr ImVec4 light_green{0.15f, 0.85f, 0.38f, 1.0f};
constexpr ImVec4 emerald_green{0.06f, 0.65f, 0.38f, 1.0f};

// Interaction states
constexpr ImVec4 ghost_hover{1.0f, 1.0f, 1.0f, 0.08f};
constexpr ImVec4 ghost_active{1.0f, 1.0f, 1.0f, 0.16f};
constexpr ImVec4 ghost_hover_light{0.0f, 0.0f, 0.0f, 0.06f};
constexpr ImVec4 ghost_active_light{0.0f, 0.0f, 0.0f, 0.12f};

// Input fields
constexpr ImVec4 frame_bg{0.14f, 0.14f, 0.14f, 1.0f};
constexpr ImVec4 frame_bg_hovered{0.18f, 0.18f, 0.18f, 1.0f};
constexpr ImVec4 frame_bg_active{0.22f, 0.22f, 0.22f, 1.0f};

constexpr ImVec4 frame_bg_light{0.92f, 0.93f, 0.95f, 1.0f};
constexpr ImVec4 frame_bg_light_hovered{0.87f, 0.88f, 0.91f, 1.0f};
constexpr ImVec4 frame_bg_light_active{0.82f, 0.84f, 0.87f, 1.0f};

constexpr ImVec4 icon_active{1.0f, 1.0f, 1.0f, 1.0f};
constexpr ImVec4 icon_idle{0.71f, 0.71f, 0.71f, 0.78f};

constexpr ImVec4 icon_active_light{0.0f, 0.0f, 0.0f, 1.0f};
constexpr ImVec4 icon_idle_light{0.35f, 0.35f, 0.38f, 0.85f};

} // namespace colors

// cppcheck-suppress-begin [unreadVariable, constParameterReference, constParameterPointer]
namespace {

auto set_window_mode(SDL_Window* window, mode_t mode) -> void {
    DISCARD(window);
    DISCARD(mode);
#ifdef _WIN32
    if (window) {
        BOOL  use_dark_mode = mode == mode_t::DARK;
        auto* hwnd          = static_cast<HWND>(SDL_GetPointerProperty(
            SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));
        if (hwnd) { DwmSetWindowAttribute(hwnd, 20, &use_dark_mode, sizeof(use_dark_mode)); }
    }
#endif
}

auto apply_dark_mode(SDL_Window* window, ImGuiStyle& style) noexcept -> void {
    set_window_mode(window, mode_t::DARK);

    gsl::span style_colors                      = style.Colors;
    style_colors[ImGuiCol_Text]                 = colors::dark_text;
    style_colors[ImGuiCol_TextDisabled]         = colors::dark_text_disabled;
    style_colors[ImGuiCol_WindowBg]             = colors::dark_grey;
    style_colors[ImGuiCol_ChildBg]              = colors::light_dark_grey;
    style_colors[ImGuiCol_PopupBg]              = colors::grey;
    style_colors[ImGuiCol_Border]               = colors::dark_border;
    style_colors[ImGuiCol_BorderShadow]         = colors::transparent;
    style_colors[ImGuiCol_FrameBg]              = colors::frame_bg;
    style_colors[ImGuiCol_FrameBgHovered]       = colors::frame_bg_hovered;
    style_colors[ImGuiCol_FrameBgActive]        = colors::frame_bg_active;
    style_colors[ImGuiCol_TitleBg]              = colors::dark_grey;
    style_colors[ImGuiCol_TitleBgActive]        = colors::dark_grey;
    style_colors[ImGuiCol_TitleBgCollapsed]     = colors::dark_grey;
    style_colors[ImGuiCol_MenuBarBg]            = colors::dark_grey;
    style_colors[ImGuiCol_ScrollbarBg]          = colors::transparent;
    style_colors[ImGuiCol_ScrollbarGrab]        = colors::dark_light_grey;
    style_colors[ImGuiCol_ScrollbarGrabHovered] = colors::light_grey;
    style_colors[ImGuiCol_ScrollbarGrabActive]  = colors::dark_text_disabled;
    style_colors[ImGuiCol_CheckMark]            = colors::light_green;
    style_colors[ImGuiCol_SliderGrab]           = colors::dark_light_green;
    style_colors[ImGuiCol_SliderGrabActive]     = colors::light_green;
    style_colors[ImGuiCol_Button]               = colors::dark_light_grey;
    style_colors[ImGuiCol_ButtonHovered]        = colors::light_grey;
    style_colors[ImGuiCol_ButtonActive]         = colors::dark_text_disabled;
    style_colors[ImGuiCol_Header]               = colors::dark_light_grey;
    style_colors[ImGuiCol_HeaderHovered]        = colors::light_grey;
    style_colors[ImGuiCol_HeaderActive]         = colors::dark_text_disabled;
    style_colors[ImGuiCol_Separator]            = colors::dark_border;
    style_colors[ImGuiCol_SeparatorHovered]     = colors::dark_light_green;
    style_colors[ImGuiCol_SeparatorActive]      = colors::light_green;
    style_colors[ImGuiCol_ResizeGrip]           = colors::transparent;
    style_colors[ImGuiCol_ResizeGripHovered]    = colors::dark_light_grey;
    style_colors[ImGuiCol_ResizeGripActive]     = colors::light_grey;
    style_colors[ImGuiCol_Tab]                  = colors::dark_light_grey;
    style_colors[ImGuiCol_TabHovered]           = colors::light_grey;
    style_colors[ImGuiCol_TabSelected]          = colors::light_dark_grey;
    style_colors[ImGuiCol_TabDimmed]            = colors::dark_grey;
    style_colors[ImGuiCol_TabDimmedSelected]    = colors::light_dark_grey;
    style_colors[ImGuiCol_TableHeaderBg]        = colors::dark_light_grey;
    style_colors[ImGuiCol_TableBorderStrong]    = colors::dark_border;
    style_colors[ImGuiCol_TableBorderLight]     = colors::dark_border;
    style_colors[ImGuiCol_TableRowBg]           = colors::transparent;
    style_colors[ImGuiCol_TableRowBgAlt]        = colors::dark_table_row_alt;
    style_colors[ImGuiCol_TextSelectedBg]       = colors::dark_text_selected_bg;
    style_colors[ImGuiCol_NavHighlight]         = colors::light_green;
    style_colors[ImGuiCol_InputTextCursor]      = colors::white;
}

auto apply_light_mode(SDL_Window* window, ImGuiStyle& style) noexcept -> void {
    set_window_mode(window, mode_t::LIGHT);

    gsl::span style_colors                      = style.Colors;
    style_colors[ImGuiCol_Text]                 = colors::light_text;
    style_colors[ImGuiCol_TextDisabled]         = colors::light_text_disabled;
    style_colors[ImGuiCol_WindowBg]             = colors::light_bg;
    style_colors[ImGuiCol_ChildBg]              = colors::light_surface;
    style_colors[ImGuiCol_PopupBg]              = colors::light_popup;
    style_colors[ImGuiCol_Border]               = colors::light_border;
    style_colors[ImGuiCol_BorderShadow]         = colors::transparent;
    style_colors[ImGuiCol_FrameBg]              = colors::frame_bg_light;
    style_colors[ImGuiCol_FrameBgHovered]       = colors::frame_bg_light_hovered;
    style_colors[ImGuiCol_FrameBgActive]        = colors::frame_bg_light_active;
    style_colors[ImGuiCol_TitleBg]              = colors::light_bg;
    style_colors[ImGuiCol_TitleBgActive]        = colors::light_bg;
    style_colors[ImGuiCol_TitleBgCollapsed]     = colors::light_bg;
    style_colors[ImGuiCol_MenuBarBg]            = colors::light_bg;
    style_colors[ImGuiCol_ScrollbarBg]          = colors::transparent;
    style_colors[ImGuiCol_ScrollbarGrab]        = colors::light_border;
    style_colors[ImGuiCol_ScrollbarGrabHovered] = colors::light_text_disabled;
    style_colors[ImGuiCol_ScrollbarGrabActive]  = colors::light_text;
    style_colors[ImGuiCol_CheckMark]            = colors::emerald_green;
    style_colors[ImGuiCol_SliderGrab]           = colors::dark_light_green;
    style_colors[ImGuiCol_SliderGrabActive]     = colors::emerald_green;
    style_colors[ImGuiCol_Button]               = colors::light_button;
    style_colors[ImGuiCol_ButtonHovered]        = colors::light_button_hover;
    style_colors[ImGuiCol_ButtonActive]         = colors::light_button_active;
    style_colors[ImGuiCol_Header]               = colors::light_header;
    style_colors[ImGuiCol_HeaderHovered]        = colors::light_header_hover;
    style_colors[ImGuiCol_HeaderActive]         = colors::light_button_active;
    style_colors[ImGuiCol_Separator]            = colors::light_border;
    style_colors[ImGuiCol_SeparatorHovered]     = colors::dark_light_green;
    style_colors[ImGuiCol_SeparatorActive]      = colors::light_green;
    style_colors[ImGuiCol_ResizeGrip]           = colors::transparent;
    style_colors[ImGuiCol_ResizeGripHovered]    = colors::light_border;
    style_colors[ImGuiCol_ResizeGripActive]     = colors::light_text_disabled;
    style_colors[ImGuiCol_Tab]                  = colors::light_header;
    style_colors[ImGuiCol_TabHovered]           = colors::light_header_hover;
    style_colors[ImGuiCol_TabSelected]          = colors::light_surface;
    style_colors[ImGuiCol_TabDimmed]            = colors::light_header;
    style_colors[ImGuiCol_TabDimmedSelected]    = colors::light_surface;
    style_colors[ImGuiCol_TableHeaderBg]        = colors::light_header;
    style_colors[ImGuiCol_TableBorderStrong]    = colors::light_border;
    style_colors[ImGuiCol_TableBorderLight]     = colors::light_border;
    style_colors[ImGuiCol_TableRowBg]           = colors::transparent;
    style_colors[ImGuiCol_TableRowBgAlt]        = colors::frame_bg_light;
    style_colors[ImGuiCol_TextSelectedBg]       = colors::light_text_selected_bg;
    style_colors[ImGuiCol_NavHighlight]         = colors::dark_light_green;
    style_colors[ImGuiCol_InputTextCursor]      = colors::black;
}

} // namespace
// cppcheck-suppress-end [unreadVariable, constParameterReference, constParameterPointer]

auto style_manager::set_mode(SDL_Window* window, mode_t mode) noexcept -> void {
    auto& style             = ImGui::GetStyle();
    style.WindowRounding    = 0;
    style.ChildRounding     = 8;
    style.FrameRounding     = 6;
    style.PopupRounding     = 8;
    style.ScrollbarRounding = 12;
    style.GrabRounding      = 6;
    style.TabRounding       = 6;
    style.WindowPadding     = {12, 12};
    style.ItemSpacing       = {8, 8};
    style.ItemInnerSpacing  = {6, 6};
    style.ScrollbarSize     = 12;
    style.GrabMinSize       = 10;
    style.WindowBorderSize  = 0.0f;
    style.ChildBorderSize   = 0.0f;
    style.PopupBorderSize   = 1.0f;
    style.FrameBorderSize   = 0.0f;

    if (mode == mode_t::DARK) {
        apply_dark_mode(window, style);
    } else {
        apply_light_mode(window, style);
    }
    mode_ = mode;
}

auto style_manager::window_bg() const noexcept -> ImVec4 {
    return is_dark() ? colors::dark_grey : colors::light_bg;
}

auto style_manager::clear_color() const noexcept -> ImVec4 { return window_bg(); }

auto style_manager::child_bg() const noexcept -> ImVec4 {
    return is_dark() ? colors::light_dark_grey : colors::light_surface;
}

auto style_manager::popup_bg() const noexcept -> ImVec4 {
    return is_dark() ? colors::grey : colors::light_popup;
}

auto style_manager::border() const noexcept -> ImVec4 {
    return is_dark() ? colors::dark_border : colors::light_border;
}

auto style_manager::text() const noexcept -> ImVec4 {
    return is_dark() ? colors::dark_text : colors::light_text;
}

auto style_manager::text_disabled() const noexcept -> ImVec4 {
    return is_dark() ? colors::dark_text_disabled : colors::light_text_disabled;
}

auto style_manager::button() const noexcept -> ImVec4 {
    return is_dark() ? colors::dark_light_grey : colors::light_button;
}

auto style_manager::button_hovered() const noexcept -> ImVec4 {
    return is_dark() ? colors::light_grey : colors::light_button_hover;
}

auto style_manager::button_active() const noexcept -> ImVec4 {
    return is_dark() ? colors::dark_text_disabled : colors::light_button_active;
}

auto style_manager::ghost_hover() const noexcept -> ImVec4 {
    return is_dark() ? colors::ghost_hover : colors::ghost_hover_light;
}

auto style_manager::ghost_active() const noexcept -> ImVec4 {
    return is_dark() ? colors::ghost_active : colors::ghost_active_light;
}

auto style_manager::frame_bg() const noexcept -> ImVec4 {
    return is_dark() ? colors::frame_bg : colors::frame_bg_light;
}

auto style_manager::frame_bg_hovered() const noexcept -> ImVec4 {
    return is_dark() ? colors::frame_bg_hovered : colors::frame_bg_light_hovered;
}

auto style_manager::frame_bg_active() const noexcept -> ImVec4 {
    return is_dark() ? colors::frame_bg_active : colors::frame_bg_light_active;
}

auto style_manager::icon_active() const noexcept -> ImVec4 {
    return is_dark() ? colors::icon_active : colors::icon_active_light;
}

auto style_manager::icon_idle() const noexcept -> ImVec4 {
    return is_dark() ? colors::icon_idle : colors::icon_idle_light;
}

auto style_manager::icon_tint() const noexcept -> ImVec4 {
    return is_dark() ? colors::white : colors::icon_active_light;
}

auto style_manager::slider_grab() const noexcept -> ImVec4 { return colors::dark_light_green; }

auto style_manager::slider_grab_active() const noexcept -> ImVec4 {
    return is_dark() ? colors::light_green : colors::emerald_green;
}

auto style_manager::transparent() const noexcept -> ImVec4 { return colors::transparent; }

auto style_manager::black() const noexcept -> ImVec4 { return colors::black; }

auto style_manager::white() const noexcept -> ImVec4 { return colors::white; }

} // namespace pbnj::ui::theme
