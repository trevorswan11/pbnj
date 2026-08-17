#pragma once

#include <imgui.hh>
#include <stdx/option.hh>
#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

template <> struct stdx::nullable<ImTextureID> {
    [[nodiscard]] static constexpr auto invalid() noexcept -> ImTextureID {
        return ImTextureID{ImTextureID_Invalid};
    }

    [[nodiscard]] static constexpr auto is_valid(ImTextureID id) noexcept -> bool {
        return id != ImTextureID_Invalid;
    }
};

#define NULLABLE_VK_PTR(Type)                                                    \
    template <> struct stdx::nullable<Type> {                                    \
        [[nodiscard]] static constexpr auto invalid() noexcept -> Type {         \
            return Type{VK_NULL_HANDLE};                                         \
        }                                                                        \
                                                                                 \
        [[nodiscard]] static constexpr auto is_valid(Type id) noexcept -> bool { \
            return id != VK_NULL_HANDLE;                                         \
        }                                                                        \
    };

NULLABLE_VK_PTR(VkSampler)
NULLABLE_VK_PTR(VkDevice)
NULLABLE_VK_PTR(VmaAllocator)
NULLABLE_VK_PTR(VkImage)
NULLABLE_VK_PTR(VmaAllocation)
NULLABLE_VK_PTR(VkImageView)
NULLABLE_VK_PTR(VkDescriptorSet)
NULLABLE_VK_PTR(VkInstance)
NULLABLE_VK_PTR(VkDebugUtilsMessengerEXT)
NULLABLE_VK_PTR(VkSurfaceKHR)
NULLABLE_VK_PTR(VkPhysicalDevice)
NULLABLE_VK_PTR(VkQueue)
NULLABLE_VK_PTR(VkDescriptorPool)
NULLABLE_VK_PTR(VkCommandPool)
NULLABLE_VK_PTR(VkSwapchainKHR)
NULLABLE_VK_PTR(VkRenderPass)
NULLABLE_VK_PTR(VkCommandBuffer)
NULLABLE_VK_PTR(VkSemaphore)
NULLABLE_VK_PTR(VkFence)

#undef NULLABLE_VK_PTR
