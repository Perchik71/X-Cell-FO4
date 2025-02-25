// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

namespace XCell
{
	namespace Utils
	{
		wstring __stdcall AnsiToWide(const string& String);
		string __stdcall WideToAnsi(const wstring& String);

		string __stdcall AnsiToUtf8(const string& String);
		string __stdcall Utf8ToAnsi(const string& String);

		string& __stdcall Trim(string& String);

		UInt32 __stdcall MurmurHash32A(const void* Key, size_t Len, UInt32 Seed);
		UInt64 __stdcall MurmurHash64A(const void* Key, size_t Len, UInt64 Seed);

		string __stdcall GetApplicationPath();
		string __stdcall GetGameDataPath();
	}
}