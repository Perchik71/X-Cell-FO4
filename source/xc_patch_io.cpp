// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <xc_patch_io.h>

namespace xc
{
	const char* patch_io::get_name() const noexcept
	{
		return "io";
	}

	bool patch_io::run() const
	{
		auto base = GetModuleHandleA(NULL);

		//
		// io optimizations:
		//
		// - Replacing FindFirstNextA, FindFirstNextW with a more optimized function FindFirstFileExA, FindFirstFileExW.
		// - Use OS file cache for less disk access.

		patch_iat(base, "kernel32.dll", "FindFirstFileA", (uintptr_t)&impl_find_first_file);
		patch_iat(base, "kernel32.dll", "FindFirstFileW", (uintptr_t)&impl_find_first_file_w);
		patch_iat(base, "kernel32.dll", "CreateFileA", (uintptr_t)&impl_create_file);
		patch_iat(base, "kernel32.dll", "CreateFileW", (uintptr_t)&impl_create_file_w);

		return true;
	}

	HANDLE patch_io::impl_find_first_file(const char* file_name, LPWIN32_FIND_DATAA pdata)
	{
		return FindFirstFileExA(file_name, FindExInfoStandard, pdata, FindExSearchNameMatch,
			nullptr, FIND_FIRST_EX_LARGE_FETCH);
	}

	HANDLE patch_io::impl_find_first_file_w(const wchar_t* file_name, LPWIN32_FIND_DATAA pdata)
	{
		return FindFirstFileExW(file_name, FindExInfoStandard, pdata, FindExSearchNameMatch,
			nullptr, FIND_FIRST_EX_LARGE_FETCH);
	}

	HANDLE patch_io::impl_create_file(const char* file_name, unsigned int desired_access, unsigned int share_mode,
		LPSECURITY_ATTRIBUTES security_attributes, unsigned int creation_disposition, unsigned int flags_and_attributes,
		HANDLE template_file)
	{
		return CreateFileA(file_name, desired_access, share_mode, security_attributes, creation_disposition,
			flags_and_attributes & ~FILE_FLAG_NO_BUFFERING, template_file);
	}

	HANDLE patch_io::impl_create_file_w(const wchar_t* file_name, unsigned int desired_access, unsigned int share_mode,
		LPSECURITY_ATTRIBUTES security_attributes, unsigned int creation_disposition, unsigned int flags_and_attributes,
		HANDLE template_file)
	{
		return CreateFileW(file_name, desired_access, share_mode, security_attributes, creation_disposition,
			flags_and_attributes & ~FILE_FLAG_NO_BUFFERING, template_file);
	}
}