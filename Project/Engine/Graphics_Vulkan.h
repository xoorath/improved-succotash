#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#include <Engine/Graphics_VulkanForwardDecl.h> //in place of: <ThirdParty/Vulkan/vulkan.h>
//#include <stdint.h> // included by Graphics_VulkanForwardDecl

////////////////////////////////////////////////////////////////////////// Lifecycle

struct eng_Vulkan* eng_VulkanMalloc();
/** 
 * @Note initialization is not fully complete until eng_VulkanGetInstance
 * is called after appropriate setup is done. A common way for this setup 
 * to be provided is through a window. Calling eng_WindowBindVulkan will 
 * cause the window to provide configuration to vulkan for you.
 * @see eng_WindowBindVulkan
 */
bool eng_VulkanInit(struct eng_Vulkan* vulkan);
void eng_VulkanFree(struct eng_Vulkan* vulkan, bool subAllocationsOnly);
size_t eng_VulkanGetSizeof();

////////////////////////////////////////////////////////////////////////// Configuration API
void eng_VulkanProvideExtensions(struct eng_Vulkan* vulkan, const char** extensions, uint32_t extensionsCount);

// Default requires compute value is false
void eng_VulkanSetRequiresCompute(struct eng_Vulkan* vulkan, bool requiresCompute);

// Default requires compute value is true
void eng_VulkanSetRequiresGraphics(struct eng_Vulkan* vulkan, bool requiresGraphics);

// Default requires present value is false. Changes to true any time 
// eng_VulkanProvideSurface is called.
void eng_VulkanSetRequiresPresent(struct eng_Vulkan* vulkan, bool requiresPresent);

////////////////////////////////////////////////////////////////////////// API

bool eng_VulkanCreateInstance(struct eng_Vulkan* vulkan);
bool eng_VulkanProvideSurface(struct eng_Vulkan* vulkan, VkSurfaceKHR surface, uint16_t width, uint16_t height);

VkInstance eng_VulkanGetInstance(struct eng_Vulkan* vulkan);

void eng_VulkanUpdate(struct eng_Vulkan* vulkan);

// Requires <Engine/Log.h> to be included. Function must return a boolean for success.
#define eng_VulkanEnsure(result, step) if(!eng_Ensure(result == VK_SUCCESS, "Failed to " step ". Error(%d): \"%s\"", (int)result, eng_InternalVkResultToString(result))) { return false; }

#ifdef __cplusplus
}
#endif