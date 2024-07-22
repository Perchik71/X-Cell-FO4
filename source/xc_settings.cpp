// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <xc_settings.h>

namespace xc
{
	settings::settings()
	{}

	settings::settings(const char* file_name)
	{
		set_filename(file_name);
	}

	settings::settings(const settings& s) : _file_name(s._file_name)
	{}

	settings& settings::operator=(const settings& s)
	{
		_file_name = s._file_name;
		return *this;
	}

	bool settings::set_filename(const char* file_name) noexcept
	{
		auto result = GetFileAttributesA(file_name);

		if ((result == INVALID_FILE_ATTRIBUTES) || ((result & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
			return false;

		_file_name = file_name;
		return true;
	}

	string settings::read_str(const char* section, const char* name, const char* default_value) const noexcept
	{
		char szBuffer[MAX_PATH];
		GetPrivateProfileStringA(section, name, default_value, szBuffer, MAX_PATH, _file_name.c_str());
		szBuffer[MAX_PATH - 1] = '\0';
		return szBuffer;
	}

	int32_t settings::read_int(const char* section, const char* name, int32_t default_value) const noexcept
	{
		auto result = (int32_t)GetPrivateProfileIntA(section, name, default_value, _file_name.c_str());
		return result;
	}

	uint32_t settings::read_uint(const char* section, const char* name, uint32_t default_value) const noexcept
	{
		auto result = (uint32_t)GetPrivateProfileIntA(section, name, (int32_t)default_value, _file_name.c_str());
		return result;
	}

	bool settings::read_bool(const char* section, const char* name, bool default_value) const noexcept
	{
		auto result = read_str(section, name, "");
		if (result.empty()) return default_value;
		return (result == "1") || !_stricmp("true", result.c_str());
	}
}