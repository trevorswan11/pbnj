#include "ui/assets/texture_cache.hh"

#include <string>
#include <string_view>
#include <utility>

#include <gsl/span>
#include <gsl/util>
#include <nanosvg.h>
#include <nanosvgrast.h>
#include <sokol.h>
#include <stdx/memory.hh>
#include <stdx/option.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "support/error.hh"
#include "ui/assets/texture.hh"

namespace pbnj::ui::assets {

auto texture_cache::get_or_load_svg(std::string_view name, gsl::span<const char> data)
    -> result<ImTextureID> {
    if (auto it = svg_cache_.find(name); it != svg_cache_.end()) { return *it->second.imgui_id; }

    std::string data_copy{data.data(), data.size()};
    auto*       svg = nsvgParse(data_copy.data(), "px", 96.0f);
    if (!svg) { return stdx::err{error::SVG_PARSE_FAILED}; }
    const auto svg_delete = gsl::finally([svg] { nsvgDelete(svg); });

    const auto dpi_scale  = sapp_dpi_scale();
    const auto render_w   = static_cast<i32>(svg->width * dpi_scale);
    const auto render_h   = static_cast<i32>(svg->height * dpi_scale);
    const auto image_size = static_cast<usize>(render_w * render_h * 4);
    auto       pixels     = stdx::make_box<unsigned char[]>(image_size);

    auto* rast = nsvgCreateRasterizer();
    if (!rast) { return stdx::err{error::SVG_PARSE_FAILED}; }
    const auto rast_delete = gsl::finally([rast] { nsvgDeleteRasterizer(rast); });
    nsvgRasterize(rast, svg, 0, 0, dpi_scale, pixels.get(), render_w, render_h, render_w * 4);

    sg_image_desc img_desc           = {};
    img_desc.width                   = render_w;
    img_desc.height                  = render_h;
    img_desc.pixel_format            = SG_PIXELFORMAT_RGBA8;
    img_desc.num_mipmaps             = 1;
    img_desc.data.mip_levels[0].ptr  = pixels.get();
    img_desc.data.mip_levels[0].size = image_size;

    texture tex;
    tex.image = sg_make_image(img_desc);
    if (!tex.image) { return stdx::err{error::SOKOL_IMG_ALLOC_FAILED}; }

    sg_view_desc view_desc  = {};
    view_desc.texture.image = *tex.image;
    tex.view                = sg_make_view(&view_desc);
    if (!tex.view) { return stdx::err{error::SOKOL_IMG_VIEW_ALLOC_FAILED}; }

    TRY(ensure_sampler());
    tex.imgui_id = simgui_imtextureid_with_sampler(*tex.view, *linear_sampler_);
    if (!tex.imgui_id) { return stdx::err{error::INVALID_IMGUI_TEXTURE}; }

    auto [it, _] = svg_cache_.emplace(name, std::move(tex));
    return *it->second.imgui_id;
}

auto texture_cache::clear() noexcept -> void {
    if (sg_isvalid() && linear_sampler_) { sg_destroy_sampler(linear_sampler_.take()); }
    svg_cache_.clear();
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

} // namespace pbnj::ui::assets
