// The basic "up and running" code in this file was borrowed from github.com/corngood/SDL_vulkan
// This licence covers this code.
/*
The MIT License (MIT)

Copyright (c) 2016 David McFarland

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Engine/Graphics_Vulkan.h>

#include <Engine/Array.h>
#include <Engine/Log.h>

#if defined(GAME_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <ThirdParty/Vulkan/vulkan.h>

#include <Engine/Graphics_VulkanInternal.h>

#include <assert.h>

#define FENCE_COUNT 2
#define SAMPLE_COUNT 1

struct eng_BufferInfo
{
	VkImage image;
	VkCommandBuffer cmd;
	VkImageView view;
	VkFramebuffer fb;
};

struct eng_Vulkan
{
	VkInstance Instance;
	VkDevice Device;
	VkSwapchainKHR Swapchain;
	VkExtent2D SwapchainExtent;
	VkCommandBuffer DrawCmd;
	VkRenderPass RenderPass;
	VkQueue Queue;
	struct eng_BufferInfo* Buffers;

	eng_ArrayDecl(Extensions, const char*);
};


////////////////////////////////////////////////////////////////////////// Lifecycle

struct eng_Vulkan* eng_VulkanMalloc()
{
	return malloc(sizeof(struct eng_Vulkan));
}

bool eng_VulkanInit(struct eng_Vulkan* vulkan)
{
	memset(vulkan, 0, sizeof(struct eng_Vulkan));

	eng_ArrayInitType(&vulkan->Extensions, const char*);

	return true;
}

void eng_VulkanFree(struct eng_Vulkan* vulkan, bool subAllocationsOnly)
{
	if (vulkan == NULL)
	{
		return;
	}

	free(vulkan->Buffers);
	eng_ArrayDestroy(&vulkan->Extensions);

	if (!subAllocationsOnly)
	{
		free(vulkan);
	}
}

size_t eng_VulkanGetSizeof()
{
	return sizeof(struct eng_Vulkan);
}

////////////////////////////////////////////////////////////////////////// Configuration API

void eng_VulkanProvideExtensions(struct eng_Vulkan* vulkan, const char** extensions, uint32_t extensionsCount)
{
	eng_ArrayResize(&vulkan->Extensions, extensionsCount);
	for (uint32_t i = 0; i < extensionsCount; ++i)
	{
		eng_ArrayIndexType(&vulkan->Extensions, const char*, i) = extensions[i];
	}
}


void eng_VulkanSetRequiresCompute(struct eng_Vulkan* vulkan, bool requiresCompute)
{
}

void eng_VulkanSetRequiresGraphics(struct eng_Vulkan* vulkan, bool requiresGraphics)
{
}

void eng_VulkanSetRequiresPresent(struct eng_Vulkan* vulkan, bool requiresPresent)
{
}

////////////////////////////////////////////////////////////////////////// API
bool eng_VulkanCreateInstance(struct eng_Vulkan* vulkan)
{
	VkResult err;
	{
		uint32_t extension_count = 0;
		const char *extension_names[64];
		extension_names[extension_count++] = VK_KHR_SURFACE_EXTENSION_NAME;
		const VkApplicationInfo app = {
			.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = "SDL_vulkan example",
			.apiVersion = VK_MAKE_VERSION(1, 0, 3),
		};

		for (uint32_t i = 0; i < vulkan->Extensions.Count; ++i)
		{
			extension_names[extension_count++] = eng_ArrayIndexType(&vulkan->Extensions, const char*, i);
		}

		VkInstanceCreateInfo inst_info = {
			.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pApplicationInfo = &app,
			.enabledExtensionCount = extension_count,
			.ppEnabledExtensionNames = extension_names,
		};

		err = vkCreateInstance(&inst_info, NULL, &vulkan->Instance);
		assert(!err);
	}



	return true;
}

bool eng_VulkanProvideSurface(struct eng_Vulkan* vulkan, VkSurfaceKHR surface, uint16_t width, uint16_t height)
{
	VkResult err;
	VkPhysicalDevice gpu;
	{
		uint32_t gpu_count;
		err = vkEnumeratePhysicalDevices(vulkan->Instance, &gpu_count, NULL);
		assert(!err && gpu_count > 0);

		if (gpu_count > 0)
		{
			VkPhysicalDevice* gpus = calloc(gpu_count, sizeof(VkPhysicalDevice));
			err = vkEnumeratePhysicalDevices(vulkan->Instance, &gpu_count, gpus);
			assert(!err);
			gpu = gpus[0];
			free(gpus);
		}
		else
		{
			gpu = VK_NULL_HANDLE;
		}
	}

	VkCommandPool cmd_pool;
	{
		uint32_t queue_family_index = UINT32_MAX;
		{
			uint32_t queue_count;
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, NULL);

			VkQueueFamilyProperties* queue_props = calloc(queue_count, sizeof(VkQueueFamilyProperties));
			vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queue_count, queue_props);
			assert(queue_count >= 1);

			for (uint32_t i = 0; i < queue_count; i++)
			{
				VkBool32 supports_present;
				vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surface, &supports_present);
				if (supports_present && (queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
				{
					queue_family_index = i;
					break;
				}
			}
			assert(queue_family_index != UINT32_MAX);
			free(queue_props);
		}

		uint32_t extension_count = 0;
		const char *extension_names[64];
		extension_count = 0;
		extension_names[extension_count++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

		float queue_priorities[1] = {0.0};
		const VkDeviceQueueCreateInfo queueInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = queue_family_index,
			.queueCount = 1,
			.pQueuePriorities = queue_priorities};

		VkDeviceCreateInfo deviceInfo = {
			.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &queueInfo,
			.enabledExtensionCount = extension_count,
			.ppEnabledExtensionNames = (const char *const *)extension_names,
		};

		err = vkCreateDevice(gpu, &deviceInfo, NULL, &vulkan->Device);
		assert(!err);

		vkGetDeviceQueue(vulkan->Device, queue_family_index, 0, &vulkan->Queue);

		const VkCommandPoolCreateInfo cmd_pool_info = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = queue_family_index,
			.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		};
		err = vkCreateCommandPool(vulkan->Device, &cmd_pool_info, NULL, &cmd_pool);
		assert(!err);
	}

	VkFormat           format;
	VkColorSpaceKHR    color_space;
	{
		uint32_t format_count;
		err = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, NULL);
		assert(!err);

		VkSurfaceFormatKHR* formats = calloc(format_count, sizeof(VkSurfaceFormatKHR));
		err = vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &format_count, formats);
		assert(!err);

		if (format_count == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
		{
			format = VK_FORMAT_B8G8R8A8_SRGB;
		}
		else
		{
			assert(format_count >= 1);
			format = formats[0].format;
		}
		color_space = formats[0].colorSpace;

		free(formats);
	}

	{
		const VkCommandBufferAllocateInfo cmd = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = cmd_pool,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		};
		err = vkAllocateCommandBuffers(vulkan->Device, &cmd, &vulkan->DrawCmd);
		assert(!err);
	}

	uint32_t swapchain_image_count;
	{
		VkSurfaceCapabilitiesKHR surf_cap;
		err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surf_cap);
		assert(!err);

		if (surf_cap.currentExtent.width == (uint32_t)-1)
		{
			vulkan->SwapchainExtent.width = (uint32_t)width;
			vulkan->SwapchainExtent.height = (uint32_t)height;
		}
		else
		{
			vulkan->SwapchainExtent = surf_cap.currentExtent;
		}

		swapchain_image_count = surf_cap.minImageCount + 1;
		if ((surf_cap.maxImageCount > 0) && (swapchain_image_count > surf_cap.maxImageCount))
		{
			swapchain_image_count = surf_cap.maxImageCount;
		}

		const VkSwapchainCreateInfoKHR swapchainInfo = {
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = surface,
			.minImageCount = swapchain_image_count,
			.imageFormat = format,
			.imageColorSpace = color_space,
			.imageExtent = vulkan->SwapchainExtent,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform = surf_cap.currentTransform,
			.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.imageArrayLayers = 1,
			.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
			.presentMode = VK_PRESENT_MODE_FIFO_KHR,
			.clipped = 1,
		};

		err = vkCreateSwapchainKHR(vulkan->Device, &swapchainInfo, NULL, &vulkan->Swapchain);
		assert(!err);
	}

	vulkan->Buffers = calloc(swapchain_image_count, sizeof(struct eng_BufferInfo));

	{
		err = vkGetSwapchainImagesKHR(vulkan->Device, vulkan->Swapchain, &swapchain_image_count, 0);
		assert(!err);
		VkImage* swapchain_images = calloc(swapchain_image_count, sizeof(VkImage));
		err = vkGetSwapchainImagesKHR(vulkan->Device, vulkan->Swapchain, &swapchain_image_count, swapchain_images);
		assert(!err);
		for (uint32_t i = 0; i < swapchain_image_count; i++)
		{
			vulkan->Buffers[i].image = swapchain_images[i];
		}
		free(swapchain_images);
	}

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		VkImageViewCreateInfo color_attachment_view = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.format = format,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_R,
				.g = VK_COMPONENT_SWIZZLE_G,
				.b = VK_COMPONENT_SWIZZLE_B,
				.a = VK_COMPONENT_SWIZZLE_A,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.image = vulkan->Buffers[i].image,
		};

		err = vkCreateImageView(vulkan->Device, &color_attachment_view, NULL, &vulkan->Buffers[i].view);
		assert(!err);
	}

	const VkAttachmentDescription attachments[1] = {
		[0] = {
			.format = format,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
			.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		},
	};
	const VkAttachmentReference color_reference = {
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	const VkSubpassDescription subpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = 1,
		.pColorAttachments = &color_reference,
	};
	const VkRenderPassCreateInfo rp_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = 1,
		.pAttachments = attachments,
		.subpassCount = 1,
		.pSubpasses = &subpass,
	};

	err = vkCreateRenderPass(vulkan->Device, &rp_info, NULL, &vulkan->RenderPass);
	assert(!err);

	for (uint32_t i = 0; i < swapchain_image_count; i++)
	{
		VkImageView attachments[1] = {
			[0] = vulkan->Buffers[i].view,
		};

		const VkFramebufferCreateInfo fb_info = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = vulkan->RenderPass,
			.attachmentCount = 1,
			.pAttachments = attachments,
			.width = vulkan->SwapchainExtent.width,
			.height = vulkan->SwapchainExtent.height,
			.layers = 1,
		};

		err = vkCreateFramebuffer(vulkan->Device, &fb_info, NULL, &vulkan->Buffers[i].fb);
		assert(!err);
	}
	return true;
}

VkInstance eng_VulkanGetInstance(struct eng_Vulkan* vulkan)
{
	return vulkan->Instance;
}

void eng_VulkanUpdate(struct eng_Vulkan* vulkan)
{
	VkResult err;
	VkSemaphore present_complete_semaphore;
	{
		VkSemaphoreCreateInfo semaphore_create_info = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};
		err = vkCreateSemaphore(vulkan->Device, &semaphore_create_info, NULL, &present_complete_semaphore);
		assert(!err);
	}

	uint32_t current_buffer;
	err = vkAcquireNextImageKHR(vulkan->Device, vulkan->Swapchain, UINT64_MAX, present_complete_semaphore, (VkFence)0, &current_buffer);
	assert(!err);

	const VkCommandBufferBeginInfo cmd_buf_info = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};
	const VkClearValue clear_values[1] = {
		[0] = {.color.float32 = {  100.f/255.f, 149.f/255.f, 237.f/255.f, .2f } },
	};

	const VkRenderPassBeginInfo rp_begin = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = vulkan->RenderPass,
		.framebuffer = vulkan->Buffers[current_buffer].fb,
		.renderArea.extent = vulkan->SwapchainExtent,
		.clearValueCount = 1,
		.pClearValues = clear_values,
	};

	err = vkBeginCommandBuffer(vulkan->DrawCmd, &cmd_buf_info);
	assert(!err);

	VkImageMemoryBarrier image_memory_barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		.image = vulkan->Buffers[current_buffer].image,
	};

	vkCmdPipelineBarrier(
		vulkan->DrawCmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0, 0, NULL, 0, NULL, 1, &image_memory_barrier);

	vkCmdBeginRenderPass(vulkan->DrawCmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdEndRenderPass(vulkan->DrawCmd);

	VkImageMemoryBarrier present_barrier = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
		.image = vulkan->Buffers[current_buffer].image,
	};

	vkCmdPipelineBarrier(
		vulkan->DrawCmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0, 0, NULL, 0, NULL, 1, &present_barrier);

	err = vkEndCommandBuffer(vulkan->DrawCmd);
	assert(!err);

	VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	VkSubmitInfo submit_info = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &vulkan->DrawCmd,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &present_complete_semaphore,
		.pWaitDstStageMask = &pipe_stage_flags,
	};

	err = vkQueueSubmit(vulkan->Queue, 1, &submit_info, VK_NULL_HANDLE);
	assert(!err);

	VkPresentInfoKHR present = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.swapchainCount = 1,
		.pSwapchains = &vulkan->Swapchain,
		.pImageIndices = &current_buffer,
	};
	err = vkQueuePresentKHR(vulkan->Queue, &present);
	if (err == VK_SUBOPTIMAL_KHR)
		eng_Log("warning: suboptimal present\n");
	else
		assert(!err);

	

	err = vkQueueWaitIdle(vulkan->Queue);
	assert(err == VK_SUCCESS);

	vkDestroySemaphore(vulkan->Device, present_complete_semaphore, NULL);
}


////////////////////////////////////////////////////////////////////////// Internal
