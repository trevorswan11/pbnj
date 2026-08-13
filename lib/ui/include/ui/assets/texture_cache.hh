#pragma once

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
#include "ui/assets/texture.hh"
#include "ui/core/compact_types.hh" // IWYU pragma: keep

namespace pbnj::ui::assets {

enum class core_icon_id_t : u8 {
    CHEVRON_LEFT,  // Navbar back button
    CHEVRON_RIGHT, // Navbar forward button
    CLOSE,         // 'x' close button
    HOME,          // Navbar home button
    MENU,          // Navbar menu button (3 horizontal lines)
    PAUSE,         // Pause button
    PLAY,          // Play button
    SEARCH,        // Magnifying glass
    SKIP_BACK,     // Skip back button
    SKIP_FORWARD,  // Skip forward button
    USER_ROUND,    // Unknown album cover
    VOLUME_ZERO,   // Lowest volume
    VOLUME_ONE,    // Medium volume
    VOLUME_TWO,    // Highest volume
    MUTED_VOLUME,  // Volume fully muted
};

class texture_cache {
  public:
    template <typename V>
    using string_map_t = ankerl::unordered_dense::
        map<std::string, V, stdx::string_transparent_hash, stdx::string_transparent_eq>;
    using core_icon_map_t = stdx::fixed::enum_map<core_icon_id_t, stdx::option<texture>>;

  public:
    texture_cache() = default;
    ~texture_cache() { clear(); };
    MAKE_MOVE_ONLY(texture_cache);

    [[nodiscard]] auto get_or_load_svg(std::string_view name, gsl::span<const char> data)
        -> result<ImTextureID>;
    [[nodiscard]] auto get_core_icon(core_icon_id_t id) -> ImTextureID;
    [[nodiscard]] auto fallback_texture() noexcept -> ImTextureID;

    auto clear() noexcept -> void;

  private:
    [[nodiscard]] auto ensure_sampler() noexcept -> result<void>;
    [[nodiscard]] auto ensure_fallback_texture() noexcept -> result<void>;

    [[nodiscard]] auto load_svg_internal(gsl::span<const char> data) -> result<texture>;
    [[nodiscard]] auto create_texture(i32 width, i32 height, gsl::span<const u8> raw_bytes) noexcept
        -> result<texture>;

  private:
    stdx::option<sg_sampler> linear_sampler_;
    string_map_t<texture>    svg_cache_;
    core_icon_map_t          core_icons_;
    texture                  fallback_texture_;
};

} // namespace pbnj::ui::assets
