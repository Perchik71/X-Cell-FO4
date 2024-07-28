// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <limits.h>
#include <libdeflate.h>
#include <xc_patch_io.h>
#include <xc_assertion.h>
#include <xc_version.h>
#include <xc_plugin.h>

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
		// - Replace old zlib decompression code with optimized libdeflate.
		// - Replacing FindFirstNextA, FindFirstNextW with a more optimized function FindFirstFileExA, FindFirstFileExW.
		// - Use OS file cache for less disk access.

		if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_163)
		{
			// 163
			detour_call((g_plugin->get_base() + 0x13267D), (uintptr_t)&impl_inflate_init);
			detour_call((g_plugin->get_base() + 0x1326AF), (uintptr_t)&impl_inflate);
		}
		else if (g_plugin->get_runtime_version() == RUNTIME_VERSION_1_10_984)
		{
			// 984
			detour_call((g_plugin->get_base() + 0x2A5352), (uintptr_t)&impl_inflate_init);
			detour_call((g_plugin->get_base() + 0x2A5384), (uintptr_t)&impl_inflate);
		}

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

	int patch_io::impl_inflate_init(z_stream_s* stream, const char* version, int mode)
	{
		// Force inflateEnd to error out and skip frees
		stream->state = nullptr;

		return 0;
	}

	int patch_io::impl_inflate(z_stream_s* stream, int flush)
	{
		size_t outBytes = 0;
		libdeflate_decompressor* decompressor = libdeflate_alloc_decompressor();

		libdeflate_result result = libdeflate_zlib_decompress(decompressor, stream->next_in, stream->avail_in,
			stream->next_out, stream->avail_out, &outBytes);
		libdeflate_free_decompressor(decompressor);

		if (result == LIBDEFLATE_SUCCESS)
		{
			_xc_assert(outBytes < numeric_limits<uint32_t>::max());

			stream->total_in = stream->avail_in;
			stream->total_out = (uint32_t)outBytes;

			return 1;
		}

		if (result == LIBDEFLATE_INSUFFICIENT_SPACE)
			return -5;

		return -2;
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