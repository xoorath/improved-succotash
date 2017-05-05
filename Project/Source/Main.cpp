#include <crtdbg.h>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <Engine/Graphics_Vulkan.h>
#include <Engine/Log.h>
#include <Engine/Stopwatch.h>
#include <Engine/Url.h>
#include <Engine/Window.h>

#if defined(_MSC_VER)
#define VISUAL_STUDIO_LEAK_DETECTION 1
#endif

class CoreSystemAllocator {
	static constexpr unsigned CoreSystemMemorySize = 1024;
public:
	template<typename T>
	T Malloc(unsigned size) {
		if (CurrentOffset + size > CoreSystemMemorySize) {
			throw "Critical failure. Buffer size is not large enough for internal systems.";
		}
		void* next = CoreSystemMemory + CurrentOffset;
		CurrentOffset += size;;
		return static_cast<T>(next);
	}

private:
	char CoreSystemMemory[CoreSystemMemorySize];
	unsigned CurrentOffset = 0;
};

struct eng_Window;
struct eng_Stopwatch;
struct eng_Url;

void OnWindowClose(void*) 
{
	eng_Log("Window Close Detected\n");
}

int main(int argsc, char** argsv) {
#ifdef VISUAL_STUDIO_LEAK_DETECTION
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
	int exitCode = 0;
	////////////////////////////////////////////////////////////////////////// Setup
	CoreSystemAllocator allocator;

	eng_Stopwatch* stopwatch = allocator.Malloc<eng_Stopwatch*>(eng_StopwatchGetSizeof());
	eng_Window* window = nullptr;

	auto GracefullyExit = [&] (int exitCode)
	{
		eng_StopwatchFree(stopwatch, true);
		eng_WindowFree(window, true);
		eng_UrlGlobalShutdown();
		exit(exitCode);
	};
	
	////////////////////////////////////////////////////////////////////////// Setup
	eng_StopwatchInit(stopwatch);
	eng_StopwatchStart(stopwatch);

	if (!eng_Ensure(eng_UrlGlobalInit(), "Url global initialization failed."))
	{
		GracefullyExit(-1);
	}

	window = allocator.Malloc<eng_Window*>(eng_WindowGetSizeof());
	if (!eng_Ensure(eng_WindowInit(window, 1280, 720, "Improved Succotash"), "Application window failed to initialize."))
	{
		GracefullyExit(-1);
	}
	eng_OnCloseBind(window, OnWindowClose, nullptr);

	eng_StopwatchStop(stopwatch);
	eng_Log("Application setup took %f seconds.\n", eng_StopwatchGetSeconds(stopwatch));

	////////////////////////////////////////////////////////////////////////// Run
	eng_StopwatchStart(stopwatch);

	const char* urls[] = {"http://google.ca", "https://google.ca", "gibberish", "http://gibberish.notaurl"};
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

	while (eng_WindowUpdate(window, 1)) {

	}
	eng_StopwatchStop(stopwatch);
	double s = eng_StopwatchGetSeconds(stopwatch);
	double m = eng_StopwatchGetMinutes(stopwatch);
	if (s < 60)
	{
		eng_Log("Application ran for %f seconds.\n", s);
	}
	else
	{
		eng_Log("Application ran for %f minutes.\n", m);
	}

	////////////////////////////////////////////////////////////////////////// Cleanup
	GracefullyExit(0);
}