#include <crtdbg.h>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <Engine/Graphics_Vulkan.h>
#include <Engine/Ini.h>
#include <Engine/Log.h>
#include <Engine/Stopwatch.h>
#include <Engine/Url.h>
#include <Engine/Window.h>

#if defined(_MSC_VER)
#define VISUAL_STUDIO_LEAK_DETECTION 1
#endif

class CoreSystemAllocator {
public:
	static constexpr unsigned CoreSystemMemorySize = 1024;

	template<typename T>
	T Malloc(size_t size) {
		if (CurrentOffset + size > CoreSystemMemorySize) {
			__debugbreak();
			throw "Critical failure. Buffer size is not large enough for internal systems.";
		}
		void* next = CoreSystemMemory + CurrentOffset;
		CurrentOffset += size;;
		return static_cast<T>(next);
	}

	size_t GetCurrentOffset() const {
		return CurrentOffset;
	}

private:
	char CoreSystemMemory[CoreSystemMemorySize];
	size_t CurrentOffset = 0;
};

struct eng_Window;
struct eng_Stopwatch;
struct eng_Url;
struct eng_IniR;

volatile bool ApplicationRunning = true;

void OnWindowClose(void*) 
{
	eng_Log("Window Closing\n");
	ApplicationRunning = false;
}

int main(int argsc, char** argsv) {
#ifdef VISUAL_STUDIO_LEAK_DETECTION
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	int exitCode = 0;
	////////////////////////////////////////////////////////////////////////// Setup
	CoreSystemAllocator allocator;

	char buffer[ENG_STOPWATCH_TOSTRING_LEN];
	eng_Stopwatch* stopwatch = allocator.Malloc<eng_Stopwatch*>(eng_StopwatchGetSizeof());
	eng_Window* window = nullptr;
	eng_Vulkan* vulkan = nullptr;
	eng_IniR* ini = nullptr;

	auto GracefullyExit = [&] (int exitCode)
	{
		eng_StopwatchFree(stopwatch, true);
		eng_VulkanFree(vulkan, true);
		eng_WindowFree(window, true);
		eng_WindowGlobalShutdown();
		eng_UrlGlobalShutdown();
		eng_IniRFree(ini, true);
		return exitCode;
	};

	////////////////////////////////////////////////////////////////////////// Setup
	eng_StopwatchInit(stopwatch);
	eng_StopwatchStart(stopwatch);

	ini = allocator.Malloc<eng_IniR*>(eng_IniRGetSizeof());
	if (!eng_Ensure(eng_IniRInit(ini, "engine.ini"), "Failed to load engine.ini"))
	{
		return GracefullyExit(-1);
	}

	if (!eng_Ensure(eng_UrlGlobalInit(), "Url global initialization failed."))
	{
		return GracefullyExit(-1);
	}

	if (!eng_Ensure(eng_WindowGlobalInit(), "Window global initialization failed."))
	{
		return GracefullyExit(-1);
	}

	window = allocator.Malloc<eng_Window*>(eng_WindowGetSizeof());
	if (!eng_Ensure(eng_WindowInit(window, 1280, 720, "Improved Succotash"), "Application window failed to initialize."))
	{
		return GracefullyExit(-1);
	}
	eng_OnCloseBind(window, OnWindowClose, nullptr);

	if (eng_WindowSupportsVulkan(window)) 
	{
		vulkan = allocator.Malloc<eng_Vulkan*>(eng_VulkanGetSizeof());
		if (!eng_Ensure(eng_VulkanInit(vulkan), "Vulkan initialization failed."))
		{
			return GracefullyExit(-1);
		}
		if (!eng_Ensure(eng_WindowBindVulkan(window, vulkan), "Failed to bind window with vulkan"))
		{
			return GracefullyExit(-1);
		}
	}
	else
	{
		eng_Ensure(false, "No supported graphics API found.");
		return GracefullyExit(-1);
	}

	const char* urls[] = {"http://google.ca" /*, "https://google.ca", "gibberish", "http://gibberish.notaurl" */};
	for (auto& url : urls)
	{
		if (eng_UrlEasyTestConnection(url))
		{
			eng_Log("Connected to: %s\n", url);
		}
		else
		{
			eng_Log("Not connected to: %s\n", url);
		}
	}

	eng_StopwatchStop(stopwatch);
	eng_StopwatchToString(stopwatch, buffer, sizeof(buffer));
	eng_Log("Application setup took: %s\n", buffer);
	////////////////////////////////////////////////////////////////////////// Run
	eng_StopwatchStart(stopwatch);

	while (ApplicationRunning) {
		eng_WindowUpdate(window);
	}
	eng_StopwatchStop(stopwatch);

	eng_StopwatchToString(stopwatch, buffer, sizeof(buffer));
	eng_Log("Application ran for: %s\n", buffer);

	eng_Log("Core systems used %d/%d bytes of available memory.\n", allocator.GetCurrentOffset(), CoreSystemAllocator::CoreSystemMemorySize);

	////////////////////////////////////////////////////////////////////////// Cleanup
	return GracefullyExit(0);
}