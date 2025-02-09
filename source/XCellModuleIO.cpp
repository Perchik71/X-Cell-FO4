// Copyright © 2024-2025 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include "XCellModuleIO.h"
#include "XCellTableID.h"
#include "XCellPlugin.h"
#include "XCellCVar.h"

namespace XCell
{
	static DWORD gIOCacheFlag = 0;

	static HANDLE WINAPI HKFindFirstFileExA(LPCSTR file_name, LPWIN32_FIND_DATAA pdata)
	{
		return FindFirstFileExA(file_name, FindExInfoStandard, pdata, FindExSearchNameMatch,
			nullptr, FIND_FIRST_EX_LARGE_FETCH);
	}

	static HANDLE WINAPI HKFindFirstFileExW(LPCWSTR file_name, LPWIN32_FIND_DATAA pdata)
	{
		return FindFirstFileExW(file_name, FindExInfoStandard, pdata, FindExSearchNameMatch,
			nullptr, FIND_FIRST_EX_LARGE_FETCH);
	}

	static HANDLE WINAPI HKCreateFileA(LPCSTR FileName, DWORD DesiredAccess, DWORD ShareMode,
		LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDisposition, DWORD FlagsAndAttributes,
		HANDLE TemplateFile)
	{
		return CreateFileA(FileName, DesiredAccess, ShareMode, SecurityAttributes, CreationDisposition,
			FILE_FLAG_OVERLAPPED | gIOCacheFlag, TemplateFile);
	}

	static HANDLE WINAPI HKCreateFileW(LPCWSTR FileName, DWORD DesiredAccess, DWORD ShareMode,
		LPSECURITY_ATTRIBUTES SecurityAttributes, DWORD CreationDisposition, DWORD FlagsAndAttributes,
		HANDLE TemplateFile)
	{
		return CreateFileW(FileName, DesiredAccess, ShareMode, SecurityAttributes, CreationDisposition,
			FILE_FLAG_OVERLAPPED | gIOCacheFlag, TemplateFile);
	}

	XCellModuleIO::XCellModuleIO(void* Context) :
		Module(Context, SourceName, CVarIO)
	{
		auto gContext = (XCell::Context*)Context;
		auto base = gContext->ProcessBase;

		//
		// io optimizations:
		//
		// - Replacing FindFirstNextA, FindFirstNextW with a more optimized function FindFirstFileExA, FindFirstFileExW.
		// - Use OS file cache for less disk access.

		_functions[0].Install(base, "kernel32.dll", "FindFirstFileA", (UInt64)&HKFindFirstFileExA);
		_functions[1].Install(base, "kernel32.dll", "FindFirstFileW", (uintptr_t)&HKFindFirstFileExW);
		_functions[2].Install(base, "kernel32.dll", "CreateFileA", (uintptr_t)&HKCreateFileA);
		_functions[3].Install(base, "kernel32.dll", "CreateFileW", (uintptr_t)&HKCreateFileW);

		gIOCacheFlag = CVarUseIORandomAccess->GetBool() ? FILE_FLAG_RANDOM_ACCESS : FILE_FLAG_SEQUENTIAL_SCAN;
	}

	HRESULT XCellModuleIO::InstallImpl()
	{
		if ((REL::Version() == RUNTIME_VERSION_1_10_163) && GetModuleHandleA("libdiskCacheEnabler.dll"))
			_WARNING("Mod \"\" has been detected. X-Cell it's unnecessary.");

		//
		// io optimizations:
		//
		// - Replacing FindFirstNextA, FindFirstNextW with a more optimized function FindFirstFileExA, FindFirstFileExW.
		// - Use OS file cache for less disk access.

		_functions[0].Enable();
		_functions[1].Enable();
		_functions[2].Enable();
		_functions[3].Enable();

		return S_OK;
	}

	HRESULT XCellModuleIO::ShutdownImpl()
	{
		// Returned

		_functions[0].Disable();
		_functions[1].Disable();
		_functions[2].Disable();
		_functions[3].Disable();

		return S_OK;
	}
}