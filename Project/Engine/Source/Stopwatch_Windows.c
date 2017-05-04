#include <Engine/Stopwatch.h>

#include <stdlib.h>
#include <windows.h>

struct eng_Stopwatch
{
	LARGE_INTEGER Start, End, Freq;
	double Seconds;
};

#define SEC_TO_HR(s) ((s) * (1.0/3600.0))
#define SEC_TO_MN(s) ((s) * (1.0/60))
#define SEC_TO_MS(s) ((s) * 1e+3)
#define SEC_TO_US(s) ((s) * 1e+6)
#define SEC_TO_NS(s) ((s) * 1e+9)
#define SEC_TO_PS(s) ((s) * 1e+12)

////////////////////////////////////////////////////////////////////////// Lifecycle
struct eng_Stopwatch* eng_StopwatchMalloc()
{
	return malloc(sizeof(struct eng_Stopwatch));
}

bool eng_StopwatchInit(struct eng_Stopwatch* stopwatch)
{
	memset(stopwatch, 0, sizeof(struct eng_Stopwatch));
	return QueryPerformanceFrequency(&stopwatch->Freq) == TRUE;
}

void eng_StopwatchFree(struct eng_Stopwatch* stopwatch, bool subAllocationsOnly) {
	if (!subAllocationsOnly) {
		free(stopwatch);
	}
}

unsigned eng_StopwatchGetSizeof() {
	return sizeof(struct eng_Stopwatch);
}

////////////////////////////////////////////////////////////////////////// Stopwatch API
void eng_StopwatchStart(struct eng_Stopwatch* stopwatch) {
	QueryPerformanceCounter(&stopwatch->Start);
}

void eng_StopwatchStop(struct eng_Stopwatch* stopwatch) {
	QueryPerformanceCounter(&stopwatch->End);
	stopwatch->Seconds = (double)(stopwatch->End.QuadPart - stopwatch->Start.QuadPart) / (double)stopwatch->Freq.QuadPart;
}

double eng_StopwatchGetHours(struct eng_Stopwatch* stopwatch)
{
	return SEC_TO_HR((double)stopwatch->Seconds);
}

double eng_StopwatchGetMinutes(struct eng_Stopwatch* stopwatch)
{
	return SEC_TO_MN((double)stopwatch->Seconds);
}

double eng_StopwatchGetSeconds(struct eng_Stopwatch* stopwatch)
{
	return (double)stopwatch->Seconds;
}

double eng_StopwatchGetMilliseconds(struct eng_Stopwatch* stopwatch)
{
	return SEC_TO_MS((double)stopwatch->Seconds);
}

double eng_StopwatchGetMicroseconds(struct eng_Stopwatch* stopwatch)
{
	return SEC_TO_US((double)stopwatch->Seconds);
}

double eng_StopwatchGetNanoseconds(struct eng_Stopwatch* stopwatch)
{
	return SEC_TO_NS((double)stopwatch->Seconds);
}

double eng_StopwatchGetPicoseconds(struct eng_Stopwatch* stopwatch)
{
	return SEC_TO_PS((double)stopwatch->Seconds);
}