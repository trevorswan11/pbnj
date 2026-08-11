#pragma once

#include <string>
#include <string_view>

#include <ankerl/unordered_dense.h>
#include <gsl/span>
#include <sokol.h>
#include <stdx/hash.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "support/error.hh"
#include "ui/assets/texture.hh"

namespace pbnj::ui::assets {

class texture_cache {
  public:
    template <typename V>
    using string_map_t = ankerl::unordered_dense::
        map<std::string, V, stdx::string_transparent_hash, stdx::string_transparent_eq>;

  public:
    texture_cache()  = default;
    ~texture_cache() = default;
    MAKE_MOVE_ONLY(texture_cache);

    [[nodiscard]] auto get_or_load_svg(std::string_view name, gsl::span<const char> data)
        -> result<ImTextureID>;

  private:
    string_map_t<texture> svg_cache_;
};

} // namespace pbnj::ui::assets
