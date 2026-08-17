#pragma once

#include <array>
#include <concepts>
#include <vector>

#include <imgui.hh>
#include <stdx/option.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>
#include <vk_mem_alloc.h>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

#include "support/logger.hh"
#include "ui/core/compact_types.hh" // IWYU pragma: keep

namespace pbnj::ui {

class vk_context {
  public:
    vk_context(logger log) noexcept : log_{std::move(log)} {}
    ~vk_context() { cleanup(); }
    MAKE_PINNED(vk_context);

    auto init(GLFWwindow* window, bool enable_validation = false) noexcept -> bool;
    auto cleanup() noexcept -> void;

    auto recreate_swapchain(GLFWwindow* window) noexcept -> bool;
    auto cleanup_swapchain() noexcept -> void;

    auto begin_frame(GLFWwindow* window, ImVec4 clear_color) noexcept -> bool;
    auto end_frame(GLFWwindow* window, ImDrawData* draw_data) noexcept -> void;

    template <std::invocable<VkCommandBuffer> Fn>
    auto execute_one_time_command(Fn&& func) noexcept -> bool {
        if (!device_ || !command_pool_ || !queue_) { return false; }

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool        = *command_pool_;
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer cmd = VK_NULL_HANDLE;
        if (vkAllocateCommandBuffers(*device_, &alloc_info, &cmd) != VK_SUCCESS) { return false; }

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(cmd, &begin_info) != VK_SUCCESS) {
            vkFreeCommandBuffers(*device_, *command_pool_, 1, &cmd);
            return false;
        }

        std::forward<Fn>(func)(cmd);

        if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
            vkFreeCommandBuffers(*device_, *command_pool_, 1, &cmd);
            return false;
        }

        VkSubmitInfo submit_info{};
        submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers    = &cmd;

        if (vkQueueSubmit(*queue_, 1, &submit_info, VK_NULL_HANDLE) != VK_SUCCESS) {
            vkFreeCommandBuffers(*device_, *command_pool_, 1, &cmd);
            return false;
        }

        vkQueueWaitIdle(*queue_);
        vkFreeCommandBuffers(*device_, *command_pool_, 1, &cmd);
        return true;
    }

    auto wait_idle() const noexcept -> void {
        if (device_) { vkDeviceWaitIdle(*device_); }
    }

    [[nodiscard]] auto is_initialized() const noexcept -> bool { return is_initialized_; }
    [[nodiscard]] auto instance() const noexcept -> VkInstance { return *instance_; }
    [[nodiscard]] auto physical_device() const noexcept -> VkPhysicalDevice {
        return *physical_device_;
    }
    [[nodiscard]] auto device() const noexcept -> VkDevice { return *device_; }
    [[nodiscard]] auto queue() const noexcept -> VkQueue { return *queue_; }
    [[nodiscard]] auto queue_family_index() const noexcept -> u32 { return queue_family_index_; }
    [[nodiscard]] auto allocator() const noexcept -> VmaAllocator { return *allocator_; }
    [[nodiscard]] auto descriptor_pool() const noexcept -> VkDescriptorPool {
        return *descriptor_pool_;
    }
    [[nodiscard]] auto render_pass() const noexcept -> VkRenderPass { return *render_pass_; }
    [[nodiscard]] auto swapchain_image_count() const noexcept -> u32 {
        return static_cast<u32>(swapchain_images_.size());
    }
    [[nodiscard]] auto min_image_count() const noexcept -> u32 { return min_image_count_; }

    auto notify_framebuffer_resized() noexcept -> void { framebuffer_resized_ = true; }

  private:
    static constexpr usize MAX_FRAMES_IN_FLIGHT = 2;

  private:
    auto create_instance(bool enable_validation) noexcept -> bool;
    auto create_surface(GLFWwindow* window) noexcept -> bool;
    auto select_physical_device() noexcept -> bool;
    auto create_logical_device() noexcept -> bool;
    auto create_vma_allocator() noexcept -> bool;
    auto create_descriptor_pool() noexcept -> bool;
    auto create_command_pool() noexcept -> bool;
    auto create_sync_objects() noexcept -> bool;
    auto create_swapchain_internal(GLFWwindow* window) noexcept -> bool;

    static VKAPI_ATTR auto VKAPI_CALL
    debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
                   VkDebugUtilsMessageTypeFlagsEXT             type,
                   const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                   void*                                       user_data) -> VkBool32;

  private:
    logger                                 log_;
    stdx::option<VkInstance>               instance_;
    stdx::option<VkDebugUtilsMessengerEXT> debug_messenger_;
    stdx::option<VkSurfaceKHR>             surface_;
    stdx::option<VkPhysicalDevice>         physical_device_;
    stdx::option<VkDevice>                 device_;
    stdx::option<VkQueue>                  queue_;
    stdx::option<VmaAllocator>             allocator_;
    stdx::option<VkDescriptorPool>         descriptor_pool_;
    stdx::option<VkCommandPool>            command_pool_;

    stdx::option<VkSwapchainKHR> swapchain_;
    VkFormat                     swapchain_format_{VK_FORMAT_B8G8R8A8_UNORM};
    VkExtent2D                   swapchain_extent_{};
    u32                          queue_family_index_{0};
    u32                          min_image_count_{2};
    std::vector<VkImage>         swapchain_images_;
    std::vector<VkImageView>     swapchain_image_views_;
    stdx::option<VkRenderPass>   render_pass_;
    std::vector<VkFramebuffer>   framebuffers_;

    std::array<stdx::option<VkCommandBuffer>, MAX_FRAMES_IN_FLIGHT> command_buffers_{};
    std::array<stdx::option<VkSemaphore>, MAX_FRAMES_IN_FLIGHT>     image_available_semaphores_{};
    std::array<stdx::option<VkSemaphore>, MAX_FRAMES_IN_FLIGHT>     render_finished_semaphores_{};
    std::array<stdx::option<VkFence>, MAX_FRAMES_IN_FLIGHT>         in_flight_fences_{};

    usize current_frame_{0};
    u32   image_index_{0};
    bool  framebuffer_resized_{false};
    bool  in_frame_{false};
    bool  is_initialized_{false};
};

} // namespace pbnj::ui
