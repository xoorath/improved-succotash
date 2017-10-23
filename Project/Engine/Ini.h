#pragma once

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif
#include <stdint.h>

typedef struct eng_IniR eng_IniR;

////////////////////////////////////////////////////////////////////////// Lifecycle
/**
* Ini (readable) Malloc
*
* @note The url is not ready for use until eng_IniRInit is called.
* @return A newly allocated ini.
*/
eng_IniR* eng_IniRMalloc(void);

/**
* Ini (readable) Init
*
* @description Initializes the ini, making it ready for use.
* @return true if initialization was successful.
*/
bool eng_IniRInit(eng_IniR* ini, const char* path);
	
/**
* Ini (readable) Free
*
* Frees memory associated with the ini. If subAllocationsOnly is  true, 
* the ini pointer itself will not be freed.
*/
void eng_IniRFree(eng_IniR* ini, bool subAllocationsOnly);

/**
* Ini (readable) Get Sizeof
*
* @return the sizeof the internal eng_Url object, for use with custom
* allocators.
*/
size_t eng_IniRGetSizeof(void);

////////////////////////////////////////////////////////////////////////// Ini API

const char* eng_IniRRead(eng_IniR* ini, const char* section, const char* key);

#ifdef __cplusplus
}
#endif