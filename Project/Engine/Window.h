#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#include <stdint.h>

	////////////////////////////////////////////////////////////////////////// Lifecycle
	/**
	* Window Global Init
	*
	* Call once and only once at application startup.
	* @returns true on successful initialization.
	*/
	bool eng_WindowGlobalInit();

	/**
	* Window Global Shutdown.
	*
	* Call one and only once at application shutdown.
	* @note initialization does not need to be successful for this call to be safe.
	*/
	void eng_WindowGlobalShutdown();

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
	bool eng_WindowInit(struct eng_Window* window, uint16_t width, uint16_t height, const char* title);

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
	size_t eng_WindowGetSizeof();

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
	 */
	void eng_WindowUpdate(struct eng_Window* window);
	
	/** @returns Currently set window title. */
	const char* eng_WindowGetTitle(struct eng_Window* window);
	/** @returns Currently set window width. */
	uint16_t eng_WindowGetWidth(struct eng_Window* window);
	/** @returns Currently set window height. */
	uint16_t eng_WindowGetHeight(struct eng_Window* window);

	/** Sets a new title for this window. */
	void eng_WindowSetTitle(struct eng_Window* window, const char* title);
	/** Sets a new size for this window. */
	void eng_WindowSetSize(struct eng_Window* window, uint16_t width, uint16_t height);

	/** @returns true if the window supports vulkan. */
	bool eng_WindowSupportsVulkan(struct eng_Window* window);
	/** @returns true if binding vulkan was a success. */
	bool eng_WindowBindVulkan(struct eng_Window* window, struct eng_Vulkan* vulkan);;

	////////////////////////////////////////////////////////////////////////// Callbacks
	
	typedef void(*eng_WindowCallback_t)(void*);

	/** Binds a function to the OnClose callback, triggered when this window is closing. */
	void eng_OnCloseBind(struct eng_Window* window, eng_WindowCallback_t OnClose, void* UserData);
	/** Unbinds a function from the OnClose callback, triggered when this window is closing. */
	void eng_OnCloseUnbind(struct eng_Window* window, eng_WindowCallback_t OnClose);

#ifdef __cplusplus
}
#endif