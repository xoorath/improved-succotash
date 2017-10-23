#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define ENG_ARRAY_CALLOC calloc
#define ENG_ARRAY_REALLOC realloc
#define ENG_ARRAY_FREE free

typedef struct eng_Array {
	void* Buffer;
	uint32_t Count;
	uint32_t BufferSize;
	uint32_t TypeSize;
} eng_Array;

#define eng_ArrayDecl(name, type) eng_Array name

void eng_ArrayInit(eng_Array* array, uint32_t typeSize);
#define eng_ArrayInitType(array, type) eng_ArrayInit(array, sizeof(type))
void eng_ArrayDestroy(eng_Array* array);

void eng_ArrayReserve(eng_Array* array, uint32_t elementsToReserve);
void eng_ArrayResize(eng_Array* array, uint32_t newElementCount);

void* eng_ArrayIndex(eng_Array* array, uint32_t index);
#define eng_ArrayIndexType(array, type, index) (*(type*)eng_ArrayIndex(array, index))
#define eng_ArrayPIndexType(array, type, index) ((type*)eng_ArrayIndex(array, index))

uint32_t eng_ArrayPushBack(eng_Array* array, void* object);
void eng_ArrayPushBackMany(eng_Array* array, void* objects, uint32_t count);

void* eng_ArrayBegin(eng_Array* array);
#define eng_ArrayBeginType(array, type) ((type*)eng_ArrayBegin(array));
void* eng_ArrayEnd(eng_Array* array);
#define eng_ArrayEndType(array, type) ((type*)eng_ArrayEnd(array));

void eng_ArrayRemoveLastSwap(eng_Array* array, uint32_t index);
void eng_ArrayRemoveInPlace(eng_Array* array, uint32_t index);

#ifdef __cplusplus
}
#endif