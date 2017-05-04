#pragma once

#define WINDOWS_MAX 4

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

	////////////////////////////////////////////////////////////////////////// Lifecycle
	struct eng_Window* eng_WindowMalloc();
	bool eng_WindowInit(struct eng_Window* window, unsigned width, unsigned height, const char* title);
	void eng_WindowFree(struct eng_Window* window, bool subAllocationsOnly);

	unsigned eng_WindowGetSizeof();

	////////////////////////////////////////////////////////////////////////// Window API
	void eng_WindowClose(struct eng_Window* window);
	
	const char* eng_WindowGetTitle(struct eng_Window* window);
	unsigned eng_WindowGetWidth(struct eng_Window* window);
	unsigned eng_WindowGetHeight(struct eng_Window* window);

	void eng_WindowSetTitle(struct eng_Window* window, const char* title);
	void eng_WindowSetSize(struct eng_Window* window, unsigned width, unsigned height);

	////////////////////////////////////////////////////////////////////////// Callbacks
	void eng_OnCloseBind(struct eng_Window* window, void(*on_close)(void* user_data), void* user_data);
	void eng_OnCloseUnbind(struct eng_Window* window, void(*on_close)(void* user_data));

	////////////////////////////////////////////////////////////////////////// Debug
	void eng_WindowDbgPrint(struct eng_Window* window);

#ifdef __cplusplus
}
#endif