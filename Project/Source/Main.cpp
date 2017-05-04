#include <stdio.h>
#include <stdlib.h>

#include <Engine/Window.h>
#include <Engine/Stopwatch.h>

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

int main(int argsc, char** argsv) {
	////////////////////////////////////////////////////////////////////////// Setup
	CoreSystemAllocator allocator;

	eng_Stopwatch* stopwatch = allocator.Malloc<eng_Stopwatch*>(eng_StopwatchGetSizeof());
	eng_Window* window = allocator.Malloc<eng_Window*>(eng_WindowGetSizeof());


	eng_StopwatchInit(stopwatch);
	eng_StopwatchStart(stopwatch);
	////////////////////////////////////////////////////////////////////////// Run
	if (eng_WindowInit(window, 1280, 720, "Improved Succotash"))
	{
		eng_WindowDbgPrint(window);
	}

	
	////////////////////////////////////////////////////////////////////////// Cleanup
	eng_StopwatchStop(stopwatch);
	printf("Window setup took %f seconds.", eng_StopwatchGetSeconds(stopwatch));

	eng_StopwatchFree(stopwatch, true);
	eng_WindowFree(window, true);
	window = nullptr;

	return 0;
}