// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <mini/ini.h>
#include <limits.h>
#include <libdeflate.h>
#include <xc_patch_io.h>
#include <xc_assertion.h>
#include <xc_version.h>
#include <xc_plugin.h>

namespace xc
{
	unordered_map<string_view, mINI::INIStructure*> _cache;

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
		// - Replacing functions WritePrivateProfileStringA, GetPrivateProfileStringA, GetPrivateProfileIntA
		//   They are outdated and constantly open and parsing the ini file.
		// - Replacing FindFirstNextA with a more optimized function FindFirstFileExA.

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

		patch_iat(base, "kernel32.dll", "WritePrivateProfileStringA", (uintptr_t)&impl_write_private_profile_string);
		patch_iat(base, "kernel32.dll", "GetPrivateProfileStringA", (uintptr_t)&impl_get_private_profile_string);
		patch_iat(base, "kernel32.dll", "GetPrivateProfileIntA", (uintptr_t)&impl_get_private_profile_int);
		patch_iat(base, "kernel32.dll", "FindFirstFileA", (uintptr_t)&impl_find_first_file);

		return true;
	}

	mINI::INIStructure* impl_get_file(const char* lpFileName)
	{
		mINI::INIStructure* ini_data = nullptr;

		auto iterator_find = _cache.find(lpFileName);
		if (iterator_find != _cache.end())
			ini_data = iterator_find->second;

		if (!ini_data)
		{
			mINI::INIFile* file = new mINI::INIFile(lpFileName);
			if (!file) return nullptr;

			ini_data = new mINI::INIStructure();
			if (!ini_data) return nullptr;
			file->read(*ini_data);

			_cache.insert(std::pair<string_view, mINI::INIStructure*>(lpFileName, ini_data));
		}

		return ini_data;
	}

	unsigned int patch_io::impl_get_private_profile_string(const char* app_name, const char* key_name, const char* default_value,
		char* returned_string, unsigned int size, const char* file_name)
	{
		if (!returned_string || !size)
			return 0;

		auto ini_data = impl_get_file(file_name);
		if (!ini_data)
		{
			// In the event the initialization file specified by file_name is not found, or contains invalid values, 
			// this function will set errorno with a value of '0x2' (File Not Found). 
			// To retrieve extended error information, call GetLastError.
			_set_errno(0x2);

			return 0;
		}

		string s;
		size_t l = 0;

		if (app_name && !key_name)
		{
			auto ip = ini_data->get(app_name);

			for (auto i = ip.begin(); i != ip.end(); i++)
			{
				s.append(i->first).append("=").append(i->second).append("\n");
				l = min((size_t)size, s.length());
				if (l == size) break;
			}

			memcpy(returned_string, (const void*)s.c_str(), l);
		}
		else if (!app_name)
		{
			for (auto j = ini_data->begin(); j != ini_data->end(); j++)
			{
				s.append("[").append(j->first).append("]\n");

				for (auto i = j->second.begin(); i != j->second.end(); i++)
				{
					s.append(i->first).append("=").append(i->second).append("\n");
					l = min((size_t)size, s.length());
					if (l == size) break;
				}
			}

			memcpy(returned_string, (const void*)s.c_str(), l);
		}
		else
		{
			auto ip = ini_data->get(app_name);
			if (!ip.has(key_name))
				s = default_value ? default_value : "";
			else
				s = ip.get(key_name);

			l = min((size_t)size, s.length());
			memcpy(returned_string, (const void*)s.c_str(), l);
		}

		returned_string[(l == size) ? l - 1 : l] = '\0';
		return l;
	}

	unsigned int patch_io::impl_get_private_profile_int(const char* app_name, const char* key_name, int default_value,
		const char* file_name)
	{
		if (!key_name || !app_name)
			return default_value;

		auto ini_data = impl_get_file(file_name);
		if (!ini_data)
		{
			// In the event the initialization file specified by file_name is not found, or contains invalid values, 
			// this function will set errorno with a value of '0x2' (File Not Found). 
			// To retrieve extended error information, call GetLastError.
			_set_errno(0x2);

			return default_value;
		}

		string s;
		auto ip = ini_data->get(app_name);
		if (!ip.has(key_name))
			return (DWORD)default_value;
		else
			s = ip.get(key_name);

		char* end_ptr = nullptr;
		return strtoul(s.c_str(), &end_ptr, 10);
	}

	bool patch_io::impl_write_private_profile_string(const char* app_name, const char* key_name, const char* string,
		const char* file_name)
	{
		if (!app_name || !file_name) return false;

		auto ini_data = impl_get_file(file_name);
		if (!ini_data) return false;

		if (!key_name)
			// The name of the key to be associated with a string.
			// If the key does not exist in the specified section, it is created.
			// If this parameter is NULL, the entire section, including all entries within the section, is deleted.
			ini_data->remove(app_name);
		else if (!string)
			// A null - terminated string to be written to the file.
			// If this parameter is NULL, the key pointed to by the key_name parameter is deleted.
			ini_data->get(app_name).remove(key_name);
		else
			ini_data->get(app_name).set(key_name, string ? string : "");

		mINI::INIFile file(file_name);
		return file.write(*ini_data);
	}

	HANDLE patch_io::impl_find_first_file(const char* file_name, LPWIN32_FIND_DATAA pdata)
	{
		return FindFirstFileExA(file_name, FindExInfoStandard, pdata, FindExSearchNameMatch,
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
}