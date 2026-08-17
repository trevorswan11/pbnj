#include "ui/core/vk_context.hh"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string_view>
#include <vector>

#include <glfw.hh>
#include <gsl/pointers>
#include <gsl/span>
#include <gsl/util>
#include <imgui.hh>
#include <pbnj/config.h>
#include <stdx/types.hh>
#include <stdx/utility.hh>
#include <vulkan/vk_platform.h>
#include <vulkan/vulkan_core.h>

namespace pbnj::ui {

auto vk_context::init(GLFWwindow* window, bool enable_validation) noexcept -> bool {
    if (volkInitialize() != VK_SUCCESS) { return false; }
    if (!create_instance(enable_validation)) { return false; }
    if (!create_surface(window)) { return false; }
    if (!select_physical_device()) { return false; }
    if (!create_logical_device()) { return false; }
    if (!create_vma_allocator()) { return false; }
    if (!create_descriptor_pool()) { return false; }
    if (!create_command_pool()) { return false; }
    if (!create_sync_objects()) { return false; }
    if (!create_swapchain_internal(window)) { return false; }

    is_initialized_ = true;
    return true;
}

auto vk_context::cleanup() noexcept -> void {
    if (!is_initialized_) { return; }
    wait_idle();
    cleanup_swapchain();

    for (auto& fence : in_flight_fences_) { vkDestroyFence(*device_, fence.take(), nullptr); }
    for (auto& semaphore : render_finished_semaphores_) {
        vkDestroySemaphore(*device_, semaphore.take(), nullptr);
    }
    for (auto& img : image_available_semaphores_) {
        vkDestroySemaphore(*device_, img.take(), nullptr);
    }

    if (command_pool_) { vkDestroyCommandPool(*device_, command_pool_.take(), nullptr); }
    if (descriptor_pool_) { vkDestroyDescriptorPool(*device_, descriptor_pool_.take(), nullptr); }
    if (allocator_) { vmaDestroyAllocator(allocator_.take()); }
    if (device_) { vkDestroyDevice(device_.take(), nullptr); }

    if (debug_messenger_ && vkDestroyDebugUtilsMessengerEXT) {
        vkDestroyDebugUtilsMessengerEXT(*instance_, debug_messenger_.take(), nullptr);
    }

    if (surface_) { vkDestroySurfaceKHR(*instance_, surface_.take(), nullptr); }
    if (instance_) { vkDestroyInstance(instance_.take(), nullptr); }
    is_initialized_ = false;
}

auto vk_context::recreate_swapchain(GLFWwindow* window) noexcept -> bool {
    i32 width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    if (width <= 0 || height <= 0) { return false; }

    wait_idle();
    cleanup_swapchain();
    return create_swapchain_internal(window);
}

auto vk_context::cleanup_swapchain() noexcept -> void {
    for (auto fb : framebuffers_) {
        if (fb) { vkDestroyFramebuffer(*device_, fb, nullptr); }
    }
    framebuffers_.clear();

    if (render_pass_) { vkDestroyRenderPass(*device_, render_pass_.take(), nullptr); }
    for (auto iv : swapchain_image_views_) {
        if (iv) { vkDestroyImageView(*device_, iv, nullptr); }
    }
    swapchain_image_views_.clear();
    swapchain_images_.clear();

    if (swapchain_) { vkDestroySwapchainKHR(*device_, swapchain_.take(), nullptr); }
}

auto vk_context::begin_frame(GLFWwindow* window, ImVec4 clear_color) noexcept -> bool {
    if (!is_initialized_ || in_frame_) { return false; }

    i32 cur_w = 0, cur_h = 0;
    glfwGetFramebufferSize(window, &cur_w, &cur_h);
    if (cur_w <= 0 || cur_h <= 0) { return false; }

    if (framebuffer_resized_ || static_cast<u32>(cur_w) != swapchain_extent_.width ||
        static_cast<u32>(cur_h) != swapchain_extent_.height) {
        framebuffer_resized_ = false;
        if (!recreate_swapchain(window)) { return false; }
    }

    vkWaitForFences(*device_, 1, in_flight_fences_[current_frame_].get(), VK_TRUE, UINT64_MAX);

    VkResult acquire_res = vkAcquireNextImageKHR(*device_,
                                                 *swapchain_,
                                                 UINT64_MAX,
                                                 *image_available_semaphores_[current_frame_],
                                                 VK_NULL_HANDLE,
                                                 &image_index_);

    if (acquire_res == VK_ERROR_OUT_OF_DATE_KHR) {
        if (!recreate_swapchain(window)) { return false; }
        return false;
    }
    if (acquire_res != VK_SUCCESS && acquire_res != VK_SUBOPTIMAL_KHR) { return false; }

    vkResetFences(*device_, 1, in_flight_fences_[current_frame_].get());

    auto cmd = command_buffers_[current_frame_];
    vkResetCommandBuffer(*cmd.raw(), 0);

    VkCommandBufferBeginInfo begin_info{};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(*cmd, &begin_info) != VK_SUCCESS) { return false; }

    VkClearValue clear_value{};
    clear_value.color = {{clear_color.x, clear_color.y, clear_color.z, clear_color.w}};

    VkRenderPassBeginInfo rp_begin{};
    rp_begin.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.renderPass        = *render_pass_;
    rp_begin.framebuffer       = framebuffers_[image_index_];
    rp_begin.renderArea.offset = {0, 0};
    rp_begin.renderArea.extent = swapchain_extent_;
    rp_begin.clearValueCount   = 1;
    rp_begin.pClearValues      = &clear_value;

    vkCmdBeginRenderPass(*cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

    in_frame_ = true;
    return true;
}

auto vk_context::end_frame(GLFWwindow* window, ImDrawData* draw_data) noexcept -> void {
    if (!is_initialized_ || !in_frame_) { return; }
    in_frame_ = false;

    auto cmd = command_buffers_[current_frame_];
    if (draw_data) { ImGui_ImplVulkan_RenderDrawData(draw_data, *cmd); }

    vkCmdEndRenderPass(*cmd);
    vkEndCommandBuffer(*cmd);

    const std::array wait_semaphores = {*image_available_semaphores_[current_frame_]};
    const std::array wait_stages     = {
        VkPipelineStageFlags{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}};
    const std::array signal_semaphores = {*render_finished_semaphores_[current_frame_]};

    VkSubmitInfo submit_info{};
    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pWaitSemaphores      = wait_semaphores.data();
    submit_info.pWaitDstStageMask    = wait_stages.data();
    submit_info.commandBufferCount   = 1;
    submit_info.pCommandBuffers      = cmd.get();
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = signal_semaphores.data();

    vkQueueSubmit(*queue_, 1, &submit_info, *in_flight_fences_[current_frame_]);

    const std::array swapchains = {*swapchain_};
    VkPresentInfoKHR present_info{};
    present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores    = signal_semaphores.data();
    present_info.swapchainCount     = 1;
    present_info.pSwapchains        = swapchains.data();
    present_info.pImageIndices      = &image_index_;

    VkResult present_res = vkQueuePresentKHR(*queue_, &present_info);

    if (present_res == VK_ERROR_OUT_OF_DATE_KHR || present_res == VK_SUBOPTIMAL_KHR ||
        framebuffer_resized_) {
        framebuffer_resized_ = false;
        recreate_swapchain(window);
    }

    current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

auto vk_context::create_instance(bool enable_validation) noexcept -> bool {
    u32          glfw_ext_count = 0;
    const char** glfw_exts      = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
    if (!glfw_exts || glfw_ext_count == 0) { return false; }

    std::vector<const char*> extensions(glfw_exts, glfw_exts + glfw_ext_count);
    std::vector<const char*> layers;

    u32 instance_ext_count = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instance_ext_count, nullptr);
    std::vector<VkExtensionProperties> available_instance_exts(instance_ext_count);
    vkEnumerateInstanceExtensionProperties(
        nullptr, &instance_ext_count, available_instance_exts.data());

#ifdef PBNJ_APPLE
    const auto has_instance_ext = [&](const char* name) {
        return std::ranges::any_of(available_instance_exts, [&](const VkExtensionProperties& p) {
            return std::string_view{p.extensionName} == name;
        });
    };

    if (has_instance_ext(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    }
    if (has_instance_ext(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
        extensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    }
#endif

    if (enable_validation) {
        u32 layer_count = 0;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for (const auto& layer : available_layers) {
            if (std::string_view{layer.layerName} == "VK_LAYER_KHRONOS_validation") {
                layers.emplace_back("VK_LAYER_KHRONOS_validation");
                extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                break;
            }
        }
    }

    VkApplicationInfo app_info{};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = "PBnJ";
    app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.pEngineName        = "PBnJ";
    app_info.engineVersion      = VK_MAKE_VERSION(0, 0, 1);
    app_info.apiVersion         = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifdef PBNJ_APPLE
    if (has_instance_ext(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        create_info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif
    create_info.pApplicationInfo        = &app_info;
    create_info.enabledExtensionCount   = static_cast<u32>(extensions.size());
    create_info.ppEnabledExtensionNames = extensions.data();
    create_info.enabledLayerCount       = static_cast<u32>(layers.size());
    create_info.ppEnabledLayerNames     = layers.data();

    if (vkCreateInstance(&create_info, nullptr, instance_.raw()) != VK_SUCCESS) { return false; }
    volkLoadInstance(*instance_);

    if (enable_validation && vkCreateDebugUtilsMessengerEXT) {
        VkDebugUtilsMessengerCreateInfoEXT debug_info{};
        debug_info.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_info.pfnUserCallback = debug_callback;
        debug_info.pUserData       = this;
        vkCreateDebugUtilsMessengerEXT(*instance_, &debug_info, nullptr, debug_messenger_.raw());
    }

    return true;
}

auto vk_context::create_surface(GLFWwindow* window) noexcept -> bool {
    return glfwCreateWindowSurface(*instance_, window, nullptr, surface_.raw()) == VK_SUCCESS;
}

auto vk_context::select_physical_device() noexcept -> bool {
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(*instance_, &device_count, nullptr);
    if (device_count == 0) { return false; }

    std::vector<VkPhysicalDevice> devices(device_count);
    vkEnumeratePhysicalDevices(*instance_, &device_count, devices.data());

    VkPhysicalDevice fallback_device      = VK_NULL_HANDLE;
    u32              fallback_queue_index = 0;

    for (const auto& dev : devices) {
        u32 queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_count, queue_families.data());

        for (u32 i = 0; i < queue_family_count; ++i) {
            VkBool32 present_support = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(dev, i, *surface_, &present_support);

            if ((queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present_support) {
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(dev, &props);

                if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                    physical_device_    = dev;
                    queue_family_index_ = i;
                    return true;
                }

                if (fallback_device == VK_NULL_HANDLE) {
                    fallback_device      = dev;
                    fallback_queue_index = i;
                }
            }
        }
    }

    if (fallback_device != VK_NULL_HANDLE) {
        physical_device_    = fallback_device;
        queue_family_index_ = fallback_queue_index;
        return true;
    }

    return false;
}

auto vk_context::create_logical_device() noexcept -> bool {
    const float queue_priority = 1.0f;

    VkDeviceQueueCreateInfo queue_create_info{};
    queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = queue_family_index_;
    queue_create_info.queueCount       = 1;
    queue_create_info.pQueuePriorities = &queue_priority;

    std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    u32 ext_count = 0;
    vkEnumerateDeviceExtensionProperties(*physical_device_, nullptr, &ext_count, nullptr);
    std::vector<VkExtensionProperties> available_device_exts(ext_count);
    vkEnumerateDeviceExtensionProperties(
        *physical_device_, nullptr, &ext_count, available_device_exts.data());

    for (const auto& ext : available_device_exts) {
        if (std::strcmp(ext.extensionName, "VK_KHR_portability_subset") == 0) {
            device_extensions.emplace_back("VK_KHR_portability_subset");
            break;
        }
    }

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo create_info{};
    create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount    = 1;
    create_info.pQueueCreateInfos       = &queue_create_info;
    create_info.enabledExtensionCount   = static_cast<u32>(device_extensions.size());
    create_info.ppEnabledExtensionNames = device_extensions.data();
    create_info.pEnabledFeatures        = &device_features;

    if (vkCreateDevice(*physical_device_, &create_info, nullptr, device_.raw()) != VK_SUCCESS) {
        return false;
    }

    volkLoadDevice(*device_);
    vkGetDeviceQueue(*device_, queue_family_index_, 0, queue_.raw());
    return true;
}

auto vk_context::create_vma_allocator() noexcept -> bool {
    VmaVulkanFunctions vma_funcs{};
    vma_funcs.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vma_funcs.vkGetDeviceProcAddr   = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo alloc_info{};
    alloc_info.vulkanApiVersion = VK_API_VERSION_1_4;
    alloc_info.physicalDevice   = *physical_device_;
    alloc_info.device           = *device_;
    alloc_info.instance         = *instance_;
    alloc_info.pVulkanFunctions = &vma_funcs;

    return vmaCreateAllocator(&alloc_info, allocator_.raw()) == VK_SUCCESS;
}

auto vk_context::create_descriptor_pool() noexcept -> bool {
    const auto pool_sizes = std::to_array<VkDescriptorPoolSize>({
        {VK_DESCRIPTOR_TYPE_SAMPLER, 100},
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1'000},
        {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1'000},
        {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100},
        {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100},
        {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100},
        {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100},
    });

    VkDescriptorPoolCreateInfo pool_info{};
    pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets       = 1'000;
    pool_info.poolSizeCount = static_cast<u32>(pool_sizes.size());
    pool_info.pPoolSizes    = pool_sizes.data();

    return vkCreateDescriptorPool(*device_, &pool_info, nullptr, descriptor_pool_.raw()) ==
           VK_SUCCESS;
}

auto vk_context::create_command_pool() noexcept -> bool {
    VkCommandPoolCreateInfo pool_info{};
    pool_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family_index_;

    return vkCreateCommandPool(*device_, &pool_info, nullptr, command_pool_.raw()) == VK_SUCCESS;
}

auto vk_context::create_sync_objects() noexcept -> bool {
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool        = *command_pool_;
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> raw_cmd_buffers{};
    if (vkAllocateCommandBuffers(*device_, &alloc_info, raw_cmd_buffers.data()) != VK_SUCCESS) {
        return false;
    }
    for (usize i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) { command_buffers_[i] = raw_cmd_buffers[i]; }

    VkSemaphoreCreateInfo sem_info{};
    sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info{};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (auto& img : image_available_semaphores_) {
        if (vkCreateSemaphore(*device_, &sem_info, nullptr, img.raw()) != VK_SUCCESS) {
            return false;
        }
    }
    for (auto& semaphore : render_finished_semaphores_) {
        if (vkCreateSemaphore(*device_, &sem_info, nullptr, semaphore.raw()) != VK_SUCCESS) {
            return false;
        }
    }
    for (auto& fence : in_flight_fences_) {
        if (vkCreateFence(*device_, &fence_info, nullptr, fence.raw()) != VK_SUCCESS) {
            return false;
        }
    }

    return true;
}

auto vk_context::create_swapchain_internal(GLFWwindow* window) noexcept -> bool {
    i32 width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    if (width <= 0 || height <= 0) { return false; }

    VkSurfaceCapabilitiesKHR caps{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*physical_device_, *surface_, &caps);

    u32 format_count = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(*physical_device_, *surface_, &format_count, nullptr);
    if (format_count == 0) { return false; }
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(
        *physical_device_, *surface_, &format_count, formats.data());

    VkSurfaceFormatKHR selected_format = formats[0];
    for (const auto& fmt : formats) {
        if (fmt.format == VK_FORMAT_B8G8R8A8_UNORM &&
            fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            selected_format = fmt;
            break;
        }
    }
    swapchain_format_ = selected_format.format;

    if (caps.currentExtent.width != std::numeric_limits<u32>::max()) {
        swapchain_extent_ = caps.currentExtent;
    } else {
        swapchain_extent_ = {
            std::clamp(
                static_cast<u32>(width), caps.minImageExtent.width, caps.maxImageExtent.width),
            std::clamp(
                static_cast<u32>(height), caps.minImageExtent.height, caps.maxImageExtent.height),
        };
    }

    u32 img_count = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && img_count > caps.maxImageCount) {
        img_count = caps.maxImageCount;
    }
    min_image_count_ = caps.minImageCount;

    VkSwapchainCreateInfoKHR create_info{};
    create_info.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface          = *surface_;
    create_info.minImageCount    = img_count;
    create_info.imageFormat      = selected_format.format;
    create_info.imageColorSpace  = selected_format.colorSpace;
    create_info.imageExtent      = swapchain_extent_;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    create_info.preTransform     = caps.currentTransform;
    create_info.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
    create_info.clipped          = VK_TRUE;

    if (vkCreateSwapchainKHR(*device_, &create_info, nullptr, swapchain_.raw()) != VK_SUCCESS) {
        return false;
    }

    u32 actual_image_count = 0;
    vkGetSwapchainImagesKHR(*device_, *swapchain_, &actual_image_count, nullptr);
    swapchain_images_.resize(actual_image_count);
    vkGetSwapchainImagesKHR(*device_, *swapchain_, &actual_image_count, swapchain_images_.data());

    swapchain_image_views_.resize(actual_image_count);
    for (usize i = 0; i < actual_image_count; ++i) {
        VkImageViewCreateInfo view_info{};
        view_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        view_info.image                           = swapchain_images_[i];
        view_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        view_info.format                          = swapchain_format_;
        view_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        view_info.subresourceRange.baseMipLevel   = 0;
        view_info.subresourceRange.levelCount     = 1;
        view_info.subresourceRange.baseArrayLayer = 0;
        view_info.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(*device_, &view_info, nullptr, &swapchain_image_views_[i]) !=
            VK_SUCCESS) {
            return false;
        }
    }

    VkAttachmentDescription color_attachment{};
    color_attachment.format         = swapchain_format_;
    color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_ref{};
    color_ref.attachment = 0;
    color_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &color_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass    = 0;
    dependency.srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rp_info{};
    rp_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = 1;
    rp_info.pAttachments    = &color_attachment;
    rp_info.subpassCount    = 1;
    rp_info.pSubpasses      = &subpass;
    rp_info.dependencyCount = 1;
    rp_info.pDependencies   = &dependency;

    if (vkCreateRenderPass(*device_, &rp_info, nullptr, render_pass_.raw()) != VK_SUCCESS) {
        return false;
    }

    framebuffers_.resize(actual_image_count);
    for (usize i = 0; i < actual_image_count; ++i) {
        const std::array attachments = {swapchain_image_views_[i]};

        VkFramebufferCreateInfo fb_info{};
        fb_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.renderPass      = *render_pass_;
        fb_info.attachmentCount = 1;
        fb_info.pAttachments    = attachments.data();
        fb_info.width           = swapchain_extent_.width;
        fb_info.height          = swapchain_extent_.height;
        fb_info.layers          = 1;

        if (vkCreateFramebuffer(*device_, &fb_info, nullptr, &framebuffers_[i]) != VK_SUCCESS) {
            return false;
        }
    }

    return true;
}

VKAPI_ATTR auto VKAPI_CALL
vk_context::debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
                           VkDebugUtilsMessageTypeFlagsEXT             type,
                           const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                           void*                                       user_data) -> VkBool32 {
    static constexpr std::array severity_strs{
        "verbose",
        "info",
        "warning",
        "error",
        "unknown",
    };
    auto* vk_ctx = static_cast<vk_context*>(user_data);
    if (!vk_ctx) { return VK_FALSE; }

    const std::string_view severity_str = severity_strs[severity];
    std::string_view       message      = "NO MESSAGE!";
    if (callback_data) { message = callback_data->pMessage; }

    std::string_view type_str = "unknown";
    if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        type_str = "general";
    } else if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        type_str = "validation";
    } else if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        type_str = "performance";
    } else if (type & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT) {
        type_str = "device addr";
    }

    vk_ctx->log_.info("[{}][{}]: {}", severity_str, type_str, message);
    return VK_FALSE;
}

} // namespace pbnj::ui
