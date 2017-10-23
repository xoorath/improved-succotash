#include <Engine/Array.h>

#include <Engine/Log.h>

#include <stdlib.h>
#include <string.h>

// This was not some carefully selected decision
// If you have a better idea about array growth rates you should propose it.
#define INITIAL_ARRAY_SIZE 4
#define GROWTH_PER_EXCESS 2.0

void eng_ArrayInit(eng_Array* array, uint32_t typeSize)
{
	array->TypeSize = typeSize;
	array->Count = 0;
	array->BufferSize = INITIAL_ARRAY_SIZE * typeSize;
	array->Buffer = ENG_ARRAY_CALLOC(array->BufferSize, array->TypeSize);
}

void eng_ArrayDestroy(eng_Array* array)
{
	free(array->Buffer);
}

void eng_ArrayReserve(eng_Array* array, uint32_t reservation)
{
#if !defined(GAME_FINAL)
	if (array->BufferSize == reservation)
	{
		eng_DevFatal("Reserving an array to the size it's already at is redundant.\n");
		return;
	}
	else 
#endif
	if (array->BufferSize < reservation)
	{
		array->Buffer = ENG_ARRAY_REALLOC(array->Buffer, reservation);
		array->BufferSize = reservation;
	}
	else
	{
#if !defined(GAME_FINAL)
		if (reservation < array->Count)
		{
			eng_DevFatal("Reserving an array to be smaller than its contents is invalid. Did you mean to resize then reserve?\n");
		}
#endif
		array->Buffer = ENG_ARRAY_REALLOC(array->Buffer, reservation);
		array->BufferSize = reservation;
	}
}

void eng_ArrayResize(eng_Array* array, uint32_t newSize)
{
#if !defined(GAME_FINAL)
	if (newSize == array->Count)
	{
		eng_DevFatal("Array resize is redundant.");
	}
#endif	
	if (newSize < array->Count)
	{
		array->Count = newSize;
		// todo: re-reserve if delta is too great.
	}
	else
	{
		array->Count += newSize;
	}
}

void* eng_ArrayIndex(eng_Array* array, uint32_t index)
{
#if !defined(GAME_FINAL)
	if (index > array->Count)
	{
		eng_DevFatal("Array index out of bounds");
	}
#endif
	return (char*)array->Buffer + (intptr_t)(index * array->TypeSize);
}

uint32_t eng_ArrayPushBack(eng_Array* array, void* object)
{
	eng_ArrayPushBackMany(array, object, 1);
	return array->Count - 1;
}

void eng_ArrayPushBackMany(eng_Array* array, void* objects, uint32_t count)
{
	uint32_t newCount = array->Count + count;
	uint32_t requiredBufferSize = newCount * array->TypeSize;
	if (array->BufferSize < requiredBufferSize)
	{
		double newBufferSize = (double)array->BufferSize;
		do 
		{
			newBufferSize *= GROWTH_PER_EXCESS;
		} while ((uint32_t)newBufferSize < requiredBufferSize);
		eng_ArrayReserve(array, (uint32_t)newBufferSize);
	}

	memcpy((char*)array->Buffer + (intptr_t)(array->Count * array->TypeSize), objects, count * array->TypeSize);
	array->Count = newCount;
}


void* eng_ArrayBegin(eng_Array* array)
{
	return array->Buffer;
}

void* eng_ArrayEnd(eng_Array* array)
{
	return (char*)array->Buffer + (intptr_t)(array->Count * array->TypeSize);
}

void eng_ArrayRemoveLastSwap(eng_Array* array, uint32_t index)
{
#if !defined(GAME_FINAL)
	if (index >= array->Count)
	{
		eng_DevFatal("Trying to remove an element that is not included in the array.");
	}
#endif
	void* element = (char*)array->Buffer + (intptr_t)(index * array->TypeSize);
	memcpy(element, (char*)array->Buffer + (intptr_t)(--array->Count * array->TypeSize), array->TypeSize);
}

void eng_ArrayRemoveInPlace(eng_Array* array, uint32_t index)
{
#if !defined(GAME_FINAL)
	if (index >= array->Count)
	{
		eng_DevFatal("Trying to remove an element that is not included in the array.");
	}
#endif
	void* element = (char*)array->Buffer + (intptr_t)(index * array->TypeSize);
	memcpy(element, (char*)element + (intptr_t)array->TypeSize, (size_t)((char*)eng_ArrayEnd(array)- (char*)element) - array->TypeSize);
	--array->Count;
}