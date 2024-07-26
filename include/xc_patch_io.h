// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <unordered_map>
#include <xc_patch.h>

namespace xc
{
	class patch_io : public patch
	{
	public:
		struct z_stream_s
		{
			const void* next_in;
			uint32_t avail_in;
			uint32_t total_in;
			void* next_out;
			uint32_t avail_out;
			uint32_t total_out;
			const char* msg;
			struct internal_state* state;
		};

		patch_io() = default;
		patch_io(const patch_io&) = default;
		virtual ~patch_io() = default;

		patch_io& operator=(const patch_io&) = default;

		virtual const char* get_name() const noexcept;
	protected:
		virtual bool run() const;
	private:
		static unsigned int impl_get_private_profile_string(const char* app_name, const char* key_name, const char* default_value,
			char* returned_string, unsigned int size, const char* file_name);
		static unsigned int impl_get_private_profile_int(const char* app_name, const char* key_name, int default_value,
			const char* file_name);
		static bool impl_write_private_profile_string(const char* app_name, const char* key_name, const char* string,
			const char* file_name);
		static HANDLE impl_find_first_file(const char* file_name, LPWIN32_FIND_DATAA pdata);
		static int impl_inflate_init(z_stream_s* stream, const char* version, int mode);
		static int impl_inflate(z_stream_s* stream, int flush);
	};
}