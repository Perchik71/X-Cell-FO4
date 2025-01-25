// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <stdarg.h>

#include <comdef.h>
#include <XCellAssertion.h>

namespace XCell
{
	static char szAssertFormattedMessageString[512];
	static char szAssertFormattedMessageString2[512];

	static void XCAssertInstantlyFormattedMessageImpl(const char* FormattedMessage, va_list Args)
	{
		if (!FormattedMessage)
			return;

		vsprintf_s(szAssertFormattedMessageString, FormattedMessage, Args);

		auto Error = GetLastError();
		if (Error)
		{
			strcat_s(szAssertFormattedMessageString, "\n\nSystem Message: ");
			strcat_s(szAssertFormattedMessageString, _com_error(Error).ErrorMessage());
		}

		_ERROR("[ASSERTION] %s", szAssertFormattedMessageString);
		MessageBoxA(0, szAssertFormattedMessageString, "ASSERTION", MB_OK | MB_ICONERROR);
		TerminateProcess(GetCurrentProcess(), 1);
		__assume(0);
	}

	static void XCAssertInstantlyFormattedMessage(const char* FormattedMessage, ...)
	{
		if (!FormattedMessage)
			return;

		va_list Args;
		va_start(Args, FormattedMessage);
		XCAssertInstantlyFormattedMessageImpl(FormattedMessage, Args);
		va_end(Args);
	}

	void XCAssertImpl(bool CompareResult, const char* SourceFile, int SourceLine, const char* FormattedMessage, ...)
	{
		if (CompareResult || !SourceFile || !FormattedMessage)
			return;

		va_list Args;
		va_start(Args, FormattedMessage);
		vsprintf_s(szAssertFormattedMessageString2, FormattedMessage, Args);
		va_end(Args);

		XCAssertInstantlyFormattedMessage("ASSERTION:\n\n\t\tSource: \"%s\"\n\t\tLine: %i\n\n%s", SourceFile, SourceLine, szAssertFormattedMessageString2);
	}
}