#include "ui/assets/texture_cache.hh"

#include <array>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>

#include <SDL3/SDL_gpu.h>
#include <gsl/span>
#include <gsl/util>
#include <imgui.hh>
#include <nanosvg.h>
#include <nanosvgrast.h>
#include <stdx/assert.hh>
#include <stdx/memory.hh>
#include <stdx/option.hh>
#include <stdx/result.hh>
#include <stdx/types.hh>

#include "support/error.hh"
#include "ui/assets/lucide.hh"
#include "ui/assets/texture.hh"

namespace pbnj::ui::assets {

namespace {

// 2x2 RGBA8 Magenta
constexpr auto fallback_pixels =
    std::to_array<u8>({255, 0, 255, 255, 24, 24, 24, 255, 24, 24, 24, 255, 255, 0, 255, 255});

} // namespace

auto texture_cache::get_or_load_svg(std::string_view name, gsl::span<const char> data)
    -> result<ImTextureID> {
    if (auto it = svg_cache_.find(name); it != svg_cache_.end()) { return *it->second.imgui_id; }
    auto [it, _] = svg_cache_.emplace(name, TRY(load_svg_internal(data)));
    return *it->second.imgui_id;
}

auto texture_cache::get_core_icon(core_icon_id_t id) -> ImTextureID {
    auto& tex = core_icons_[id];
    if (tex) { return *tex->imgui_id; }

    auto data = [id] -> gsl::span<const char> {
        switch (id) {
        case core_icon_id_t::CHEVRON_LEFT:  return assets::chevron_left;
        case core_icon_id_t::CHEVRON_RIGHT: return assets::chevron_right;
        case core_icon_id_t::CLOSE:         return assets::close;
        case core_icon_id_t::HOME:          return assets::home;
        case core_icon_id_t::MENU:          return assets::menu;
        case core_icon_id_t::PAUSE:         return assets::pause;
        case core_icon_id_t::PLAY:          return assets::play;
        case core_icon_id_t::SEARCH:        return assets::search;
        case core_icon_id_t::SKIP_BACK:     return assets::skip_back;
        case core_icon_id_t::SKIP_FORWARD:  return assets::skip_forward;
        case core_icon_id_t::USER_ROUND:    return assets::user_round;
        case core_icon_id_t::VOLUME_ZERO:   return assets::volume_0;
        case core_icon_id_t::VOLUME_ONE:    return assets::volume_1;
        case core_icon_id_t::VOLUME_TWO:    return assets::volume_2;
        case core_icon_id_t::MUTED_VOLUME:  return assets::volume_x;
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
    if (device_ && linear_sampler_) {
        SDL_ReleaseGPUSampler(device_, linear_sampler_);
        linear_sampler_ = nullptr;
    }
    svg_cache_.clear();
    for (auto& core_icon : core_icons_) { core_icon.reset(); }
    fallback_texture_.release();
    device_ = nullptr;
}

auto texture_cache::ensure_fallback_texture() noexcept -> result<void> {
    if (fallback_texture_.imgui_id) { return {}; }
    fallback_texture_ = TRY(create_texture(2, 2, fallback_pixels));
    return {};
}

auto texture_cache::ensure_sampler() noexcept -> result<void> {
    if (!device_) { return stdx::err{error::GPU_DEVICE_NOT_FOUND}; }
    if (!linear_sampler_) {
        SDL_GPUSamplerCreateInfo sampler_info{};
        sampler_info.min_filter     = SDL_GPU_FILTER_LINEAR;
        sampler_info.mag_filter     = SDL_GPU_FILTER_LINEAR;
        sampler_info.mipmap_mode    = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
        sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
        sampler_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

        auto* smp = SDL_CreateGPUSampler(device_, &sampler_info);
        if (!smp) { return stdx::err{error::GPU_SAMPLER_CREATE_FAILED}; }
        linear_sampler_ = smp;
    }
    return {};
}

auto texture_cache::load_svg_internal(gsl::span<const char> data) -> result<texture> {
    std::string data_copy{data.data(), data.size()};
    auto*       svg = nsvgParse(data_copy.data(), "px", 96.0f);
    if (!svg) { return stdx::err{error::SVG_PARSE_FAILED}; }
    const auto svg_delete = gsl::finally([svg] { nsvgDelete(svg); });

    const auto render_w   = static_cast<i32>(svg->width * dpi_scale_);
    const auto render_h   = static_cast<i32>(svg->height * dpi_scale_);
    const auto image_size = static_cast<usize>(render_w * render_h * 4);
    auto       pixels     = stdx::make_box<u8[]>(image_size);

    auto* rast = nsvgCreateRasterizer();
    if (!rast) { return stdx::err{error::SVG_PARSE_FAILED}; }
    const auto rast_delete = gsl::finally([rast] { nsvgDeleteRasterizer(rast); });
    nsvgRasterize(rast, svg, 0, 0, dpi_scale_, pixels.get(), render_w, render_h, render_w * 4);

    for (usize i = 0; i < image_size; i += 4) {
        pixels[i + 0] = 255;
        pixels[i + 1] = 255;
        pixels[i + 2] = 255;
    }

    return create_texture(static_cast<u32>(render_w),
                          static_cast<u32>(render_h),
                          gsl::span{pixels.get(), image_size});
}

auto texture_cache::create_texture(u32 width, u32 height, gsl::span<const u8> raw_bytes) noexcept
    -> result<texture> {
    if (!device_) { return stdx::err{error::GPU_DEVICE_NOT_FOUND}; }

    SDL_GPUTextureCreateInfo texture_info{};
    texture_info.type                 = SDL_GPU_TEXTURETYPE_2D;
    texture_info.format               = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    texture_info.usage                = SDL_GPU_TEXTUREUSAGE_SAMPLER;
    texture_info.width                = width;
    texture_info.height               = height;
    texture_info.layer_count_or_depth = 1;
    texture_info.num_levels           = 1;

    auto* gpu_tex = SDL_CreateGPUTexture(device_, &texture_info);
    if (!gpu_tex) { return stdx::err{error::GPU_TEXTURE_ALLOC_FAILED}; }

    SDL_GPUTransferBufferCreateInfo transfer_info{};
    transfer_info.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_info.size  = static_cast<u32>(raw_bytes.size());

    auto* transfer_buf = SDL_CreateGPUTransferBuffer(device_, &transfer_info);
    if (!transfer_buf) {
        SDL_ReleaseGPUTexture(device_, gpu_tex);
        return stdx::err{error::GPU_TRANSFER_BUFFER_FAILED};
    }
    const auto cleanup_transfer =
        gsl::finally([this, transfer_buf] { SDL_ReleaseGPUTransferBuffer(device_, transfer_buf); });

    auto* map_ptr = SDL_MapGPUTransferBuffer(device_, transfer_buf, false);
    if (!map_ptr) {
        SDL_ReleaseGPUTexture(device_, gpu_tex);
        return stdx::err{error::GPU_TRANSFER_BUFFER_FAILED};
    }
    std::memcpy(map_ptr, raw_bytes.data(), raw_bytes.size());
    SDL_UnmapGPUTransferBuffer(device_, transfer_buf);

    auto* cmd = SDL_AcquireGPUCommandBuffer(device_);
    if (!cmd) {
        SDL_ReleaseGPUTexture(device_, gpu_tex);
        return stdx::err{error::GPU_TRANSFER_BUFFER_FAILED};
    }

    {
        auto*      copy_pass = SDL_BeginGPUCopyPass(cmd);
        const auto end_pass  = gsl::finally([copy_pass] { SDL_EndGPUCopyPass(copy_pass); });
        SDL_GPUTextureTransferInfo src_info{};
        src_info.transfer_buffer = transfer_buf;
        src_info.offset          = 0;
        src_info.pixels_per_row  = static_cast<u32>(width);
        src_info.rows_per_layer  = static_cast<u32>(height);

        SDL_GPUTextureRegion dst_region{};
        dst_region.texture = gpu_tex;
        dst_region.w       = static_cast<u32>(width);
        dst_region.h       = static_cast<u32>(height);
        dst_region.d       = 1;

        SDL_UploadToGPUTexture(copy_pass, &src_info, &dst_region, false);
    }
    SDL_SubmitGPUCommandBuffer(cmd);

    TRY(ensure_sampler());
    texture tex;
    tex.device   = device_;
    tex.handle   = gpu_tex;
    tex.sampler  = linear_sampler_;
    tex.imgui_id = reinterpret_cast<ImTextureID>(gpu_tex);

    return tex;
}

} // namespace pbnj::ui::assets
