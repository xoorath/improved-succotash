#define _CRT_SECURE_NO_WARNINGS
#include <Engine/Stopwatch.h>

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <stdint.h>

typedef struct eng_Stopwatch
{
	LARGE_INTEGER Start, End, Freq;
	double Seconds;
} eng_Stopwatch;

#define SEC_TO_HR(s) ((s) * (1.0/3600.0))
#define SEC_TO_MN(s) ((s) * (1.0/60))
#define SEC_TO_MS(s) ((s) * 1e+3)
#define SEC_TO_US(s) ((s) * 1e+6)
#define SEC_TO_NS(s) ((s) * 1e+9)
#define SEC_TO_PS(s) ((s) * 1e+12)

////////////////////////////////////////////////////////////////////////// Lifecycle
eng_Stopwatch* eng_StopwatchMalloc(void)
{
	return malloc(sizeof(eng_Stopwatch));
}

bool eng_StopwatchInit(eng_Stopwatch* stopwatch)
{
	memset(stopwatch, 0, sizeof(eng_Stopwatch));
	return QueryPerformanceFrequency(&stopwatch->Freq) == TRUE;
}

void eng_StopwatchFree(eng_Stopwatch* stopwatch, bool subAllocationsOnly) 
{
	if (stopwatch == NULL)
	{
		return;
	}

	if (!subAllocationsOnly) 
	{
		free(stopwatch);
	}
}

unsigned eng_StopwatchGetSizeof(void)
{
	return sizeof(eng_Stopwatch);
}

////////////////////////////////////////////////////////////////////////// Stopwatch API
void eng_StopwatchStart(eng_Stopwatch* stopwatch) 
{
	QueryPerformanceCounter(&stopwatch->Start);
}

void eng_StopwatchStop(eng_Stopwatch* stopwatch) 
{
	QueryPerformanceCounter(&stopwatch->End);
	stopwatch->Seconds = (double)(stopwatch->End.QuadPart - stopwatch->Start.QuadPart) / (double)stopwatch->Freq.QuadPart;
}

void eng_StopwatchToString(eng_Stopwatch* stopwatch, char* str, size_t strLen)
{
	char buffer[ENG_STOPWATCH_TOSTRING_LEN] = { 0 };
	int32_t h = (int32_t)eng_StopwatchGetHours(stopwatch);
	int32_t m = (int32_t)eng_StopwatchGetMinutes(stopwatch) % 60;
	int32_t s = (int32_t)eng_StopwatchGetSeconds(stopwatch) % 60;
	int32_t mi = (int32_t)eng_StopwatchGetMilliseconds(stopwatch) % 1000;

	sprintf(buffer, "[%02d:%02d:%02d:%03d]", h % 100, m % 100, s % 100, mi % 1000);

	size_t bufferLen = strlen(buffer);
	if (bufferLen <= strLen)
	{
		memcpy(str, buffer, bufferLen+1);
	}
}

double eng_StopwatchGetHours(eng_Stopwatch* stopwatch)
{
	return SEC_TO_HR((double)stopwatch->Seconds);
}

double eng_StopwatchGetMinutes(eng_Stopwatch* stopwatch)
{
	return SEC_TO_MN((double)stopwatch->Seconds);
}

double eng_StopwatchGetSeconds(eng_Stopwatch* stopwatch)
{
	return (double)stopwatch->Seconds;
}

double eng_StopwatchGetMilliseconds(eng_Stopwatch* stopwatch)
{
	return SEC_TO_MS((double)stopwatch->Seconds);
}

double eng_StopwatchGetMicroseconds(eng_Stopwatch* stopwatch)
{
	return SEC_TO_US((double)stopwatch->Seconds);
}

double eng_StopwatchGetNanoseconds(eng_Stopwatch* stopwatch)
{
	return SEC_TO_NS((double)stopwatch->Seconds);
}

double eng_StopwatchGetPicoseconds(eng_Stopwatch* stopwatch)
{
	return SEC_TO_PS((double)stopwatch->Seconds);
}