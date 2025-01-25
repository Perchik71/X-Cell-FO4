// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

namespace XCell
{
	void XCAssertImpl(bool CompareResult, const char* SourceFile, int SourceLine, const char* FormattedMessage, ...);
}

#define XCAssert(Cond)									XCell::XCAssertImpl(Cond, __FILE__, __LINE__, #Cond)
#define XCAssertWithFormattedMessage(Cond, Msg, ...)	XCell::XCAssertImpl(Cond, __FILE__, __LINE__, "%s\n\n" Msg, #Cond, ##__VA_ARGS__)
#define XCAssertWithMessage(Cond, Msg)					XCAssertWithFormattedMessage(Cond, Msg)