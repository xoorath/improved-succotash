#define _CRT_SECURE_NO_WARNINGS
#include <Engine/Log.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include <Windows.h>
#endif

void eng_LogInternal(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	static char buffer[2048];

	if (vsprintf(buffer, fmt, args) < 0)
	{
		printf("[ERROR] Internal error: log buffer not big enough for the log we're printing.");
		exit(-1);
	}

	va_end(args);

#if defined(_MSC_VER)
	OutputDebugStringA(buffer);
#else
	printf(buffer);
#endif
}

bool eng_LogInternalCondition(bool condition, const char* fmt, ...)
{
	if (condition)
	{
		return true;
	}

	va_list args;
	va_start(args, fmt);

	static char buffer[2048];

	if (vsprintf(buffer, fmt, args) < 0)
	{
		printf("[ERROR] Internal error: log buffer not big enough for the log we're printing.");
		exit(-1);
	}

	va_end(args);

#if defined(_MSC_VER)
	OutputDebugStringA(buffer);
#else
	printf(buffer);
#endif

	return false;
}