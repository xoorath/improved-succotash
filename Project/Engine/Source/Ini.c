#define _CRT_SECURE_NO_WARNINGS
#include <Engine/Ini.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Engine/Log.h>

typedef struct eng_IniKeyValue
{
	char* Key;
	char* Value;
	char* SourceStart;
	char* SourceEnd;
} eng_IniKeyValue;

typedef struct eng_IniSection
{
	char* SectionHead;
	uint32_t KeyValuePairCount;
	struct eng_IniKeyValue* KeyValuePairs;
} eng_IniSection;

typedef struct eng_IniR
{
	uint32_t FileSize;
	/**
	 * It's common to read multiple values from a single section.
	 * By tracking the last section read, we can often improve performance 
	 * by starting the search from there.
	 */
	uint32_t LastSectionPosition;
	char* FileContents;
	FILE* File;
	
	uint32_t SectionCount;
	struct eng_IniSection* Sections;
} eng_IniR;

uint32_t eng_IniRCountSections(eng_IniR* ini);
void eng_IniRInitSections(eng_IniR* ini);

eng_IniR* eng_IniRMalloc(void)
{
	return malloc(sizeof(eng_IniR));
}

bool eng_IniRInit(eng_IniR* ini, const char* path)
{
	{ // temp
		memset(ini, 0, sizeof(eng_IniR));
		return true;
	}

	ini->File = fopen(path, "r");
	if (ini->File == NULL)
	{
		return false;
	}
	fseek(ini->File, 0, SEEK_END);
	ini->FileSize = ftell(ini->File);
	rewind(ini->File);
	if (ini->FileSize)
	{
		ini->FileContents = malloc(ini->FileSize+1);
		fread(ini->FileContents, 1, ini->FileSize, ini->File);
		ini->FileContents[ini->FileSize] = '\0';
		for (uint32_t i = 0; i < ini->FileSize; ++i)
		{
			switch (ini->FileContents[i])
			{
			case '#':
			case ';':
			{
				char* s = &ini->FileContents[i];
				bool doneRemovingComment = false;
				// Anything that's commented out: remove entirely. 
				// Write the comment char and following text as '\0'
				for (;i < ini->FileSize;++s, ++i)
				{
					switch (*s)
					{
					case '\n':
					case '\r':
					case '\0':
						doneRemovingComment = true;
						break;
					default:
						*s = '\0';
						break;
					}
					if (doneRemovingComment)
					{
						break;
					}
				}
			}
			case '\n':
			case '\r':
				ini->FileContents[i] = '\0';
			}
			ini->FileContents[i] = toupper(ini->FileContents[i]);
		}

		ini->SectionCount = eng_IniRCountSections(ini);
		eng_IniRInitSections(ini);

	}
	else
	{
		ini->FileContents = NULL;
	}
	ini->LastSectionPosition = 0;

	return true;
}

void eng_IniRFree(eng_IniR* ini, bool subAllocationsOnly)
{
	if (ini == NULL)
	{
		return;
	}
	free(ini->FileContents);
	if (!subAllocationsOnly)
	{
		free(ini);
	}
}

size_t eng_IniRGetSizeof(void)
{
	return sizeof(eng_IniR);
}

////////////////////////////////////////////////////////////////////////// Ini API

const char* eng_IniRRead(eng_IniR* ini, const char* section, const char* key)
{
	return NULL;
}

////////////////////////////////////////////////////////////////////////// Internal
bool eng_IniRLineGetSectionHead(char* line, char** outSectionHeadStart, char** outSectionHeadEnd)
{
	for(;;)
	{
		switch (*line)
		{
		// Acceptable/skipable characters:
		case ' ':
		case '\t':
			break;
		// Terminating characters:
		default:
			*outSectionHeadEnd = line+1;
			return false;
		case '[':
			*outSectionHeadStart = line;
			++line;
			for (;;)
			{
				switch (*line)
				{
					// Terminating characters:
					case ';':
					case '#':
					case '[':
					case '\0':
						*outSectionHeadEnd = line+1;
						return false;
					case ']':
						*outSectionHeadEnd = line+1;
						return true;
				}
				++line;
			}
			*outSectionHeadEnd = line+1;
			return false;
		}
		++line;
	}
	*outSectionHeadEnd = line+1;
	return false;
}

bool eng_IniRLineGetVarStr(char* line, char** outVarStart, char** outVarEnd)
{
	for(;;)
	{
		switch (*line)
		{
		// Acceptable/skipable characters:
		case ' ':
		case '\t':
			break;
		// Terminating characters:
		case '[':
		case ']':
		case ';':
		case '#':
		case '\"':
			*outVarEnd = line+1;
			return false;
		default:
			*outVarStart = line;
			++line;
			for (;;)
			{
				switch (*line)
				{
					// Terminating characters:
					case '[':
					case ']':
					case ';':
					case '#':
					case '\"':
						*outVarEnd = line+1;
						return false;
					case '\0':
						*outVarEnd = line+1;
						return true;
				}
				++line;
			}
			*outVarEnd = line+1;
			return false;
		}
		++line;
	}
	*outVarEnd = line+1;
	return false;
}

uint32_t eng_IniRCountSections(eng_IniR* ini)
{
	char* c = ini->FileContents;
	char* s;
	char* e;
	
	intptr_t count = 0;
	for (intptr_t i = 0; i < (intptr_t)ini->FileSize;)
	{
		s = &ini->FileContents[i];
		if (eng_IniRLineGetSectionHead(s, &s, &e))
		{
			++count;
		}
		i += e - s;
		
	}
	if (count > UINT32_MAX)
	{
		eng_DevFatal("Something went wrong, count is higher than 32bit max.");
	}
	return (uint32_t)count;
}

uint32_t eng_IniRCoutnVars(eng_IniR* ini)
{
	char* c = ini->FileContents;
	char* s;
	char* e;
	intptr_t count = 0;
	for (intptr_t i = 0; i < (intptr_t)ini->FileSize;)
	{
		s = &ini->FileContents[i];
		if (eng_IniRLineGetSectionHead(s, &s, &e))
		{
			++count;
		}
		i += e - s;
		
	}
	if (count > UINT32_MAX)
	{
		eng_DevFatal("Something went wrong, count is higher than 32bit max.");
	}
	return (uint32_t)count;
}

void eng_IniRInitSections(eng_IniR* ini)
{
	if (ini->SectionCount == 0)
	{
		ini->Sections = NULL;
		return;
	}
	ini->Sections = calloc(ini->SectionCount, sizeof(eng_IniSection));

	char* c = ini->FileContents;
	char* s;
	char* e;
	uint32_t sectionIdx = 0;
	for (intptr_t i = 0; i < (intptr_t)ini->FileSize;)
	{
		s = &ini->FileContents[i];
		if (eng_IniRLineGetSectionHead(s, &s, &e))
		{
			eng_IniSection* section = &ini->Sections[sectionIdx++];

		}
		i += e - s;
		
	}
}