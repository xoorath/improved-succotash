#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

	////////////////////////////////////////////////////////////////////////// Lifecycle
	/**
	 * Window Malloc
	 *
	 * @note The window is not ready for use until eng_WindowInit is called.
	 * @return A newly allocated window.
	 */
	struct eng_Window* eng_WindowMalloc();

	/**
	* Window Init
	*
	* @description Initializes the window, making it ready for use.
	* @return true if initialization was successful.
	*/
	bool eng_WindowInit(struct eng_Window* window, unsigned width, unsigned height, const char* title);

	/**
	* Window Free
	*
	* Frees memory associated with the window. If subAllocationsOnly is
	* true, the window pointer itself will not be freed.
	*/
	void eng_WindowFree(struct eng_Window* window, bool subAllocationsOnly);


	/**
	* Window Get Sizeof
	*
	* @return the sizeof the internal eng_Window object, for use with
	* custom allocators.
	*/
	unsigned eng_WindowGetSizeof();

	////////////////////////////////////////////////////////////////////////// Window API
	/**
	 * Window Close
	 *
	 * Attempts to close the window if the window is open. Multiple calls to 
	 * this function are safe. Closing a window does not free it, however a 
	 * window cannot be shown once closed.
	 */
	void eng_WindowClose(struct eng_Window* window);

	/**
	 * Window Update
	 *
	 * This function should be called regularly for all windows, allowing the
	 * window to process messages.
	 * @returns true if at least one window is not in a closing state.
	 */
	bool eng_WindowUpdate(struct eng_Window* window, unsigned windowCount);
	
	/** @returns Currently set window title. */
	const char* eng_WindowGetTitle(struct eng_Window* window);
	/** @returns Currently set window width. */
	unsigned eng_WindowGetWidth(struct eng_Window* window);
	/** @returns Currently set window height. */
	unsigned eng_WindowGetHeight(struct eng_Window* window);

	/** Sets a new title for this window. */
	void eng_WindowSetTitle(struct eng_Window* window, const char* title);
	/** Sets a new size for this window. */
	void eng_WindowSetSize(struct eng_Window* window, unsigned width, unsigned height);

	////////////////////////////////////////////////////////////////////////// Callbacks
	/** Binds a function to the OnClose callback, triggered when this window is closing. */
	void eng_OnCloseBind(struct eng_Window* window, void(*OnClose)(void* UserData), void* UserData);
	/** Unbinds a function from the OnClose callback, triggered when this window is closing. */
	void eng_OnCloseUnbind(struct eng_Window* window, void(*OnClose)(void* UserData));

#ifdef __cplusplus
}
#endif