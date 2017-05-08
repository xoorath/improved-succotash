#pragma once

#if !defined(VULKAN_H_)

#include <stdint.h>

#define VK_DEFINE_HANDLE(object) typedef struct object##_T* object;
#if !defined(VK_DEFINE_NON_DISPATCHABLE_HANDLE)
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__) ) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif
#endif

// Uncomment or add handle forward declarations as needed:

VK_DEFINE_HANDLE(VkInstance)
//VK_DEFINE_HANDLE(VkPhysicalDevice)
//VK_DEFINE_HANDLE(VkDevice)
//VK_DEFINE_HANDLE(VkQueue)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSemaphore)
//VK_DEFINE_HANDLE(VkCommandBuffer)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFence)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDeviceMemory)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBuffer)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImage)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkEvent)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkQueryPool)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBufferView)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImageView)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkShaderModule)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineCache)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineLayout)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkRenderPass)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipeline)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorSetLayout)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSampler)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorPool)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorSet)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFramebuffer)
//VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkCommandPool)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)

#undef VK_DEFINE_HANDLE
#undef VK_DEFINE_NON_DISPATCHABLE_HANDLE

#endif