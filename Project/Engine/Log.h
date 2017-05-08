#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

#define SILENT_IN_FINAL

#if defined(GAME_FINAL) && defined(SILENT_IN_FINAL)
#define eng_Log(fmt, ...)
#define eng_Warn(fmt, ...)
#define eng_Err(fmt, ...)
#define eng_Ensure(condition, fmt, ...) (condition)
#else

// Log a message
#define eng_Log(fmt, ...) eng_LogInternal(fmt, __VA_ARGS__)

// Log a warning
#define eng_Warn(fmt, ...)  eng_LogInternal("[WARNING] " fmt, __VA_ARGS__)

// Log an error
#define eng_Err(fmt, ...) eng_LogInternal("[ERROR] " fmt, __VA_ARGS__)

// Always evaluate/return the condition. If the condition is false, eng_Err the message.
#define eng_Ensure(condition, fmt, ...) eng_LogInternalCondition(condition, "[ERROR] " fmt, __VA_ARGS__)

#endif

// Log a fatal error, then debug break
#define eng_DevFatal(fmt, ...) eng_LogInternal("[ERROR] " fmt, __VA_ARGS__); __debugbreak()

// Log a fatal error, then crash
#define eng_Fatal(fmt, ...) eng_LogInternal("[ERROR] " fmt, __VA_ARGS__); exit(-1)


void eng_LogInternal(const char* fmt, ...);
bool eng_LogInternalCondition(bool condition, const char* fmt, ...);

#ifdef __cplusplus
}
#endif