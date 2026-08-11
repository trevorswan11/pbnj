#include "ui/assets/texture_cache.hh"

#include <string>
#include <string_view>
#include <utility>

#include <gsl/span>
#include <nanosvg.h>
#include <nanosvgrast.h>
#include <sokol.h>
#include <stdx/memory.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "support/error.hh"
#include "ui/assets/texture.hh"

namespace pbnj::ui::assets {

auto texture_cache::get_or_load_svg(std::string_view name, gsl::span<const char> data)
    -> result<ImTextureID> {
    auto it = svg_cache_.try_emplace(name);
    if (!it.second) { return *it.first->second.imgui_id; }

    std::string data_copy = data.data();
    auto*       svg       = nsvgParse(data_copy.data(), "px", 96.0f);
    if (!svg) { return stdx::err{error::SVG_PARSE_FAILED}; }

    const auto dpi_scale = sapp_dpi_scale();
    const auto render_w  = static_cast<i32>(svg->width * dpi_scale);
    const auto render_h  = static_cast<i32>(svg->height * dpi_scale);
    auto pixels = stdx::make_box<unsigned char[]>(static_cast<usize>(render_w * render_h * 4));

    auto* rast = nsvgCreateRasterizer();
    nsvgRasterize(rast, svg, 0, 0, dpi_scale, pixels.get(), render_w, render_h, render_w * 4);
    nsvgDeleteRasterizer(rast);
    nsvgDelete(svg);

    sg_image_desc img_desc          = {};
    img_desc.width                  = render_w;
    img_desc.height                 = render_h;
    img_desc.pixel_format           = SG_PIXELFORMAT_RGBA8;
    img_desc.num_mipmaps            = 1;
    img_desc.data.mip_levels[0].ptr = pixels.get();

    texture tex;
    tex.image = sg_make_image(img_desc);
    if (!tex.image) { return stdx::err{error::SOKOL_IMG_ALLOC_FAILED}; }

    sg_view_desc view_desc  = {};
    view_desc.texture.image = *tex.image;
    tex.view                = sg_make_view(&view_desc);
    if (!tex.view) { return stdx::err{error::SOKOL_IMG_VIEW_ALLOC_FAILED}; }

    tex.imgui_id = simgui_imtextureid(*tex.view);
    if (!tex.imgui_id) { return stdx::err{error::INVALID_IMGUI_TEXTURE}; }

    it.first->second = std::move(tex);
    return *it.first->second.imgui_id;
}

} // namespace pbnj::ui::assets
