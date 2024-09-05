// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <xc_patch.h>

namespace xc
{
	class patch_profile : public patch
	{
	public:
		patch_profile() = default;
		patch_profile(const patch_profile&) = default;
		virtual ~patch_profile() = default;

		patch_profile& operator=(const patch_profile&) = default;

		virtual const char* get_name() const noexcept;
		virtual bool game_data_ready_handler() const noexcept;
	protected:
		virtual bool run() const;
	private:
		static unsigned int impl_get_private_profile_string(const char* app_name, const char* key_name, const char* default_value,
			char* returned_string, unsigned int size, const char* file_name);
		static unsigned int impl_get_private_profile_int(const char* app_name, const char* key_name, int default_value,
			const char* file_name);
		static bool impl_write_private_profile_string(const char* app_name, const char* key_name, const char* string,
			const char* file_name);
	};
}