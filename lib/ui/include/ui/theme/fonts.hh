#pragma once

#include <gsl/pointers>
#include <gsl/span>
#include <stdx/option.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

struct ImFont;
struct ImFontConfig;

namespace pbnj::ui::theme {

enum class font_id : u8 {
    BODY,
    BOLD_BODY,
    ITALIC_BODY,
    CAPTION,
    ITALIC_CAPTION,
    SUBHEADING,
    HEADING,
    TITLE,
};

class font_manager {
  public:
    auto               init(f32 dpi_scale) noexcept -> void;
    [[nodiscard]] auto get(font_id id) const noexcept -> stdx::option<ImFont&>;

  private:
    [[nodiscard]] auto add_font(gsl::span<unsigned char> raw_ttf,
                                f32                      font_size,
                                f32                      dpi_scale,
                                const ImFontConfig&      cfg) noexcept -> gsl::not_null<ImFont*>;

  private:
    stdx::option<ImFont&> reg_body_;
    stdx::option<ImFont&> caption_;
    stdx::option<ImFont&> bold_body_;
    stdx::option<ImFont&> italic_body_;
    stdx::option<ImFont&> italic_caption_;
    stdx::option<ImFont&> subheading_;
    stdx::option<ImFont&> heading_;
    stdx::option<ImFont&> title_;
};

class font_scope {
  public:
    explicit font_scope(stdx::option<ImFont&> font) noexcept;
    font_scope(const font_manager& manager, font_id id) noexcept : font_scope{manager.get(id)} {}
    ~font_scope();
    MAKE_MOVE_ONLY(font_scope);

    explicit operator bool() const noexcept { return pushed_; }

  private:
    bool pushed_{false};
};

} // namespace pbnj::ui::theme
