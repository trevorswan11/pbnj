#include "ui/assets/texture_cache.hh"

#include <array>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>

#include <gsl/pointers>
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
#include <vulkan/vulkan_core.h>

#include "support/error.hh"
#include "ui/assets/lucide.hh"
#include "ui/assets/texture.hh"
#include "ui/core/vk_context.hh"

namespace pbnj::ui::assets {

namespace {

// 2x2 RGBA8 Magenta
constexpr auto fallback_pixels =
    std::to_array<u8>({255, 0, 255, 255, 24, 24, 24, 255, 24, 24, 24, 255, 255, 0, 255, 255});

} // namespace

auto texture_cache::init(gsl::not_null<vk_context*> vk_ctx, f32 dpi_scale) noexcept -> void {
    vk_ctx_.emplace(*vk_ctx);
    dpi_scale_ = dpi_scale;
}

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
    if (vk_ctx_ && vk_ctx_->device() && linear_sampler_) {
        vkDestroySampler(vk_ctx_->device(), linear_sampler_.take(), nullptr);
        linear_sampler_ = VK_NULL_HANDLE;
    }
    svg_cache_.clear();
    for (auto& core_icon : core_icons_) { core_icon.reset(); }
    fallback_texture_.release();
    vk_ctx_ = nullptr;
}

auto texture_cache::ensure_fallback_texture() noexcept -> result<void> {
    if (fallback_texture_.imgui_id) { return {}; }
    fallback_texture_ = TRY(create_texture(2, 2, fallback_pixels));
    return {};
}

auto texture_cache::ensure_sampler() noexcept -> result<void> {
    if (!vk_ctx_ || !vk_ctx_->device()) { return stdx::err{error::GPU_DEVICE_NOT_FOUND}; }
    if (!linear_sampler_) {
        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType         = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.minFilter     = VK_FILTER_LINEAR;
        sampler_info.magFilter     = VK_FILTER_LINEAR;
        sampler_info.mipmapMode    = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.addressModeU  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.addressModeV  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.addressModeW  = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.maxAnisotropy = 1.0f;

        if (vkCreateSampler(vk_ctx_->device(), &sampler_info, nullptr, linear_sampler_.raw()) !=
            VK_SUCCESS) {
            return stdx::err{error::GPU_SAMPLER_CREATE_FAILED};
        }
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
    if (!vk_ctx_ || !vk_ctx_->device() || !vk_ctx_->allocator()) {
        return stdx::err{error::GPU_DEVICE_NOT_FOUND};
    }

    VkBufferCreateInfo buffer_info{};
    buffer_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size        = raw_bytes.size();
    buffer_info.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
    alloc_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

    VkBuffer      staging_buffer = VK_NULL_HANDLE;
    VmaAllocation staging_alloc  = VK_NULL_HANDLE;

    if (vmaCreateBuffer(vk_ctx_->allocator(),
                        &buffer_info,
                        &alloc_info,
                        &staging_buffer,
                        &staging_alloc,
                        nullptr) != VK_SUCCESS) {
        return stdx::err{error::GPU_TRANSFER_BUFFER_FAILED};
    }
    const auto cleanup_staging = gsl::finally([this, staging_buffer, staging_alloc] {
        vmaDestroyBuffer(vk_ctx_->allocator(), staging_buffer, staging_alloc);
    });

    void* mapped_data = nullptr;
    if (vmaMapMemory(vk_ctx_->allocator(), staging_alloc, &mapped_data) != VK_SUCCESS) {
        return stdx::err{error::GPU_TRANSFER_BUFFER_FAILED};
    }
    std::memcpy(mapped_data, raw_bytes.data(), raw_bytes.size());
    vmaUnmapMemory(vk_ctx_->allocator(), staging_alloc);

    VkImageCreateInfo image_info{};
    image_info.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType     = VK_IMAGE_TYPE_2D;
    image_info.extent.width  = width;
    image_info.extent.height = height;
    image_info.extent.depth  = 1;
    image_info.mipLevels     = 1;
    image_info.arrayLayers   = 1;
    image_info.format        = VK_FORMAT_R8G8B8A8_UNORM;
    image_info.tiling        = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.samples       = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo img_alloc_info{};
    img_alloc_info.usage = VMA_MEMORY_USAGE_AUTO;

    VkImage       gpu_image = VK_NULL_HANDLE;
    VmaAllocation gpu_alloc = VK_NULL_HANDLE;

    if (vmaCreateImage(
            vk_ctx_->allocator(), &image_info, &img_alloc_info, &gpu_image, &gpu_alloc, nullptr) !=
        VK_SUCCESS) {
        return stdx::err{error::GPU_TEXTURE_ALLOC_FAILED};
    }

    const bool upload_ok = vk_ctx_->execute_one_time_command([&](VkCommandBuffer cmd) {
        VkImageMemoryBarrier barrier1{};
        barrier1.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier1.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier1.newLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier1.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier1.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier1.image                           = gpu_image;
        barrier1.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier1.subresourceRange.baseMipLevel   = 0;
        barrier1.subresourceRange.levelCount     = 1;
        barrier1.subresourceRange.baseArrayLayer = 0;
        barrier1.subresourceRange.layerCount     = 1;
        barrier1.srcAccessMask                   = 0;
        barrier1.dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;

        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier1);

        VkBufferImageCopy region{};
        region.bufferOffset                    = 0;
        region.bufferRowLength                 = 0;
        region.bufferImageHeight               = 0;
        region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel       = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount     = 1;
        region.imageOffset                     = {0, 0, 0};
        region.imageExtent                     = {width, height, 1};

        vkCmdCopyBufferToImage(
            cmd, staging_buffer, gpu_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        VkImageMemoryBarrier barrier2{};
        barrier2.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier2.oldLayout                       = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier2.newLayout                       = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier2.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier2.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
        barrier2.image                           = gpu_image;
        barrier2.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier2.subresourceRange.baseMipLevel   = 0;
        barrier2.subresourceRange.levelCount     = 1;
        barrier2.subresourceRange.baseArrayLayer = 0;
        barrier2.subresourceRange.layerCount     = 1;
        barrier2.srcAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier2.dstAccessMask                   = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cmd,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier2);
    });

    if (!upload_ok) {
        vmaDestroyImage(vk_ctx_->allocator(), gpu_image, gpu_alloc);
        return stdx::err{error::GPU_TRANSFER_BUFFER_FAILED};
    }

    VkImageViewCreateInfo view_info{};
    view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image                           = gpu_image;
    view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format                          = VK_FORMAT_R8G8B8A8_UNORM;
    view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel   = 0;
    view_info.subresourceRange.levelCount     = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount     = 1;

    VkImageView image_view = VK_NULL_HANDLE;
    if (vkCreateImageView(vk_ctx_->device(), &view_info, nullptr, &image_view) != VK_SUCCESS) {
        vmaDestroyImage(vk_ctx_->allocator(), gpu_image, gpu_alloc);
        return stdx::err{error::GPU_TEXTURE_ALLOC_FAILED};
    }

    TRY(ensure_sampler());

    VkDescriptorSet desc_set = ImGui_ImplVulkan_AddTexture(
        *linear_sampler_, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    if (desc_set == VK_NULL_HANDLE) {
        vkDestroyImageView(vk_ctx_->device(), image_view, nullptr);
        vmaDestroyImage(vk_ctx_->allocator(), gpu_image, gpu_alloc);
        return stdx::err{error::GPU_TEXTURE_ALLOC_FAILED};
    }

    texture tex;
    tex.device         = vk_ctx_->device();
    tex.allocator      = vk_ctx_->allocator();
    tex.image          = gpu_image;
    tex.allocation     = gpu_alloc;
    tex.view           = image_view;
    tex.sampler        = *linear_sampler_;
    tex.descriptor_set = desc_set;
    tex.imgui_id       = reinterpret_cast<ImTextureID>(desc_set);
    return tex;
}

} // namespace pbnj::ui::assets
