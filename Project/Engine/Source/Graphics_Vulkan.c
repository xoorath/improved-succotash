#include <Engine/Graphics_Vulkan.h>

#include <Engine/Array.h>
#include <Engine/Log.h>

#if defined(GAME_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <ThirdParty/Vulkan/vulkan.h>

#include <Engine/Graphics_VulkanInternal.h>

struct eng_Vulkan {
	// Handles
	VkInstance Instance;
	VkPhysicalDevice PhysicalDevice;
	VkDevice LogicalDevice;
	VkSurfaceKHR Surface;
	
	// Hardware
	uint32_t PhysicalDevicesCount;
	VkPhysicalDevice* PhysicalDevices;

	// Queue Data
	uint32_t QueueCount;
	float* QueuePriorities;
	uint32_t QueueFamilyIndex;
	VkCommandPool CommandPool;
	VkCommandBuffer Command;

	// Surface Data
	uint16_t Width, Height;
	uint32_t PresentFamilyIndex;
	VkFormat SurfaceFormat;
	VkColorSpaceKHR SurfaceColorSpace;
	VkPresentModeKHR PresentMode;
	VkSurfaceCapabilitiesKHR SurfaceCapabilities;

	// Setup Data
	struct eng_Array ExtensionsList;

	// Configured requirements
	bool RequiresGraphics : 1;
	bool RequiresCompute : 1;
	bool RequiresPresent : 1;
};

bool eng_VulkanCreateInstanceInternal(struct eng_Vulkan* vulkan);
bool eng_VulkanEnumerateDevices(struct eng_Vulkan* vulkan);
bool eng_VulkanSelectDevice(struct eng_Vulkan* vulkan);
bool eng_VulkanCreateLogicalDevice(struct eng_Vulkan* vulkan);
bool eng_VulkanCreateCommandBuffer(struct eng_Vulkan* vulkan);
bool eng_VulkanDetermineDeviceSurfaceCapability(struct eng_Vulkan* vulkan);
bool eng_VulkanCreateSwapchain(struct eng_Vulkan* vulkan);

////////////////////////////////////////////////////////////////////////// Lifecycle

struct eng_Vulkan* eng_VulkanMalloc() {
	return malloc(sizeof(struct eng_Vulkan));
}

bool eng_VulkanInit(struct eng_Vulkan* vulkan) {
	memset(vulkan, 0, sizeof(struct eng_Vulkan));

	vulkan->RequiresGraphics = true;
	vulkan->RequiresCompute = false;
	vulkan->RequiresPresent = false;

	eng_ArrayInit(&vulkan->ExtensionsList, sizeof(char*));

	return true;
}

void eng_VulkanFree(struct eng_Vulkan* vulkan, bool subAllocationsOnly) {
	if (vulkan == NULL)
	{
		return;
	}

	char** extensionsItterator = eng_ArrayBeginType(&vulkan->ExtensionsList, char*);
	char** extensionsEnd = eng_ArrayEndType(&vulkan->ExtensionsList, char*);
	for (; extensionsItterator < extensionsEnd; ++extensionsItterator)
	{
		free(*extensionsItterator);
	}
	eng_ArrayDestroy(&vulkan->ExtensionsList);

	free(vulkan->QueuePriorities);
	free(vulkan->PhysicalDevices);

	vkDestroySurfaceKHR(vulkan->Instance, vulkan->Surface, NULL);
	
	if (vulkan->LogicalDevice)
	{
		VkCommandBuffer cmd_bufs[] = { vulkan->Command };
		vkFreeCommandBuffers(vulkan->LogicalDevice, vulkan->CommandPool, 1, cmd_bufs);
		vkDestroyCommandPool(vulkan->LogicalDevice, vulkan->CommandPool, NULL);
		vkDestroyDevice(vulkan->LogicalDevice, NULL);

	}
	vkDestroyInstance(vulkan->Instance, NULL);

	if (!subAllocationsOnly) {
		free(vulkan);
	}
}

size_t eng_VulkanGetSizeof() {
	return sizeof(struct eng_Vulkan);
}

////////////////////////////////////////////////////////////////////////// Configuration API

void eng_VulkanProvideExtensions(struct eng_Vulkan* vulkan, const char** extensions, uint32_t extensionsCount)
{
	for (uint32_t i = 0; i < extensionsCount; ++i)
	{
		size_t len = strlen(extensions[i]);
		char* copy = malloc(len + 1);
		memcpy(copy, extensions[i], len + 1);
		eng_ArrayPushBack(&vulkan->ExtensionsList, &copy);
	}
}


void eng_VulkanSetRequiresCompute(struct eng_Vulkan* vulkan, bool requiresCompute)
{
	vulkan->RequiresCompute = requiresCompute;
}

void eng_VulkanSetRequiresGraphics(struct eng_Vulkan* vulkan, bool requiresGraphics)
{
	vulkan->RequiresGraphics = requiresGraphics;
}

void eng_VulkanSetRequiresPresent(struct eng_Vulkan* vulkan, bool requiresPresent)
{
	vulkan->RequiresPresent = requiresPresent;
}

////////////////////////////////////////////////////////////////////////// API
bool eng_VulkanCreateInstance(struct eng_Vulkan* vulkan)
{
	bool(*stages[])(struct eng_Vulkan*) = {
		eng_VulkanCreateInstanceInternal,
		eng_VulkanEnumerateDevices,
		eng_VulkanSelectDevice,
		eng_VulkanCreateLogicalDevice,
		eng_VulkanCreateCommandBuffer
	};

	const size_t count = sizeof(stages) / sizeof(stages[0]);
	for (size_t i = 0; i < count; ++i) {
		if (!stages[i](vulkan))
		{
			return false;
		}
	}
	return true;
}

bool eng_VulkanProvideSurface(struct eng_Vulkan* vulkan, VkSurfaceKHR surface, uint16_t width, uint16_t height)
{
	vulkan->Surface = surface;
	vulkan->Width = width;
	vulkan->Height = height;
	vulkan->RequiresPresent = true;

	bool(*stages[])(struct eng_Vulkan*) = {
		eng_VulkanDetermineDeviceSurfaceCapability,
		eng_VulkanCreateSwapchain
	};

	const size_t count = sizeof(stages) / sizeof(stages[0]);
	for (size_t i = 0; i < count; ++i) {
		if (!stages[i](vulkan))
		{
			return false;
		}
	}

	return true;
}

VkInstance eng_VulkanGetInstance(struct eng_Vulkan* vulkan) 
{
	return vulkan->Instance;
}


////////////////////////////////////////////////////////////////////////// Internal

bool eng_VulkanCreateInstanceInternal(struct eng_Vulkan* vulkan)
{
	struct VkApplicationInfo appInfo = { 0 };
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = NULL;
	appInfo.pApplicationName = "Improved Succotash Demo";
	appInfo.applicationVersion = 0;
	appInfo.pEngineName = "Improved Succotash";
	appInfo.engineVersion = 0;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	struct VkInstanceCreateInfo createInfo = { 0 };
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledLayerCount = 0;
	createInfo.ppEnabledLayerNames = NULL;
	
	createInfo.enabledExtensionCount = vulkan->ExtensionsList.Count;
	createInfo.ppEnabledExtensionNames = vulkan->ExtensionsList.Buffer;

	VkResult result = vkCreateInstance(&createInfo, NULL, &vulkan->Instance);
	eng_VulkanEnsure(result, "create vulkan instance");

	return true;
}

bool eng_VulkanEnumerateDevices(struct eng_Vulkan* vulkan) 
{
	VkResult result = vkEnumeratePhysicalDevices(vulkan->Instance, &vulkan->PhysicalDevicesCount, NULL);
	eng_VulkanEnsure(result, "enumerate physical devices");

	if (!eng_Ensure(vulkan->PhysicalDevicesCount != 0, "No GPU was found.")) {
		return false;
	}

	vulkan->PhysicalDevices = calloc(vulkan->PhysicalDevicesCount, sizeof(VkPhysicalDevice));
	memset(vulkan->PhysicalDevices, 0, vulkan->PhysicalDevicesCount*sizeof(VkPhysicalDevice));
	result = vkEnumeratePhysicalDevices(vulkan->Instance, &vulkan->PhysicalDevicesCount, vulkan->PhysicalDevices);
	eng_VulkanEnsure(result, "enumerate physical devices");

	return true;
}

// TODO: Currently device selection finds an discrete device if there is one, and then 
// prioritizes highest memory. This criteria considers all non-discrete devices equal
// and should likely re-prioritize each of these non-discrete devices.
// A better performance metric than memory may also be used if possible.
bool eng_VulkanSelectDevice(struct eng_Vulkan* vulkan)
{
	VkQueueFamilyProperties* families;
	uint32_t familiesCount;

	vulkan->PhysicalDevice = 0; // Start off with no selected device.
	uint32_t bestDeviceQueueCount = 0;

	// Tohelp us in the device selection process.
	bool hasFoundDescreteGpu = false;
	VkDeviceSize descreteGpuHighestMem = 0;
	VkDeviceSize integratedGpuHighestMem = 0;

	for (uint32_t i = 0; i < vulkan->PhysicalDevicesCount; ++i) {
		VkPhysicalDevice device = vulkan->PhysicalDevices[i];
		
		vkGetPhysicalDeviceQueueFamilyProperties(device, &familiesCount, NULL);
		if (familiesCount == 0)
		{
			continue;
		}

		families = calloc(familiesCount, sizeof(VkQueueFamilyProperties));
		vkGetPhysicalDeviceQueueFamilyProperties(device, &familiesCount, families);


		VkQueueFamilyProperties* selectedFamily = NULL;
		uint32_t familyIndex;
		uint32_t presentFamilyIndex = UINT32_MAX;
		for (familyIndex = 0; familyIndex < familiesCount; ++familyIndex)
		{
			if (vulkan->RequiresPresent)
			{
				VkBool32 supportsPresent;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, familyIndex, vulkan->Surface, &supportsPresent);
				if (!supportsPresent)
				{
					presentFamilyIndex = familyIndex;
				}
			}

			if (vulkan->RequiresGraphics && !(families[familyIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT))
			{
				continue;
			}
			
			if (vulkan->RequiresCompute && !(families[familyIndex].queueFlags & VK_QUEUE_COMPUTE_BIT))
			{
				continue;
			}

			selectedFamily = &families[familyIndex];
			break;
		}


		if (selectedFamily == NULL)
		{
			free(families);
			continue;
		}

		bool isDiscrete = false;
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(device, &properties);
		isDiscrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

		// Don't bother, we already have a discrete device to pick from.
		if (hasFoundDescreteGpu && !isDiscrete)
		{
			free(families);
			continue;
		}
		hasFoundDescreteGpu = isDiscrete;
		
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(device, &memProperties);
		VkDeviceSize mem = 0;
		for (uint32_t j = 0; j < memProperties.memoryHeapCount; ++j) 
		{
			mem += memProperties.memoryHeaps[j].size;
		}

		bool isBestDevice = false;
		if (isDiscrete)
		{
			if (mem > descreteGpuHighestMem)
			{
				descreteGpuHighestMem = mem;
				isBestDevice = true;
			}
		}
		else
		{
			if (mem > integratedGpuHighestMem)
			{
				integratedGpuHighestMem = mem;
				isBestDevice = true;
			}
		}

		if (isBestDevice && (!vulkan->RequiresPresent || presentFamilyIndex != UINT32_MAX))
		{
			vulkan->PhysicalDevice = device;
			vulkan->QueueFamilyIndex = familyIndex;
			bestDeviceQueueCount = selectedFamily->queueCount;
		}

		free(families);
	}

	if (!eng_Ensure(vulkan->PhysicalDevice, "No suitable vulkan device found.\n"))
	{
		return false;
	}

	// HACK: For simplicity while setting up and testing vulkan, I've set the queue to one.
	// TODO: Play with using the available queues, and see the impact around designing for
	// different queue values. Investigate the implications and make an informed decision.
	bestDeviceQueueCount = 1;

	vulkan->QueueCount = bestDeviceQueueCount;
	vulkan->QueuePriorities = calloc(bestDeviceQueueCount, sizeof(float));
	memset(vulkan->QueuePriorities, 0, bestDeviceQueueCount * sizeof(float));

	return true;
}

bool eng_VulkanCreateLogicalDevice(struct eng_Vulkan* vulkan)
{
	VkDeviceQueueCreateInfo queueInfo;
	queueInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	queueInfo.pNext = NULL;
	queueInfo.queueCount = vulkan->QueueCount;
	queueInfo.pQueuePriorities = vulkan->QueuePriorities;

	VkDeviceCreateInfo deviceInfo;
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = NULL;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledExtensionCount = 0;
	deviceInfo.ppEnabledExtensionNames = NULL;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = NULL;
	deviceInfo.pEnabledFeatures = NULL;

	VkResult result = vkCreateDevice(vulkan->PhysicalDevice, &deviceInfo, NULL, &vulkan->LogicalDevice);
	eng_VulkanEnsure(result, "create logical device");
	return true;
}

bool eng_VulkanCreateCommandBuffer(struct eng_Vulkan* vulkan)
{
	VkCommandPoolCreateInfo poolInfo;
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.pNext = NULL;
	poolInfo.queueFamilyIndex = vulkan->QueueFamilyIndex;
	poolInfo.flags = 0;
	
	VkResult result = vkCreateCommandPool(vulkan->LogicalDevice, &poolInfo, NULL, &vulkan->CommandPool);
	eng_VulkanEnsure(result, "create command pool");

	/* Create the command buffer from the command pool */
	VkCommandBufferAllocateInfo poolAllocateInfo;
	poolAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	poolAllocateInfo.pNext = NULL;
	poolAllocateInfo.commandPool = vulkan->CommandPool;
	poolAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	poolAllocateInfo.commandBufferCount = 1;

	result = vkAllocateCommandBuffers(vulkan->LogicalDevice, &poolAllocateInfo, &vulkan->Command);
	eng_VulkanEnsure(result, "allocate command buffers");

	return true;
}

bool eng_VulkanDetermineDeviceSurfaceCapability(struct eng_Vulkan* vulkan)
{
	uint32_t formatCount;
	VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan->PhysicalDevice, vulkan->Surface, &formatCount, NULL);
	eng_VulkanEnsure(result, "get physical device surface formats");

	VkSurfaceFormatKHR *surfFormats = calloc(formatCount, sizeof(VkSurfaceFormatKHR));
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkan->PhysicalDevice, vulkan->Surface, &formatCount, surfFormats);
	eng_VulkanEnsure(result, "get physical device surface formats");

	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		vulkan->SurfaceFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else
	{
		// TODO: Should we be selecting a better surface format based on some criteria?
		vulkan->SurfaceFormat = surfFormats[0].format;
	}
	vulkan->SurfaceColorSpace = surfFormats[0].colorSpace;

	eng_Log("Vulkan surface format selected: %s\n", eng_InternalVKFormatToString(vulkan->SurfaceFormat));

	free(surfFormats);

	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkan->PhysicalDevice, vulkan->Surface, &vulkan->SurfaceCapabilities);
	eng_VulkanEnsure(result, "get surface capabilities");

	VkExtent2D swapchainExtent = { (uint32_t)vulkan->Width, (uint32_t)vulkan->Height };
	// width and height are either both 0xffffffff, or both not 0xffffffff
	if (vulkan->SurfaceCapabilities.currentExtent.width == 0xffffffff)
	{
		if (swapchainExtent.width < vulkan->SurfaceCapabilities.minImageExtent.width)
		{
			eng_Warn("The graphics card can't render to a surface with the size we requested. "
					"Requested width: %d, minimum width: %d\n", 
					swapchainExtent.width, vulkan->SurfaceCapabilities.minImageExtent.width);
			swapchainExtent.width = vulkan->SurfaceCapabilities.minImageExtent.width;
		}
		else if (swapchainExtent.width > vulkan->SurfaceCapabilities.maxImageExtent.width)
		{
			eng_Warn("The graphics card can't render to a surface with the size we requested. "
					"Requested width: %d, maximum width: %d\n", 
					swapchainExtent.width, vulkan->SurfaceCapabilities.minImageExtent.width);
			swapchainExtent.width = vulkan->SurfaceCapabilities.minImageExtent.width;
		}

		if (swapchainExtent.height < vulkan->SurfaceCapabilities.minImageExtent.height)
		{
			eng_Warn("The graphics card can't render to a surface with the size we requested. "
					"Requested height: %d, minimum height: %d\n", 
					swapchainExtent.height, vulkan->SurfaceCapabilities.minImageExtent.height);
			swapchainExtent.height = vulkan->SurfaceCapabilities.minImageExtent.height;
		}
		else if (swapchainExtent.height > vulkan->SurfaceCapabilities.maxImageExtent.height)
		{
			eng_Warn("The graphics card can't render to a surface with the size we requested. "
					"Requested height: %d, maximum height: %d\n", 
					swapchainExtent.height, vulkan->SurfaceCapabilities.minImageExtent.height);
			swapchainExtent.height = vulkan->SurfaceCapabilities.minImageExtent.height;
		}
	}
	else
	{
		swapchainExtent = vulkan->SurfaceCapabilities.currentExtent;
	}

	if (swapchainExtent.width > UINT16_MAX)
	{
		eng_Err("Swapchain extent width is larger than uint16 max. Our renderer isn't configured to handle that.");
		swapchainExtent.width = UINT16_MAX;
	}

	if (swapchainExtent.height > UINT16_MAX)
	{
		eng_Err("Swapchain extent height is larger than uint16 max. Our renderer isn't configured to handle that.");
		swapchainExtent.height = UINT16_MAX;
	}

	vulkan->Width = (uint32_t)swapchainExtent.width;
	vulkan->Height = (uint32_t)swapchainExtent.height;

	// The FIFO present mode is guaranteed by the spec to be supported
	// and to have no tearing.  It's a great default present mode to use.
	vulkan->PresentMode = VK_PRESENT_MODE_FIFO_KHR;

	// Currently we take no action to select a preferred present mode.
	/*
	uint32_t presentModesCount;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan->PhysicalDevice, vulkan->Surface, &presentModesCount, NULL);
	eng_VulkanEnsure("get physical device surface present modes");

	VkPresentModeKHR* presentModes = calloc(presentModesCount, sizeof(VkPresentModeKHR));
	result = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkan->PhysicalDevice, vulkan->Surface, &presentModesCount, presentModes);
	eng_VulkanEnsure("get physical device surface present modes");
	*/

	return true;
}

bool eng_VulkanCreateSwapchain(struct eng_Vulkan* vulkan)
{
	uint32_t desiredNumberOfSwapchainImages = 3;

	if (desiredNumberOfSwapchainImages < vulkan->SurfaceCapabilities.minImageCount)
	{
		eng_Err("Desired number of swapchain images is lower than the minimum ammount. Desired: %d, minimum: %d", 
				desiredNumberOfSwapchainImages,
				vulkan->SurfaceCapabilities.minImageCount);
		desiredNumberOfSwapchainImages = vulkan->SurfaceCapabilities.minImageCount;
	}
	else if (desiredNumberOfSwapchainImages > vulkan->SurfaceCapabilities.maxImageCount)
	{
		eng_Err("Desired number of swapchain images is higher than the maximum ammount. Desired: %d, maximum: %d", 
				desiredNumberOfSwapchainImages,
				vulkan->SurfaceCapabilities.minImageCount);
		desiredNumberOfSwapchainImages = vulkan->SurfaceCapabilities.maxImageCount;
	}

	eng_Log("Vulkan using %d images in swapchain. min: %d, max: %d\n", 
			desiredNumberOfSwapchainImages,
			vulkan->SurfaceCapabilities.minImageCount,
			vulkan->SurfaceCapabilities.maxImageCount);

	VkSurfaceTransformFlagsKHR preTransform;
	if (vulkan->SurfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = vulkan->SurfaceCapabilities.currentTransform;
	}

	// Find a supported composite alpha mode - one of these is guaranteed to be set
	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	};

	for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); i++)
	{
		if (vulkan->SurfaceCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
		{
			compositeAlpha = compositeAlphaFlags[i];
			break;
		}
	}

	VkSwapchainCreateInfoKHR swapchainInfo;
	
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.pNext = NULL;
	swapchainInfo.surface = vulkan->Surface;
	swapchainInfo.minImageCount = desiredNumberOfSwapchainImages;
	swapchainInfo.imageFormat = vulkan->SurfaceFormat;
	swapchainInfo.imageColorSpace = vulkan->SurfaceColorSpace;
	swapchainInfo.imageExtent.width = (uint32_t)vulkan->Width;
	swapchainInfo.imageExtent.height = (uint32_t)vulkan->Height;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainInfo.preTransform = preTransform;
	swapchainInfo.compositeAlpha = compositeAlpha;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainInfo.queueFamilyIndexCount = 0;
	swapchainInfo.pQueueFamilyIndices = NULL;
	swapchainInfo.presentMode = vulkan->PresentMode;
	// swapchainInfo.oldSwapchain = NULL; // TODO?
	swapchainInfo.clipped = true;

	return true;
}