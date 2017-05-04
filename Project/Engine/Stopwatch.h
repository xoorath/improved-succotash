#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

	////////////////////////////////////////////////////////////////////////// Lifecycle
	struct eng_Stopwatch* eng_StopwatchMalloc();
	bool eng_StopwatchInit(struct eng_Stopwatch* stopwatch);
	void eng_StopwatchFree(struct eng_Stopwatch* stopwatch, bool subAllocationsOnly);

	unsigned eng_StopwatchGetSizeof();

	////////////////////////////////////////////////////////////////////////// Stopwatch API
	void eng_StopwatchStart(struct eng_Stopwatch* stopwatch);
	void eng_StopwatchStop(struct eng_Stopwatch* stopwatch);
	
	double eng_StopwatchGetHours(struct eng_Stopwatch* stopwatch);
	double eng_StopwatchGetMinutes(struct eng_Stopwatch* stopwatch);
	double eng_StopwatchGetSeconds(struct eng_Stopwatch* stopwatch);
	double eng_StopwatchGetMilliseconds(struct eng_Stopwatch* stopwatch);
	double eng_StopwatchGetMicroseconds(struct eng_Stopwatch* stopwatch);
	double eng_StopwatchGetNanoseconds(struct eng_Stopwatch* stopwatch);
	double eng_StopwatchGetPicoseconds(struct eng_Stopwatch* stopwatch);

#ifdef __cplusplus
}
#endif