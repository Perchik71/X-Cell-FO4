#pragma once

#include "f4se_common/Utilities.h"

class Heap_OG
{
public:
	MEMBER_FN_PREFIX(Heap_OG);
	DEFINE_MEMBER_FN(Allocate, void *, 0x01B0EFD0, size_t size, size_t alignment, bool aligned);
	DEFINE_MEMBER_FN(Free, void, 0x01B0F2E0, void * buf, bool aligned);
};

extern RelocPtr <Heap_OG> g_mainHeap_OG;

void * Heap_Allocate_OG(size_t size);
void Heap_Free_OG(void * ptr);

class ConsoleManager_OG
{
public:
	MEMBER_FN_PREFIX(ConsoleManager_OG);
	DEFINE_MEMBER_FN(VPrint, void, 0x01262EC0, const char * fmt, va_list args);
	DEFINE_MEMBER_FN(Print, void, 0x01262F50, const char * str);
};

extern RelocPtr <ConsoleManager_OG*> g_console_OG;
extern RelocAddr <UInt32 *> g_consoleHandle_OG;

void Console_Print_OG(const char * fmt, ...);

LONGLONG GetPerfCounter_OG(void);
