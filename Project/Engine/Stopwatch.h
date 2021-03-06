#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif
#include <stdint.h>

typedef struct eng_Stopwatch eng_Stopwatch;

////////////////////////////////////////////////////////////////////////// Lifecycle
/**
* Stopwatch Malloc
*
* @note The stopwatch is not ready for use until eng_StopwatchInit is 
* called.
* @return A newly allocated stopwatch.
*/
eng_Stopwatch* eng_StopwatchMalloc(void);

/**
* Stopwatch Init
*
* @description Initializes the stopwatch, making it ready for use.
* @return true if initialization was successful.
*/
bool eng_StopwatchInit(eng_Stopwatch* stopwatch);
	
/**
* Stopwatch Free
*
* Frees memory associated with the stopwatch. If subAllocationsOnly is 
* true, the stopwatch pointer itself will not be freed.
*/
void eng_StopwatchFree(eng_Stopwatch* stopwatch, bool subAllocationsOnly);

/**
* Stopwatch Get Sizeof
*
* @return the sizeof the internal eng_Stopwatch object, for use with
* custom allocators.
*/
unsigned eng_StopwatchGetSizeof(void);

////////////////////////////////////////////////////////////////////////// Stopwatch API
/**
* Stopwatch Start
*
* Starts the stopwatch.
*/
void eng_StopwatchStart(eng_Stopwatch* stopwatch);
/**
* Stopwatch Stop
*
* Stops the stopwatch, also caches the seconds elapsed internally.
*/
void eng_StopwatchStop(eng_Stopwatch* stopwatch);

/**
* Stopwatch to string
* Displays hours, minutes, seconds, milliseconds in the following format:
* [00:00:00:0000]
* 
* string length must be large enough for this format. ENG_STOPWATCH_TOSTRINGLEN
* can be used for convenience.
*/
void eng_StopwatchToString(eng_Stopwatch* stopwatch, char* str, size_t strLen);
#define ENG_STOPWATCH_TOSTRING_LEN  sizeof("[00:00:00:000]")
	
/** @returns the hours elapsed between stopwatch start and stop.  */
double eng_StopwatchGetHours(eng_Stopwatch* stopwatch);
	
/** @returns the minutes elapsed between stopwatch start and stop.  */
double eng_StopwatchGetMinutes(eng_Stopwatch* stopwatch);
	
/** @returns the hours seconds between stopwatch start and stop.  */
double eng_StopwatchGetSeconds(eng_Stopwatch* stopwatch);
	
/** @returns the hours milliseconds between stopwatch start and stop.  */
double eng_StopwatchGetMilliseconds(eng_Stopwatch* stopwatch);
	
/** @returns the hours microseconds between stopwatch start and stop.  */
double eng_StopwatchGetMicroseconds(eng_Stopwatch* stopwatch);
	
/** @returns the hours nanoseconds between stopwatch start and stop.  */
double eng_StopwatchGetNanoseconds(eng_Stopwatch* stopwatch);

/** @returns the hours picoseconds between stopwatch start and stop.  */
double eng_StopwatchGetPicoseconds(eng_Stopwatch* stopwatch);

#ifdef __cplusplus
}
#endif