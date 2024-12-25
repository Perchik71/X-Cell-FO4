// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

namespace xc
{
	class settings
	{
	public:
		settings();
		settings(const char* file_name);
		settings(int folder_id, const char* relpath);
		settings(const settings& s);
		~settings();

		settings& operator=(const settings& p);

		bool set_filename(const char* file_name) noexcept;
		bool set_filename(int folder_id, const char* relpath) noexcept;
		inline string get_filename() const noexcept { return _file_name; }
		inline void* get_handle() const noexcept { return _handle; }
		void save() const noexcept;

		string read_str(const char* section, const char* name, const char* default_value) const noexcept;
		int32_t read_int(const char* section, const char* name, int32_t default_value) const noexcept;
		uint32_t read_uint(const char* section, const char* name, uint32_t default_value) const noexcept;
		float read_float(const char* section, const char* name, float default_value) const noexcept;
		bool read_bool(const char* section, const char* name, bool default_value) const noexcept;

		void write_str(const char* section, const char* name, const char* value) const noexcept;
		void write_int(const char* section, const char* name, int32_t value) const noexcept;
		void write_uint(const char* section, const char* name, uint32_t value) const noexcept;
		void write_float(const char* section, const char* name, float value) const noexcept;
		void write_bool(const char* section, const char* name, bool value) const noexcept;
	private:
		string _file_name;
		void* _handle;
	};
}