#include "ui/assets/texture_cache.hh"

#include <string>
#include <string_view>
#include <utility>

#include <gsl/span>
#include <gsl/util>
#include <nanosvg.h>
#include <nanosvgrast.h>
#include <sokol.h>
#include <stdx/assert.hh>
#include <stdx/memory.hh>
#include <stdx/option.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "support/error.hh"
#include "ui/assets/icons.hh"
#include "ui/assets/lucide.hh"
#include "ui/assets/texture.hh"

namespace pbnj::ui::assets {

auto texture_cache::get_or_load_svg(std::string_view name, gsl::span<const char> data)
    -> result<ImTextureID> {
    if (auto it = svg_cache_.find(name); it != svg_cache_.end()) { return *it->second.imgui_id; }
    auto [it, _] = svg_cache_.emplace(name, TRY(load_svg_internal(data)));
    return *it->second.imgui_id;
}

auto texture_cache::get_icon(icon ic) -> ImTextureID {
    auto& tex = core_icons_[ic];
    if (tex) { return *tex->imgui_id; }

    auto data = [ic] -> gsl::span<const char> {
        switch (ic) {
        case icon::CHEVRON_LEFT:  return assets::chevron_left;
        case icon::CHEVRON_RIGHT: return assets::chevron_right;
        }
        UNREACHABLE("Invalid core icon id");
    }();

    auto loaded = load_svg_internal(data);
    ASSERT(loaded && loaded->imgui_id, "Failed to load core icon texture");
    tex.emplace(std::move(*loaded));
    return *tex->imgui_id;
}

auto texture_cache::fallback_texture() noexcept -> ImTextureID {
    const auto res = ensure_fallback_texture();
    ASSERT(res && fallback_texture_.imgui_id, "Could not load fallback texture");
    return *fallback_texture_.imgui_id;
}

auto texture_cache::clear() noexcept -> void {
    if (sg_isvalid() && linear_sampler_) { sg_destroy_sampler(linear_sampler_.take()); }
    svg_cache_.clear();
    for (auto& core_icon : core_icons_) { core_icon.reset(); }
    fallback_texture_ = {};
}

auto texture_cache::ensure_fallback_texture() noexcept -> result<void> {
    if (fallback_texture_.imgui_id) { return {}; }
    fallback_texture_ = TRY(create_texture(2, 2, fallback_pixels));
    return {};
}

auto texture_cache::ensure_sampler() noexcept -> result<void> {
    if (!linear_sampler_ && sg_isvalid()) {
        sg_sampler_desc desc{};
        desc.min_filter = SG_FILTER_LINEAR;
        desc.mag_filter = SG_FILTER_LINEAR;
        desc.wrap_u     = SG_WRAP_CLAMP_TO_EDGE;
        desc.wrap_v     = SG_WRAP_CLAMP_TO_EDGE;

        stdx::option<sg_sampler> smp = sg_make_sampler(desc);
        if (!smp) { return stdx::err{error::SOKOL_SAMPLER_CREATE_FAILED}; }
        linear_sampler_ = smp;
    }
    return {};
}

auto texture_cache::load_svg_internal(gsl::span<const char> data) -> result<texture> {
    std::string data_copy{data.data(), data.size()};
    auto*       svg = nsvgParse(data_copy.data(), "px", 96.0f);
    if (!svg) { return stdx::err{error::SVG_PARSE_FAILED}; }
    const auto svg_delete = gsl::finally([svg] { nsvgDelete(svg); });

    const auto dpi_scale  = sapp_dpi_scale();
    const auto render_w   = static_cast<i32>(svg->width * dpi_scale);
    const auto render_h   = static_cast<i32>(svg->height * dpi_scale);
    const auto image_size = static_cast<usize>(render_w * render_h * 4);
    auto       pixels     = stdx::make_box<u8[]>(image_size);

    auto* rast = nsvgCreateRasterizer();
    if (!rast) { return stdx::err{error::SVG_PARSE_FAILED}; }
    const auto rast_delete = gsl::finally([rast] { nsvgDeleteRasterizer(rast); });
    nsvgRasterize(rast, svg, 0, 0, dpi_scale, pixels.get(), render_w, render_h, render_w * 4);

    return create_texture(render_w, render_h, gsl::span{pixels.get(), image_size});
}

auto texture_cache::create_texture(i32 width, i32 height, gsl::span<const u8> data) noexcept
    -> result<texture> {
    sg_image_desc img_desc{};
    img_desc.width                   = width;
    img_desc.height                  = height;
    img_desc.pixel_format            = SG_PIXELFORMAT_RGBA8;
    img_desc.num_mipmaps             = 1;
    img_desc.data.mip_levels[0].ptr  = data.data();
    img_desc.data.mip_levels[0].size = data.size();

    texture tex;
    tex.image = sg_make_image(img_desc);
    if (!tex.image) { return stdx::err{error::SOKOL_IMG_ALLOC_FAILED}; }

    sg_view_desc view_desc{};
    view_desc.texture.image = *tex.image;
    tex.view                = sg_make_view(&view_desc);
    if (!tex.view) { return stdx::err{error::SOKOL_IMG_VIEW_ALLOC_FAILED}; }

    TRY(ensure_sampler());
    tex.imgui_id = simgui_imtextureid_with_sampler(*tex.view, *linear_sampler_);
    if (!tex.imgui_id) { return stdx::err{error::INVALID_IMGUI_TEXTURE}; }

    return tex;
}

} // namespace pbnj::ui::assets
