// Copyright © 2024 aka perchik71. All rights reserved.
// Contacts: <email:timencevaleksej@gmail.com>
// License: https://www.gnu.org/licenses/gpl-3.0.html

#pragma once

#include <xc_patch.h>

namespace xc
{
	class fix_package_allocate_location : public patch
	{
	public:
		fix_package_allocate_location() = default;
		fix_package_allocate_location(const fix_package_allocate_location&) = default;
		virtual ~fix_package_allocate_location() = default;

		fix_package_allocate_location& operator=(const fix_package_allocate_location&) = default;

		static void* sub(void* self);

		virtual const char* get_name() const noexcept;
		virtual bool game_data_ready_handler() const noexcept;
	protected:
		virtual bool run() const;
	};
}