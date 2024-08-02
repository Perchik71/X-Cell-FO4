// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <stdarg.h>
#include <xc_assertion.h>

namespace xc
{
	void _assert(bool comp, const char* file_name, int line, const char* fmt, ...)
	{
		if (comp) return;

		char szFormatted[512];
		char szBuffer[512];

		sprintf_s(szFormatted, "ASSERTION:\n\n\t\tSource: \"%s\"\n\t\tLine: %i\n\n%s", 
			file_name, line, fmt);
		szFormatted[511] = '\0';

		va_list ap;
		va_start(ap, fmt);
		vsprintf_s(szBuffer, szFormatted, ap);
		szBuffer[511] = '\0';
		va_end(ap);
		
		MessageBoxA(0, szBuffer, "ASSERTION", MB_OK | MB_ICONERROR);
		_FATALERROR(szBuffer);

		TerminateProcess(GetCurrentProcess(), 0);
		__assume(0);
	}
}