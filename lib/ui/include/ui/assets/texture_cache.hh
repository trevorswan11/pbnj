#pragma once

#include <array>
#include <string>
#include <string_view>

#include <ankerl/unordered_dense.h>
#include <gsl/span>
#include <sokol.h>
#include <stdx/fixed/enum_map.hh>
#include <stdx/hash.hh>
#include <stdx/option.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "support/error.hh"
#include "ui/assets/icons.hh"
#include "ui/assets/texture.hh"
#include "ui/core/compact_types.hh" // IWYU pragma: keep

namespace pbnj::ui::assets {

class texture_cache {
  public:
    template <typename V>
    using string_map_t = ankerl::unordered_dense::
        map<std::string, V, stdx::string_transparent_hash, stdx::string_transparent_eq>;
    using core_icon_map_t = stdx::fixed::enum_map<icon, stdx::option<texture>>;

  public:
    texture_cache() = default;
    ~texture_cache() { clear(); };
    MAKE_MOVE_ONLY(texture_cache);

    [[nodiscard]] auto get_or_load_svg(std::string_view name, gsl::span<const char> data)
        -> result<ImTextureID>;
    [[nodiscard]] auto get_icon(icon ic) -> ImTextureID;
    [[nodiscard]] auto fallback_texture() noexcept -> ImTextureID;

    auto clear() noexcept -> void;

  private:
    // 2x2 RGBA8 Magenta
    static constexpr auto fallback_pixels =
        std::to_array<u8>({255, 0, 255, 255, 24, 24, 24, 255, 24, 24, 24, 255, 255, 0, 255, 255});

  private:
    [[nodiscard]] auto ensure_sampler() noexcept -> result<void>;
    [[nodiscard]] auto ensure_fallback_texture() noexcept -> result<void>;

    [[nodiscard]] auto load_svg_internal(gsl::span<const char> data) -> result<texture>;
    [[nodiscard]] auto create_texture(i32 width, i32 height, gsl::span<const u8> data) noexcept
        -> result<texture>;

  private:
    stdx::option<sg_sampler> linear_sampler_;
    string_map_t<texture>    svg_cache_;
    core_icon_map_t          core_icons_;
    texture                  fallback_texture_;
};

} // namespace pbnj::ui::assets
