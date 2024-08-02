// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

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
		static HANDLE impl_find_first_file(const char* file_name, LPWIN32_FIND_DATAA pdata);
		static HANDLE impl_find_first_file_w(const wchar_t* file_name, LPWIN32_FIND_DATAA pdata);
		static int impl_inflate_init(z_stream_s* stream, const char* version, int mode);
		static int impl_inflate(z_stream_s* stream, int flush);
		static HANDLE impl_create_file(const char* file_name, unsigned int desired_access, unsigned int share_mode,
			LPSECURITY_ATTRIBUTES security_attributes, unsigned int creation_disposition, unsigned int flags_and_attributes,
			HANDLE template_file);
		static HANDLE impl_create_file_w(const wchar_t* file_name, unsigned int desired_access, unsigned int share_mode,
			LPSECURITY_ATTRIBUTES security_attributes, unsigned int creation_disposition, unsigned int flags_and_attributes,
			HANDLE template_file);
	};
}