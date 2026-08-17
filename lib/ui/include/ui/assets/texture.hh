#pragma once

#include <imgui.hh>
#include <stdx/option.hh>
#include <stdx/utility.hh>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "ui/core/compact_types.hh" // IWYU pragma: keep

namespace pbnj::ui::assets {

struct texture {
    texture() = default;
    ~texture() { release(); }

    texture(const texture&)                    = delete;
    auto operator=(const texture&) -> texture& = delete;

    texture(texture&& other) noexcept
        : device{other.device.take()}, allocator{other.allocator.take()}, image{other.image.take()},
          allocation{other.allocation.take()}, view{other.view.take()},
          sampler{other.sampler.take()}, descriptor_set{other.descriptor_set.take()},
          imgui_id{other.imgui_id.take()} {}

    auto operator=(texture&& other) noexcept -> texture& {
        if (this != &other) {
            release();
            device         = other.device.take();
            allocator      = other.allocator.take();
            image          = other.image.take();
            allocation     = other.allocation.take();
            view           = other.view.take();
            sampler        = other.sampler.take();
            descriptor_set = other.descriptor_set.take();
            imgui_id       = other.imgui_id.take();
        }
        return *this;
    }

    auto release() noexcept -> void {
        if (descriptor_set) { ImGui_ImplVulkan_RemoveTexture(descriptor_set.take()); }
        if (device && view) { vkDestroyImageView(*device, view.take(), nullptr); }
        if (allocator && image) { vmaDestroyImage(*allocator, image.take(), allocation.take()); }
        sampler.reset();
        device.reset();
        allocator.reset();
        imgui_id.reset();
    }

    stdx::option<VkDevice>        device;
    stdx::option<VmaAllocator>    allocator;
    stdx::option<VkImage>         image;
    stdx::option<VmaAllocation>   allocation;
    stdx::option<VkImageView>     view;
    stdx::option<VkSampler>       sampler;
    stdx::option<VkDescriptorSet> descriptor_set;
    stdx::option<ImTextureID>     imgui_id;
};

} // namespace pbnj::ui::assets
