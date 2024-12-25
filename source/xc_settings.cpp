// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#include <mini/ini.h>
#include <xc_settings.h>
#include <stdio.h>
#include <ShlObj.h>
#include <IFileStream.h>

namespace xc
{
	settings::settings() : _handle(nullptr)
	{}

	settings::settings(const char* file_name) : _handle(nullptr)
	{
		set_filename(file_name);
	}

	settings::settings(int folder_id, const char* relpath)
	{
		set_filename(folder_id, relpath);
	}

	settings::settings(const settings& s) : _handle(nullptr)
	{
		set_filename(s._file_name.c_str());
	}

	settings::~settings()
	{
		if (_handle)
			delete (mINI::INIStructure*)_handle;
	}

	settings& settings::operator=(const settings& s)
	{
		set_filename(s._file_name.c_str());
		return *this;
	}

	bool settings::set_filename(const char* file_name) noexcept
	{
		auto result = GetFileAttributesA(file_name);

		//if ((result == INVALID_FILE_ATTRIBUTES) || ((result & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY))
		//	return false;

		_handle = new mINI::INIStructure();
		if (_handle)
		{
			mINI::INIFile file(file_name);
			if (!file.read(*((mINI::INIStructure*)_handle)))
				_ERROR("Error opening the configuration file \"%s\"", file_name);

			_file_name = file_name;
			return true;
		}

		return false;
	}

	bool settings::set_filename(int folder_id, const char* relpath) noexcept
	{
		char path[MAX_PATH];

		HRESULT err = SHGetFolderPath(NULL, folder_id | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, path);
		if (!SUCCEEDED(err))
			_FATALERROR("SHGetFolderPath %08X failed (result = %08X lasterr = %08X)", folder_id, err, GetLastError());

		ASSERT_CODE(SUCCEEDED(err), err);

		strcat_s(path, sizeof(path), relpath);
		IFileStream::MakeAllDirs(path);
		return set_filename(path);
	}

	void settings::save() const noexcept
	{
		if (_handle)
		{
			mINI::INIFile file(_file_name);
			file.write(*((mINI::INIStructure*)_handle), false, true);
		}
	}

	string settings::read_str(const char* section, const char* name, const char* default_value) const noexcept
	{
		auto h = (mINI::INIStructure*)_handle;
		if (!h) return default_value;
		if (!h->get(section).has(name)) return default_value;
		return h->get(section).get(name);
	}

	int32_t settings::read_int(const char* section, const char* name, int32_t default_value) const noexcept
	{
		auto h = (mINI::INIStructure*)_handle;
		if (!h) return 0;
		if (!h->get(section).has(name)) return default_value;
		auto s = h->get(section).get(name);
		char* end_ptr = nullptr;
		return strtol(s.c_str(), &end_ptr, 10);
	}

	uint32_t settings::read_uint(const char* section, const char* name, uint32_t default_value) const noexcept
	{
		auto h = (mINI::INIStructure*)_handle;
		if (!h) return 0;
		if (!h->get(section).has(name)) return default_value;
		auto s = h->get(section).get(name);
		char* end_ptr = nullptr;
		return strtoul(s.c_str(), &end_ptr, 10);
	}

	float settings::read_float(const char* section, const char* name, float default_value) const noexcept
	{
		auto h = (mINI::INIStructure*)_handle;
		if (!h) return default_value;
		if (!h->get(section).has(name)) return default_value;
		return strtof(h->get(section).get(name).c_str(), nullptr);
	}

	bool settings::read_bool(const char* section, const char* name, bool default_value) const noexcept
	{
		auto result = read_str(section, name, "");
		if (result.empty()) return default_value;
		return (result == "1") || !_stricmp("true", result.c_str());
	}

	void settings::write_str(const char* section, const char* name, const char* value) const noexcept
	{
		auto h = (mINI::INIStructure*)_handle;
		if (!h) return;
		(*h)[section][name] = value;
		save();
	}

	void settings::write_int(const char* section, const char* name, int32_t value) const noexcept
	{
		auto h = (mINI::INIStructure*)_handle;
		if (!h) return;
		(*h)[section][name] = std::to_string(value);
		save();
	}

	void settings::write_uint(const char* section, const char* name, uint32_t value) const noexcept
	{
		auto h = (mINI::INIStructure*)_handle;
		if (!h) return;
		(*h)[section][name] = std::to_string(value);
		save();
	}

	void settings::write_float(const char* section, const char* name, float value) const noexcept
	{
		auto h = (mINI::INIStructure*)_handle;
		if (!h) return;
		(*h)[section][name] = std::to_string(value);
		save();
	}

	void settings::write_bool(const char* section, const char* name, bool value) const noexcept
	{
		write_str(section, name, value ? "true" : "false");
	}
}