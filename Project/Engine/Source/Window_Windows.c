#ifdef GAME_WINDOWS
#define _CRT_SECURE_NO_WARNINGS

#include <Engine/Window.h>

#include <Engine/Array.h>
#include <Engine/Graphics_Vulkan.h>
#include <Engine/Log.h>

#include <ThirdParty/Vulkan/vulkan.h>
#include <ThirdParty/GLFW/glfw3.h>

#include <Engine/Graphics_VulkanInternal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct eng_WindowCallback
{
	eng_WindowCallback_t Func;
	void* UserData;
};

struct eng_Window
{
	GLFWwindow* Window;
	bool Closing;

	// display properties
	uint16_t Width, Height;
	char* Title;

	// callbacks
	eng_ArrayDecl(OnClose, struct eng_WindowCallback);
};

bool g_VulkanSupport = false;
const char** g_VulkanRequiredExtensions = NULL;
uint32_t g_VulkanRequiredExtensionCount = 0;

void eng_WindowHandleGLFWError(int errorCode, const char* description);

bool eng_WindowSetupValidate(struct eng_Window* window, uint16_t width, uint16_t height, const char* title);

void eng_WindowCallbackListBind(struct eng_Array* callbackList, eng_WindowCallback_t func, void* userData);
void eng_WindowCallbackListUnbind(struct eng_Array* callbackList, eng_WindowCallback_t func);
void eng_WindowCallbackListExec(struct eng_Array* callbackList);

////////////////////////////////////////////////////////////////////////// Lifecycle
bool eng_WindowGlobalInit(void)
{
	if (glfwInit())
	{

		g_VulkanSupport = glfwVulkanSupported();

		glfwSetErrorCallback(eng_WindowHandleGLFWError);

		g_VulkanRequiredExtensions = glfwGetRequiredInstanceExtensions(&g_VulkanRequiredExtensionCount);

#if !defined(GAME_FINAL)
		eng_Log("Using GLFW versioin %s\n", glfwGetVersionString());
		eng_Log("GLFW vulkan? %s\n", g_VulkanSupport ? "yes" : "no");
		for (uint32_t i = 0; i < g_VulkanRequiredExtensionCount; ++i)
		{
			eng_Log("GLFW Extension: [%s]\n", g_VulkanRequiredExtensions[i]);
		}
#endif

		return true;
	}
	return false;
}

void eng_WindowGlobalShutdown(void)
{
	glfwTerminate();
}

struct eng_Window* eng_WindowMalloc(void)
{
	return malloc(sizeof(struct eng_Window));
}

void eng_WindowFree(struct eng_Window* window, bool subAllocationsOnly)
{
	if (window == NULL)
	{
		return;
	}

	free(window->Title);
	
	eng_ArrayDestroy(&window->OnClose);

	if (!subAllocationsOnly)
	{
		free(window);
	}
}

bool eng_WindowInit(struct eng_Window* window, uint16_t width, uint16_t height, const char* title)
{
	if (!eng_WindowSetupValidate(window, width, height, title))
	{
		return false;
	}
	memset(window, 0, sizeof(struct eng_Window));

	eng_ArrayInitType(&window->OnClose, struct eng_WindowCallback);

	// hint to GLFW not to create opengl/opengles contexts.
	if (g_VulkanSupport) {
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	window->Window = glfwCreateWindow(width, height, title, NULL, NULL);

	return true;
}

size_t eng_WindowGetSizeof(void)
{
	return sizeof(struct eng_Window);
}

////////////////////////////////////////////////////////////////////////// Window API

void eng_WindowClose(struct eng_Window* window)
{
	if (!window->Closing)
	{
		window->Closing = true;
		eng_WindowCallbackListExec(&window->OnClose);
	}
}

void eng_WindowUpdate(struct eng_Window* window)
{
	if (glfwWindowShouldClose(window->Window))
	{
		eng_WindowClose(window);
	}
	else
	{
		glfwPollEvents();
	}
}

const char* eng_WindowGetTitle(struct eng_Window* window)
{
	return window->Title;
}

uint16_t eng_WindowGetWidth(struct eng_Window* window)
{
	return window->Width;
}

uint16_t eng_WindowGetHeight(struct eng_Window* window)
{
	return window->Height;
}

void eng_WindowSetTitle(struct eng_Window* window, const char* title)
{
	free(window->Title);
	window->Title = malloc(strlen(title) + 1);
	strcpy(window->Title, title);
}

void eng_WindowSetSize(struct eng_Window* window, uint16_t width, uint16_t height)
{
	window->Width = width;
	window->Height = height;
}

bool eng_WindowSupportsVulkan(struct eng_Window* window)
{
	return g_VulkanSupport;
}

bool eng_WindowBindVulkan(struct eng_Window* window, struct eng_Vulkan* vulkan)
{
	if (!eng_Ensure(g_VulkanSupport, "eng_WindowBindVulkan found no vulkan support. Call eng_WindowSupportsVulkan first to check."))
	{
		return false;
	}

	if (g_VulkanRequiredExtensionCount > 0)
	{
		eng_VulkanProvideExtensions(vulkan, g_VulkanRequiredExtensions, g_VulkanRequiredExtensionCount);
	}

	eng_VulkanCreateInstance(vulkan);

	VkSurfaceKHR surface;
	VkResult result = glfwCreateWindowSurface(eng_VulkanGetInstance(vulkan), window->Window, NULL, &surface);
	eng_VulkanEnsure(result, "create window surface");

	if (!eng_Ensure(eng_VulkanProvideSurface(vulkan, surface, window->Width, window->Height), "Failed to provide vulkan with glfw surface."))
	{
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////// Callbacks

void eng_OnCloseBind(struct eng_Window* window, eng_WindowCallback_t onClose, void* userData)
{
	eng_WindowCallbackListBind(&window->OnClose, onClose, userData);
}

void eng_OnCloseUnbind(struct eng_Window* window, eng_WindowCallback_t userData)
{
	eng_WindowCallbackListUnbind(&window->OnClose, userData);
}

////////////////////////////////////////////////////////////////////////// Internal

void eng_WindowHandleGLFWError(int errorCode, const char* description)
{
	eng_Err("GLFW Error(%d): \"%s\"\n", errorCode, description);
}

bool eng_WindowSetupValidate(struct eng_Window* window, uint16_t width, uint16_t height, const char* title)
{
#if !defined(GAME_FINAL)
	if (window == NULL)
	{
		eng_DevFatal("Invalid window");
		return false;
	}
	if (width == 0 || width >= 32767)
	{
		eng_DevFatal("Invalid width");
		return false;
	}
	if (height == 0 || height >= 32767)
	{
		eng_DevFatal("Invalid height");
		return false;
	}
	if (title == NULL)
	{
		eng_DevFatal("Invalid title.");
		return false;
	}
#endif

	return true;
}

void eng_WindowCallbackListBind(struct eng_Array* callbackList, eng_WindowCallback_t func, void* UserData)
{
	eng_ArrayResize(callbackList, callbackList->Count + 1);
	struct eng_WindowCallback* callback = eng_ArrayPIndexType(callbackList, struct eng_WindowCallback, callbackList->Count-1);
	callback->Func = func;
	callback->UserData = UserData;
}

// TODO: handle callbacks that might be in the list twice.
void eng_WindowCallbackListUnbind(struct eng_Array* callbackList, eng_WindowCallback_t func)
{
	for (uint32_t i = 0; i < callbackList->Count; ++i) {
		struct eng_WindowCallback* callback = eng_ArrayPIndexType(callbackList, struct eng_WindowCallback, i);
		if (callback->Func == func)
		{
			eng_ArrayRemoveLastSwap(callbackList, i);
			return;
		}
	}
}

void eng_WindowCallbackListExec(struct eng_Array* callbackList)
{
	struct eng_WindowCallback* begin = eng_ArrayBeginType(callbackList, struct eng_WindowCallback);
	struct eng_WindowCallback* end = eng_ArrayEndType(callbackList, struct eng_WindowCallback);
	for (; begin < end; ++begin)
	{
		begin->Func(begin->UserData);
	}
}

#endif