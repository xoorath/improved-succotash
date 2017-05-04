#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

	////////////////////////////////////////////////////////////////////////// Lifecycle
	/**
	 * Stopwatch Malloc
	 *
	 * @note The stopwatch is not ready for use until eng_StopwatchInit is 
	 * called.
	 * @return A newly allocated stopwatch.
	 */
	struct eng_Stopwatch* eng_StopwatchMalloc();

	/**
	 * Stopwatch Init
	 *
	 * @description Initializes the stopwatch, making it ready for use.
	 * @return true if initialization was successful.
	 */
	bool eng_StopwatchInit(struct eng_Stopwatch* stopwatch);
	
	/**
	 * Stopwatch Free
	 *
	 * Frees memory associated with the stopwatch. If subAllocationsOnly is 
	 * true, the stopwatch pointer itself will not be freed.
	 */
	void eng_StopwatchFree(struct eng_Stopwatch* stopwatch, bool subAllocationsOnly);

	/**
	 * Stopwatch Get Sizeof
	 *
	 * @return the sizeof the internal eng_Stopwatch object, for use with
	 * custom allocators.
	 */
	unsigned eng_StopwatchGetSizeof();

	////////////////////////////////////////////////////////////////////////// Stopwatch API
	/**
	 * Stopwatch Start
	 *
	 * Starts the stopwatch.
	 */
	void eng_StopwatchStart(struct eng_Stopwatch* stopwatch);
	/**
	 * Stopwatch Stop
	 *
	 * Stops the stopwatch, also caches the seconds elapsed internally.
	 */
	void eng_StopwatchStop(struct eng_Stopwatch* stopwatch);
	
	/** @returns the hours elapsed between stopwatch start and stop.  */
	double eng_StopwatchGetHours(struct eng_Stopwatch* stopwatch);
	
	/** @returns the minutes elapsed between stopwatch start and stop.  */
	double eng_StopwatchGetMinutes(struct eng_Stopwatch* stopwatch);
	
	/** @returns the hours seconds between stopwatch start and stop.  */
	double eng_StopwatchGetSeconds(struct eng_Stopwatch* stopwatch);
	
	/** @returns the hours milliseconds between stopwatch start and stop.  */
	double eng_StopwatchGetMilliseconds(struct eng_Stopwatch* stopwatch);
	
	/** @returns the hours microseconds between stopwatch start and stop.  */
	double eng_StopwatchGetMicroseconds(struct eng_Stopwatch* stopwatch);
	
	/** @returns the hours nanoseconds between stopwatch start and stop.  */
	double eng_StopwatchGetNanoseconds(struct eng_Stopwatch* stopwatch);

	/** @returns the hours picoseconds between stopwatch start and stop.  */
	double eng_StopwatchGetPicoseconds(struct eng_Stopwatch* stopwatch);

#ifdef __cplusplus
}
#endif