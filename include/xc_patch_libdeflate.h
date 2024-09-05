// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <xc_patch.h>

namespace xc
{
	class patch_libdeflate : public patch
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

		patch_libdeflate() = default;
		patch_libdeflate(const patch_libdeflate&) = default;
		virtual ~patch_libdeflate() = default;

		patch_libdeflate& operator=(const patch_libdeflate&) = default;

		virtual const char* get_name() const noexcept;
		virtual bool game_data_ready_handler() const noexcept;
	protected:
		virtual bool run() const;
	private:
		static int impl_inflate_init(z_stream_s* stream, const char* version, int mode);
		static int impl_inflate(z_stream_s* stream, int flush);
	};
}