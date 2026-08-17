#include "ui/theme/fonts.hh"

#include <gsl/pointers>
#include <gsl/span>
#include <imgui.hh>
#include <stdx/option.hh>
#include <stdx/types.hh>

#include "ui/assets/inter_font.hh"

namespace pbnj::ui::theme {

auto font_manager::init(f32 dpi_scale) noexcept -> void {
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    cfg.OversampleH          = 2;
    cfg.OversampleV          = 2;
    cfg.PixelSnapH           = true;

    reg_body_       = *add_font(assets::inter_regular_18pt, 18.0f, dpi_scale, cfg);
    bold_body_      = *add_font(assets::inter_bold_18pt, 15.0f, dpi_scale, cfg);
    italic_body_    = *add_font(assets::inter_italic_18pt, 18.0f, dpi_scale, cfg);
    caption_        = *add_font(assets::inter_regular_18pt, 12.0f, dpi_scale, cfg);
    italic_caption_ = *add_font(assets::inter_italic_18pt, 12.0f, dpi_scale, cfg);
    subheading_     = *add_font(assets::inter_bold_24pt, 20.0f, dpi_scale, cfg);
    heading_        = *add_font(assets::inter_bold_28pt, 26.0f, dpi_scale, cfg);
    title_          = *add_font(assets::inter_bold_28pt, 34.0f, dpi_scale, cfg);

    auto& io       = ImGui::GetIO();
    io.FontDefault = reg_body_.get();
}

auto font_manager::get(font_id id) const noexcept -> stdx::option<ImFont&> {
    switch (id) {
    case font_id::BODY:           return reg_body_;
    case font_id::BOLD_BODY:      return bold_body_;
    case font_id::ITALIC_BODY:    return italic_body_;
    case font_id::CAPTION:        return caption_;
    case font_id::ITALIC_CAPTION: return italic_caption_;
    case font_id::SUBHEADING:     return subheading_;
    case font_id::HEADING:        return heading_;
    case font_id::TITLE:          return title_;
    default:                      return stdx::none;
    }
}

auto font_manager::add_font(gsl::span<unsigned char> raw_ttf,
                            f32                      font_size,
                            f32                      dpi_scale,
                            const ImFontConfig&      cfg) noexcept -> gsl::not_null<ImFont*> {
    auto& io   = ImGui::GetIO();
    auto* font = io.Fonts->AddFontFromMemoryTTF(
        raw_ttf.data(), static_cast<i32>(raw_ttf.size()), font_size * dpi_scale, &cfg);
    if (dpi_scale > 0.0f) { font->Scale = 1.0f / dpi_scale; }
    return font;
}

font_scope::font_scope(stdx::option<ImFont&> font) noexcept {
    if (font) {
        ImGui::PushFont(font.get());
        pushed_ = true;
    }
}

font_scope::~font_scope() {
    if (pushed_) { ImGui::PopFont(); }
}

} // namespace pbnj::ui::theme
