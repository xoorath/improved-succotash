#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#include <stdint.h>

	////////////////////////////////////////////////////////////////////////// Lifecycle
	/**
	 * Url Global Init
	 *
	 * Call once and only once at application startup.
	 * @returns true on successful initialization.
	 */
	bool eng_UrlGlobalInit();

	/**
	 * Url Global Shutdown.
	 *
	 * Call one and only once at application shutdown.
	 * @note initialization does not need to be successful for this call to be safe.
	 */
	void eng_UrlGlobalShutdown();

	/**
	 * Url Malloc
	 *
	 * @note The url is not ready for use until eng_UrlInit is called.
	 * @return A newly allocated url.
	 */
	struct eng_Url* eng_UrlMalloc();

	/**
	 * Url Init
	 *
	 * @description Initializes the url, making it ready for use.
	 * @return true if initialization was successful.
	 */
	bool eng_UrlInit(struct eng_Url* engUrl, const char* url);
	
	/**
	 * Url Free
	 *
	 * Frees memory associated with the url. If subAllocationsOnly is  true, 
	 * the url pointer itself will not be freed.
	 */
	void eng_UrlFree(struct eng_Url* engUrl, bool subAllocationsOnly);

	/**
	 * Url Get Sizeof
	 *
	 * @return the sizeof the internal eng_Url object, for use with custom
	 * allocators.
	 */
	size_t eng_UrlGetSizeof();

	////////////////////////////////////////////////////////////////////////// Url API

	/** @returns true if a connection can be established with the target url. */
	bool eng_UrlTestConnection(struct eng_Url* engUrl);

	/** @returns true if a connection can be established with the target url. */
	bool eng_UrlEasyTestConnection(const char* url);


#ifdef __cplusplus
}
#endif
